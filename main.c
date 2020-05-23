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
#include "vg_uinput.h"
#include "vg_usb.h"
#include "util.h"

#include <signal.h>
#include <unistd.h>
#include <string.h>


typedef struct RubusdConf {
	uint8_t target;
	uint8_t silent;
} RubusdConf;


void signal_handler(int signal);
RubusdConf parse_args(int argc, char* argv[]);

int dev_list_contains(uint32_t address);
void dev_list_connect_new();
void dev_list_remove(uint8_t i);
void dev_list_empty();


#define MAX_CONNECTIONS 15
JoyConDevice* joycon[MAX_CONNECTIONS] = {0};
uint8_t joycon_count = 0;

VGUIDevice* vg_ui = 0;
VGUSBDevice* vg_usb = 0;
VGUSBPacket vg_usb_state = {0};

uint16_t connect_timeout = 0;

RubusdConf conf = {0};


int main(int argc, char* argv[]) {
	signal(SIGINT, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);

	conf = parse_args(argc, argv);

	if (conf.silent == 0) {
		printf("rubusd - Copyright (C) 2020 TechnoElf\nThis program is licensed under the terms of the GPLv3 and comes with ABSOLUTELY NO WARRANTY.\nThis is free software, and you are welcome to redistribute it under certain conditions.\nSee <https://www.gnu.org/licenses/> for details.\n\n");
	}

	if (conf.target == 0) {
		VG_UNWRAP(vg_ui, vg_ui_create(JC_BT_VENDOR, JC_BT_PRODUCT_PRO, "rubusd"));
	} else if (conf.target == 1) {
		VG_UNWRAP(vg_usb, vg_usb_create());
	}

	while (1) {
		for (uint8_t i = 0; i < joycon_count; i++) {
			if (jc_wait_for_input(joycon[i], 100) == JC_ERR) {
				if (conf.silent == 0) {
					DEBUG_LOG("Joy-Con with address %08x disconnected", joycon[i]->address);
				}
				dev_list_remove(i);
				continue;
			}

			if (joycon[i]->type == JoyConLeft) {
				if (conf.target == 0) {
					VG_EXPECT(vg_ui_button(BTN_FORWARD, joycon[i]->state.up, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_BACK, joycon[i]->state.down, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_LEFT, joycon[i]->state.left, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_RIGHT, joycon[i]->state.right, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_TL, joycon[i]->state.l, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_TL2, joycon[i]->state.zl, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_SELECT, joycon[i]->state.minus, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_EXTRA, joycon[i]->state.capture, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_THUMBL, joycon[i]->state.stick_l, vg_ui));

					VG_EXPECT(vg_ui_axis(ABS_HAT0X, (int16_t) (joycon[i]->state.analog_l_x * INT8_MAX), vg_ui));
					VG_EXPECT(vg_ui_axis(ABS_HAT0Y, (int16_t) (joycon[i]->state.analog_l_y * INT8_MAX), vg_ui));
				} else if (conf.target == 1) {
					vg_usb_state.up = joycon[i]->state.up;
					vg_usb_state.down = joycon[i]->state.down;
					vg_usb_state.left = joycon[i]->state.left;
					vg_usb_state.right = joycon[i]->state.right;
					vg_usb_state.l = joycon[i]->state.l;
					vg_usb_state.zl = joycon[i]->state.zl;
					vg_usb_state.minus = joycon[i]->state.minus;
					vg_usb_state.capture = joycon[i]->state.capture;
					vg_usb_state.stick_l = joycon[i]->state.stick_l;

					vg_usb_state.analog_l_x = (int8_t) (joycon[i]->state.analog_l_x * INT8_MAX);
					vg_usb_state.analog_l_y = (int8_t) (joycon[i]->state.analog_l_y * INT8_MAX);
				} 
			} else if (joycon[i]->type == JoyConRight) {
				if (conf.target == 0) {
					VG_EXPECT(vg_ui_button(BTN_A, joycon[i]->state.a, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_B, joycon[i]->state.b, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_X, joycon[i]->state.x, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_Y, joycon[i]->state.y, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_TR, joycon[i]->state.r, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_TR2, joycon[i]->state.zr, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_START, joycon[i]->state.plus, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_MODE, joycon[i]->state.home, vg_ui));
					VG_EXPECT(vg_ui_button(BTN_THUMBR, joycon[i]->state.stick_r, vg_ui));

					VG_EXPECT(vg_ui_axis(ABS_HAT1X, (int16_t) (joycon[i]->state.analog_r_x * INT8_MAX), vg_ui));
					VG_EXPECT(vg_ui_axis(ABS_HAT1Y, (int16_t) (joycon[i]->state.analog_r_y * INT8_MAX), vg_ui));
				} else if (conf.target == 1) {
					vg_usb_state.a = joycon[i]->state.a;
					vg_usb_state.b = joycon[i]->state.b;
					vg_usb_state.x = joycon[i]->state.x;
					vg_usb_state.y = joycon[i]->state.y;
					vg_usb_state.r = joycon[i]->state.r;
					vg_usb_state.zr = joycon[i]->state.zr;
					vg_usb_state.plus = joycon[i]->state.plus;
					vg_usb_state.home = joycon[i]->state.home;
					vg_usb_state.stick_r = joycon[i]->state.stick_r;

					vg_usb_state.analog_r_x = (int8_t) (joycon[i]->state.analog_r_x * INT8_MAX);
					vg_usb_state.analog_r_y = (int8_t) (joycon[i]->state.analog_r_y * INT8_MAX);
				} 
			}
		}
		
		if (joycon_count > 0) {
			if (conf.target == 0) {
				VG_EXPECT(vg_ui_flush(vg_ui));
			} else if (conf.target == 1) {
				VG_EXPECT(vg_usb_send(vg_usb_state, vg_usb));
			}

			connect_timeout++;
			if (connect_timeout > 300) {
				connect_timeout = 0;
				dev_list_connect_new();
			}
		} else {
			dev_list_connect_new();
		}
	}
}


