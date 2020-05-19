/*
 * rubusd - A deamon for communicating with Joy-Con devices
 * Copyright (C) 2020 TechnoElf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "joycon.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>


uint8_t jc_recv_buf[256] = {0};
uint8_t jc_packet_number = 0;


void jc_print_buffer(uint8_t* b, uint16_t l) {
	for (uint16_t i = 0; i < l; i++) {
		if (i % 8 != 0) printf(" ");
		printf("%02X", b[i]);
		if ((i > 0) && ((i + 1) % 8 == 0) && (i != l - 1)) printf("\n");
	}
	printf("\n");
}

uint8_t jc_next_num() {
	uint8_t n = jc_packet_number;
	if (n == 0x0f) {
		jc_packet_number = 0;
	} else {
		jc_packet_number += 1;
	}
	return n;
}


uint32_t jc_parse_address(const wchar_t* address) {
	char addr[18] = {0};
	wcstombs(addr, address, 18);
	addr[2] = 0;
	addr[5] = 0;
	addr[8] = 0;
	addr[11] = 0;
	addr[14] = 0;

	uint32_t out = 0;

	out |= ((uint32_t) strtol(addr + 6, NULL, 16)) << 24;
	out |= ((uint32_t) strtol(addr + 9, NULL, 16)) << 16;
	out |= ((uint32_t) strtol(addr + 12, NULL, 16)) << 8;
	out |= (uint32_t) strtol(addr + 15, NULL, 16);

	return out;
}

JoyConDevice* jc_create(const char* path, uint32_t address) {
	JoyConDevice* con = calloc(1, sizeof(JoyConDevice));
	con->dev = hid_open_path(path);
	if (con->dev < 0) {
		JC_ERROR_LOG("Failed to open device \"%s\"", path);
		free(con);
		return JC_ERR;
	}
	con->address = address;
	
	wchar_t wide_name[32] = {0};
	char name[32] = {0};
	if (hid_get_product_string(con->dev, wide_name, 32) < 0) {
		free(con);
		return JC_ERR;
	}
	wcstombs(name, wide_name, 32);
	if (strcmp(name, "Joy-Con (L)") == 0) {
		con->type = JoyConLeft;
	} else if (strcmp(name, "Joy-Con (R)") == 0) {
		con->type = JoyConRight;
	} else {
		JC_ERROR_LOG("Device \"%s\" not supported", name);
		free(con);
		return JC_ERR;
	}

	if (jc_flush_input(con) == JC_ERR) {
		free(con);
		return JC_ERR;
	}
	
	if (jc_set_player_lights(0xf, con) == JC_ERR) {
		free(con);
		return JC_ERR;
	}

	return con;
}

JC_RESULT jc_destroy(JoyConDevice* con) {
	hid_close(con->dev);
	free(con);
	return JC_OK;
}

JC_RESULT jc_flush_input(JoyConDevice* con) {
	uint8_t data[1] = { JC_INPUT_MODE_SIMPLE };
	JC_TRY(jc_send_sub_command(JC_SC_SET_INPUT_MODE, data, 1, jc_recv_buf, 1, con));
	while (hid_read_timeout(con->dev, jc_recv_buf, 1, 100)) {}
	return JC_OK;
}

JC_RESULT jc_calibrate(JoyConDevice* con) {
	uint8_t* data;
	JC_TRY_UNWRAP(data, jc_read_spi(0x603D, 18, jc_recv_buf, con));

	con->analog_l_cal.x_cen = (data[4] << 8) & 0xf00 | data[3];
	con->analog_l_cal.y_cen = (data[5] << 4) | (data[10] >> 4);
	con->analog_l_cal.x_min = con->analog_l_cal.x_cen - ((data[7] << 8) & 0xf00 | data[6]);
	con->analog_l_cal.y_min = con->analog_l_cal.y_cen - ((data[8] << 4) | (data[7] >> 4));
	con->analog_l_cal.x_max = con->analog_l_cal.x_cen + ((data[1] << 8) & 0xf00 | data[0]);
	con->analog_l_cal.y_max = con->analog_l_cal.y_cen + ((data[2] << 4) | (data[1] >> 4));

	con->analog_r_cal.x_cen = (data[10] << 8) & 0xf00 | data[9];
	con->analog_r_cal.y_cen = (data[11] << 4) | (data[10] >> 4);
	con->analog_r_cal.x_min = con->analog_r_cal.x_cen - ((data[13] << 8) & 0xf00 | data[12]);
	con->analog_r_cal.y_min = con->analog_r_cal.y_cen - ((data[14] << 4) | (data[13] >> 4));
	con->analog_r_cal.x_max = con->analog_r_cal.x_cen + ((data[16] << 8) & 0xf00 | data[15]);
	con->analog_r_cal.y_max = con->analog_r_cal.y_cen + ((data[17] << 4) | (data[16] >> 4));

	return JC_OK;
}


JC_RESULT jc_send_command(uint8_t command, uint8_t* data, uint16_t length, uint8_t* out, uint16_t out_length, JoyConDevice* con) {
	jc_recv_buf[0] = command;
	jc_recv_buf[1] = jc_next_num();
	memcpy(jc_recv_buf + 2, data, length);

	JC_HID_TRY(hid_write(con->dev, jc_recv_buf, length + 2));
	JC_HID_TRY(hid_read(con->dev, out, out_length));

	return JC_OK;
}

JC_RESULT jc_send_sub_command(uint8_t command, uint8_t* data, uint16_t length, uint8_t* out, uint16_t out_length, JoyConDevice* con) {
	jc_recv_buf[0] = JC_C_SUBCOMMAND;
	jc_recv_buf[1] = jc_next_num();
	jc_recv_buf[2] = 0x00;
	jc_recv_buf[3] = 0x01;
	jc_recv_buf[4] = 0x40;
	jc_recv_buf[5] = 0x40;
	jc_recv_buf[6] = 0x00;
	jc_recv_buf[7] = 0x01;
	jc_recv_buf[8] = 0x40;
	jc_recv_buf[9] = 0x40;
	jc_recv_buf[10] = command;
	memcpy(jc_recv_buf + 11, data, length);
	
	JC_HID_TRY(hid_write(con->dev, jc_recv_buf, length + 11));
	JC_HID_TRY(hid_read(con->dev, out, out_length));

	return JC_OK;
}

uint8_t* jc_read_spi(uint32_t address, uint8_t length, uint8_t* out, JoyConDevice* con) {
	uint8_t data[5] = { 0 };
	data[0] = address & 0xff;
	data[1] = (address >> 8) & 0xff;
	data[2] = (address >> 16) & 0xff;
	data[3] = (address >> 24) & 0xff;
	data[4] = length;
	JC_TRY(jc_send_sub_command(JC_SC_READ_SPI, data, 5, out, 20 + length, con));
	return out + 20;
}


JC_RESULT jc_set_input_mode(uint8_t mode, JoyConDevice* con) {
	uint8_t data[1] = { mode };
	JC_TRY(jc_send_sub_command(JC_SC_SET_INPUT_MODE, data, 1, jc_recv_buf, 15, con));
	JC_INPUT_REPLY_ASSERT_ACK(jc_recv_buf, JC_SC_SET_INPUT_MODE);
	return JC_OK;
}

JC_RESULT jc_set_player_lights(uint8_t state, JoyConDevice* con) {
	uint8_t data[1] = { state };
	JC_TRY(jc_send_sub_command(JC_SC_SET_PLAYER_LIGHTS, data, 1, jc_recv_buf, 15, con));
	JC_INPUT_REPLY_ASSERT_ACK(jc_recv_buf, JC_SC_SET_PLAYER_LIGHTS);
	return JC_OK;
}


JC_RESULT jc_apply_input_packet(uint8_t* buf, JoyConDevice* con) {
	switch (((JoyConInput*) buf)->report_id) {
		case JC_INPUT_REPORT_FULL:
		case JC_INPUT_REPORT_REPLY: {
			JoyConInput* in = (JoyConInput*) buf;
			con->state.a = in->buttons_right & JC_INPUT_BUTTON_A ? 1 : 0;
			con->state.b = in->buttons_right & JC_INPUT_BUTTON_B ? 1 : 0;
			con->state.x = in->buttons_right & JC_INPUT_BUTTON_X ? 1 : 0;
			con->state.y = in->buttons_right & JC_INPUT_BUTTON_Y ? 1 : 0;
			con->state.up = in->buttons_left & JC_INPUT_BUTTON_UP ? 1 : 0;
			con->state.down = in->buttons_left & JC_INPUT_BUTTON_DOWN ? 1 : 0;
			con->state.left = in->buttons_left & JC_INPUT_BUTTON_LEFT ? 1 : 0;
			con->state.right = in->buttons_left & JC_INPUT_BUTTON_RIGHT ? 1 : 0;
			con->state.r = in->buttons_right & JC_INPUT_BUTTON_R ? 1 : 0;
			con->state.l = in->buttons_left & JC_INPUT_BUTTON_L ? 1 : 0;
			con->state.zr = in->buttons_right & JC_INPUT_BUTTON_ZR ? 1 : 0;
			con->state.zl = in->buttons_left & JC_INPUT_BUTTON_ZL ? 1 : 0;
			con->state.plus = in->buttons_shared & JC_INPUT_BUTTON_PLUS ? 1 : 0;
			con->state.minus = in->buttons_shared & JC_INPUT_BUTTON_MINUS ? 1 : 0;
			con->state.home = in->buttons_shared & JC_INPUT_BUTTON_HOME ? 1 : 0;
			con->state.capture = in->buttons_shared & JC_INPUT_BUTTON_CAPTURE ? 1 : 0;
			con->state.stick_r = in->buttons_shared & JC_INPUT_BUTTON_STICK_R ? 1 : 0;
			con->state.stick_l = in->buttons_shared & JC_INPUT_BUTTON_STICK_L ? 1 : 0;

			uint16_t uncal_r_x = in->analog_right_hz | ((in->analog_right_shared & 0xf) << 8);
			uint16_t uncal_r_y = (in->analog_right_shared >> 4) | (in->analog_right_vt << 4);
			uint16_t uncal_l_x = in->analog_left_hz | ((in->analog_left_shared & 0xf) << 8);
			uint16_t uncal_l_y = (in->analog_left_shared >> 4) | (in->analog_left_vt << 4);

			con->state.analog_r_x = fmin(fmax((float) uncal_r_x - (float) con->analog_r_cal.x_min, 0.0f) / ((float) con->analog_r_cal.x_max - (float) con->analog_r_cal.x_min), 1.0f) * 2.0f - 1.0f;
			con->state.analog_r_y = fmin(fmax((float) uncal_r_y - (float) con->analog_r_cal.y_min, 0.0f) / ((float) con->analog_r_cal.y_max - (float) con->analog_r_cal.y_min), 1.0f) * 2.0f - 1.0f;
			con->state.analog_l_x = fmin(fmax((float) uncal_l_x - (float) con->analog_l_cal.x_min, 0.0f) / ((float) con->analog_l_cal.x_max - (float) con->analog_l_cal.x_min), 1.0f) * 2.0f - 1.0f;
			con->state.analog_l_y = fmin(fmax((float) uncal_l_y - (float) con->analog_l_cal.y_min, 0.0f) / ((float) con->analog_l_cal.y_max - (float) con->analog_l_cal.y_min), 1.0f) * 2.0f - 1.0f;

			break;
		}
	}

	return JC_OK;
}

JC_RESULT jc_wait_for_input(JoyConDevice* con, uint16_t timeout) {
	JC_HID_TRY(hid_read_timeout(con->dev, jc_recv_buf, 13, timeout));
	JC_TRY(jc_apply_input_packet(jc_recv_buf, con));
	return JC_OK;
}
