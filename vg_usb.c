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


#include "vg_usb.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


VGUSBDevice* vg_usb_create() {
	VGUSBDevice* vg_usb = calloc(1, sizeof(VGUSBDevice));
	vg_usb->dev = fopen("/dev/hidg0", "w");
	if (vg_usb->dev == VG_ERR) {
		VG_ERROR_LOG("Failed to open /dev/hidg0");
		free(vg_usb);
		return VG_ERR;
	}

	return vg_usb;
}

VG_RESULT vg_usb_destroy(VGUSBDevice* vg_usb) {
	fclose(vg_usb->dev);
	free(vg_usb);
	return VG_OK;
}


VG_RESULT vg_usb_send(VGUSBPacket* packet, VGUSBDevice* vg_usb) {
	if (fwrite(packet, 1, sizeof(VGUSBPacket), vg_usb->dev) != sizeof(VGUSBPacket)) {
		VG_ERROR_LOG("Failed to write packet to usb device file");
		return VG_ERR;
	}
	fflush(vg_usb->dev);
	return VG_OK;
}