void signal_handler(int sig_num) {
	switch (sig_num) {
		case SIGINT:
		case SIGKILL:
		case SIGTERM:
			if (conf.silent == 0) {
				printf("\n");
				DEBUG_LOG("Exiting...");
			}

			if (conf.target == 0) {
				VG_EXPECT(vg_ui_destroy(vg_ui));
			} else if (conf.target == 1) {
				VG_EXPECT(vg_usb_destroy(vg_usb));
			}
			dev_list_empty();
			hid_exit();

			exit(0);
			break;
		case SIGHUP:
			break;
	}
}

RubusdConf parse_args(int argc, char* argv[]) {
	RubusdConf conf;
	conf.target = 0;
	conf.silent = 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--uinput") == 0) {
			conf.target = 0;
		} else if (strcmp(argv[i], "--usb") == 0) {
			conf.target = 1;
		} else if (strcmp(argv[i], "--silent") == 0 || strcmp(argv[i], "-s") == 0) {
			conf.silent = 1;
		} else {
			ERROR_LOG("Argument \"%s\" not recognised", argv[i]);
		}
	}

	return conf;
}


int dev_list_contains(uint32_t address) {
	for (uint8_t i = 0; i < joycon_count; i++) {
		if (joycon[i] != 0 && address == joycon[i]->address) {
			return 1;
		}
	}
	return 0;
}

void dev_list_connect_new() {
	struct hid_device_info* devs = hid_enumerate(0x057e, 0x0);
	struct hid_device_info* cur_dev = devs;

	while (cur_dev) {
		if (cur_dev->product_id == 0x2006 || cur_dev->product_id == 0x2007) {
			uint32_t address = jc_parse_address(cur_dev->serial_number);
			if (dev_list_contains(address)) {
				cur_dev = cur_dev->next;
				continue;
			}

			if (conf.silent == 0) {
				DEBUG_LOG("Joy-Con Found!\n  name: %ls\n  path: %s\n  address: %ls", cur_dev->product_string, cur_dev->path, cur_dev->serial_number);
			}

			usleep(100 * 1000);
			
			uint8_t next = joycon_count;

			JC_UNWRAP(joycon[next], jc_create(cur_dev->path, address));
			JC_EXPECT(jc_calibrate(joycon[next]));
			JC_EXPECT(jc_set_player_lights(next + 1, joycon[next]));
			JC_EXPECT(jc_set_input_mode(JC_INPUT_MODE_FULL, joycon[next]));

			joycon_count++;
		}

		cur_dev = cur_dev->next;
	}
	
	hid_free_enumeration(devs);
}

void dev_list_empty() {
	for (uint8_t i = 0; i < joycon_count; i++) {
		if (joycon[i] != 0 && joycon[i]->dev != 0) {
			JC_EXPECT(jc_destroy(joycon[i]));
			joycon[i] = 0;
			joycon_count--;
		}
	}
}

void dev_list_remove(uint8_t i) {
	JC_EXPECT(jc_destroy(joycon[i]));
	joycon[i] = 0;
	joycon_count--;

	for (uint8_t j = i; j < joycon_count; j++) {
		if (joycon[j] == 0 && joycon[j + 1] != 0) {
			joycon[j] = joycon[j + 1];
			joycon[j + 1] = 0;
			jc_set_player_lights(j + 1, joycon[j]);
		} else {
			break;
		}
	}
}


