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


#ifndef JOYCON_H
#define JOYCON_H


#include "hidapi.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>


#define JC_BT_VENDOR 0x057e
#define JC_BT_PRODUCT_LEFT 0x2006
#define JC_BT_PRODUCT_RIGHT 0x2007
#define JC_BT_PRODUCT_PRO 0x2009

#define JC_C_SUBCOMMAND 0x01
#define JC_C_IR_NFC_UPDATE 0x03
#define JC_C_RUMBLE 0x10
#define JC_C_IR_NFC 0x11

#define JC_SC_GET_STATE 0x00
#define JC_SC_BT_PAIR 0x01
#define JC_SC_GET_INFO 0x02
#define JC_SC_SET_INPUT_MODE 0x03
#define JC_SC_READ_SPI 0x10
#define JC_SC_WRITE_SPI 0x11
#define JC_SC_ERASE_SPI 0x12
#define JC_SC_RESET_NFC_IR 0x20
#define JC_SC_SET_NFC_IR_CONF 0x21
#define JC_SC_SET_NFC_IR_STATE 0x22
#define JC_SC_SET_PLAYER_LIGHTS 0x30
#define JC_SC_GET_PLAYER_LIGHTS 0x31
#define JC_SC_SET_HOME_LIGHT 0x38
#define JC_SC_ENABLE_IMU 0x40

#define JC_INPUT_MODE_FULL 0x30
#define JC_INPUT_MODE_SIMPLE 0x3f

#define JC_INPUT_REPORT_REPLY 0x21
#define JC_INPUT_REPORT_FULL 0x30
#define JC_INPUT_REPORT_SIMPLE 0x3f

typedef struct JoyConInput {
	uint8_t report_id;
	uint8_t timer;
	uint8_t info;
	uint8_t buttons_right;
	uint8_t buttons_shared;
	uint8_t buttons_left;
	uint8_t analog_left_hz;
	uint8_t analog_left_shared;
	uint8_t analog_left_vt;
	uint8_t analog_right_hz;
	uint8_t analog_right_shared;
	uint8_t analog_right_vt;
	uint8_t rumble;
} JoyConInput;

#define JC_INPUT_BUTTON_Y 0x01
#define JC_INPUT_BUTTON_X 0x02
#define JC_INPUT_BUTTON_B 0x04
#define JC_INPUT_BUTTON_A 0x08
#define JC_INPUT_BUTTON_UP 0x02
#define JC_INPUT_BUTTON_DOWN 0x01
#define JC_INPUT_BUTTON_LEFT 0x08
#define JC_INPUT_BUTTON_RIGHT 0x04
#define JC_INPUT_BUTTON_R 0x40
#define JC_INPUT_BUTTON_L 0x40
#define JC_INPUT_BUTTON_ZR 0x80
#define JC_INPUT_BUTTON_ZL 0x80
#define JC_INPUT_BUTTON_PLUS 0x02
#define JC_INPUT_BUTTON_MINUS 0x01
#define JC_INPUT_BUTTON_HOME 0x10
#define JC_INPUT_BUTTON_CAPTURE 0x20
#define JC_INPUT_BUTTON_STICK_R 0x04
#define JC_INPUT_BUTTON_STICK_L 0x08

typedef struct JoyConInputReply {
	JoyConInput common;
	uint8_t ack;
	uint8_t sc_id;
} JoyConInputReply;

#define JC_INPUT_REPLY_ACK_ACK 0x80

typedef struct JoyConInputFull {
	JoyConInput common;
} JoyConInputFull;

typedef struct JoyConState {
	uint8_t a : 1;
	uint8_t b : 1;
	uint8_t x : 1;
	uint8_t y : 1;
	uint8_t up : 1;
	uint8_t down : 1;
	uint8_t left : 1;
	uint8_t right : 1;
	uint8_t r : 1;
	uint8_t l : 1;
	uint8_t zr : 1;
	uint8_t zl : 1;
	uint8_t plus : 1;
	uint8_t minus : 1;
	uint8_t home : 1;
	uint8_t capture : 1;
	uint8_t stick_r : 1;
	uint8_t stick_l : 1;
	float analog_r_x;
	float analog_r_y;
	float analog_l_x;
	float analog_l_y;
} JoyConState;

typedef struct JoyConAnalogCal {
	uint16_t x_max;
	uint16_t y_max;
	uint16_t x_cen;
	uint16_t y_cen;
	uint16_t x_min;
	uint16_t y_min;
} JoyConAnalogCal;

typedef enum JoyConType { JoyConLeft, JoyConRight, JoyConPro } JoyConType;

typedef struct JoyConDevice {
	hid_device* dev;
	uint32_t address;
	time_t last_seen;
	JoyConType type;
	JoyConState state;
	JoyConAnalogCal analog_l_cal;
	JoyConAnalogCal analog_r_cal;
} JoyConDevice;


#define JC_RESULT uint8_t
#define JC_OK 1
#define JC_ERR 0

#define JC_DEBUG_LOG(...) { printf("[\033[34mDEBUG (JC)\033[0m] (%s:%d): ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); }
#define JC_ERROR_LOG(...) { printf("[\033[31mERROR (JC)\033[0m] (%s:%d): ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); }

#define JC_UNWRAP(n, f) { n = f; if ((uintptr_t) n == JC_ERR) { JC_ERROR_LOG("Operation \"" #f "\" failed"); exit(1); } }
#define JC_EXPECT(f) { JC_RESULT temp = (JC_RESULT) f; if (temp == JC_ERR) { JC_ERROR_LOG("Operation \"" #f "\" failed"); exit(1); } }
#define JC_TRY_UNWRAP(n, f) { n = f; if ((uintptr_t) n == JC_ERR) { return JC_ERR; } }
#define JC_TRY(f) { JC_RESULT temp = (JC_RESULT) f; if (temp == JC_ERR) { return JC_ERR; } }
#define JC_HID_TRY(f) { int temp = (int) f; if (temp < 0) { return JC_ERR; } } 

#define JC_INPUT_REPLY_ASSERT_ACK(reply, id) { JoyConInputReply* temp = (JoyConInputReply*) reply; if (temp->common.report_id == JC_INPUT_REPORT_REPLY && temp->ack & JC_INPUT_REPLY_ACK_ACK && temp->sc_id == id) {} else { JC_ERROR_LOG("Unexpected reply received to 0x%02X subcommand:", id); jc_print_buffer((uint8_t*) reply, 15); } }


uint32_t jc_parse_address(const wchar_t* address);
JoyConDevice* jc_create(const char* path, uint32_t address);
JC_RESULT jc_destroy(JoyConDevice* con);
JC_RESULT jc_flush_input(JoyConDevice* con);
JC_RESULT jc_calibrate(JoyConDevice* con);

JC_RESULT jc_send_command(uint8_t command, uint8_t* data, uint16_t length, uint8_t* out, uint16_t out_length, JoyConDevice* con);
JC_RESULT jc_send_sub_command(uint8_t command, uint8_t* data, uint16_t length, uint8_t* out, uint16_t out_length, JoyConDevice* con);
uint8_t* jc_read_spi(uint32_t address, uint8_t length, uint8_t* buffer, JoyConDevice* con);

JC_RESULT jc_set_input_mode(uint8_t mode, JoyConDevice* con);
JC_RESULT jc_set_player_lights(uint8_t state, JoyConDevice* con);

JC_RESULT jc_wait_for_input(JoyConDevice* con, uint16_t timeout);
JC_RESULT jc_input_maybe(JoyConDevice* con);


#endif
