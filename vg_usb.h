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


#ifndef VG_USB_H
#define VG_USB_H


#include "vg_common.h"


typedef struct VGUSBDevice {
	FILE* dev;
} VGUSBDevice;

typedef struct VGUSBPacket {
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
	int8_t analog_r_x;
	int8_t analog_r_y;
	int8_t analog_l_x;
	int8_t analog_l_y;
} VGUSBPacket;


VGUSBDevice* vg_usb_create();
VG_RESULT vg_usb_destroy(VGUSBDevice* vg_usb);

VG_RESULT vg_usb_send(VGUSBPacket packet, VGUSBDevice* vg_usb);


#endif
