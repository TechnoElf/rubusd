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


#ifndef VG_UINPUT_H
#define VG_UINPUT_H


#include "vg_common.h"

#include <linux/input.h>


#define VG_UI_TRY(f) { int temp = (int) f; if (temp < 0) { return VG_ERR; } }
#define VG_UI_TRY_UNWRAP(n, f) { n = f; if ((int) n < 0) { return VG_ERR; } }


typedef struct VGUIDevice {
	int dev;
} VGUIDevice;


VGUIDevice* vg_ui_create(uint16_t vid, uint16_t pid, const char* name);
VG_RESULT vg_ui_destroy(VGUIDevice* vg_ui);

VG_RESULT vg_ui_button(uint16_t code, uint8_t val, VGUIDevice* vg_ui);
VG_RESULT vg_ui_axis(uint16_t code, int8_t val, VGUIDevice* vg_ui);
VG_RESULT vg_ui_flush(VGUIDevice* vg_ui);


#endif
