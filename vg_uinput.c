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


#include "vg_uinput.h"

#include "util.h"

#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


struct input_event ui_event = {0};


#define VG_UI_CREATE_IOCTL(d, n, v) { if (ioctl(d->dev, n, v) < 0) { free(d); return VG_ERR; } }
VGUIDevice* vg_ui_create(uint16_t vid, uint16_t pid, const char* name) {
	VGUIDevice* vg_ui = calloc(1, sizeof(VGUIDevice));
	vg_ui->dev = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (vg_ui->dev < 0) {
		free(vg_ui);
		return VG_ERR;
	}

	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_EVBIT, EV_KEY);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_EVBIT, EV_ABS);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_EVBIT, EV_SYN);

	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_A);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_B);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_X);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_Y);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_LEFT);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_RIGHT);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_FORWARD);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_BACK);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_TL);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_TR);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_TL2);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_TR2);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_START);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_SELECT);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_MODE);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_EXTRA);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_THUMBL);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_KEYBIT, BTN_THUMBR);

	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_ABSBIT, ABS_HAT0X);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_ABSBIT, ABS_HAT0Y);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_ABSBIT, ABS_HAT1X);
	VG_UI_CREATE_IOCTL(vg_ui, UI_SET_ABSBIT, ABS_HAT1Y);

	struct uinput_abs_setup abs_config = {0};
	abs_config.code = ABS_HAT0X;
	abs_config.absinfo.minimum = INT16_MIN + 1;
	abs_config.absinfo.maximum = INT16_MAX;
	abs_config.absinfo.fuzz = 256;
	abs_config.absinfo.flat = 2048;
	VG_UI_CREATE_IOCTL(vg_ui, UI_ABS_SETUP, &abs_config);
	abs_config.code = ABS_HAT0Y;
	VG_UI_CREATE_IOCTL(vg_ui, UI_ABS_SETUP, &abs_config);
	abs_config.code = ABS_HAT1X;
	VG_UI_CREATE_IOCTL(vg_ui, UI_ABS_SETUP, &abs_config);
	abs_config.code = ABS_HAT1Y;
	VG_UI_CREATE_IOCTL(vg_ui, UI_ABS_SETUP, &abs_config);

	struct uinput_setup uinput_config = {0};
	snprintf(uinput_config.name, UINPUT_MAX_NAME_SIZE, name);
	uinput_config.id.bustype = BUS_BLUETOOTH;
	uinput_config.id.vendor = vid;
	uinput_config.id.product = pid;
	uinput_config.id.version = 1;

	VG_UI_CREATE_IOCTL(vg_ui, UI_DEV_SETUP, &uinput_config);
	if (ioctl(vg_ui->dev, UI_DEV_CREATE) < 0) {
		free(vg_ui);
		return VG_ERR;
	}
	return vg_ui;
}

VG_RESULT vg_ui_destroy(VGUIDevice* vg_ui) {
	VG_UI_TRY(ioctl(vg_ui->dev, UI_DEV_DESTROY));
	free(vg_ui);
	return VG_OK;
}


VG_RESULT vg_ui_button(uint16_t code, uint8_t val, VGUIDevice* vg_ui) {
	ui_event.type = EV_KEY;
	ui_event.code = code;
	ui_event.value = val;
	VG_UI_TRY(write(vg_ui->dev, &ui_event, sizeof(ui_event)));
	return VG_OK;
}

VG_RESULT vg_ui_axis(uint16_t code, int16_t val, VGUIDevice* vg_ui) {
	ui_event.type = EV_ABS;
	ui_event.code = code;
	ui_event.value = val;
	VG_UI_TRY(write(vg_ui->dev, &ui_event, sizeof(ui_event)));
	return VG_OK;
}

VG_RESULT vg_ui_flush(VGUIDevice* vg_ui) {
	ui_event.type = EV_SYN;
	ui_event.code = 0;
	ui_event.value = 0;
	VG_UI_TRY(write(vg_ui->dev, &ui_event, sizeof(ui_event)));
	return VG_OK;
}
