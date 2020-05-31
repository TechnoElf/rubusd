#!/bin/bash

sleep 10

mkdir -p /sys/kernel/config/usb_gadget/rubusd
cd /sys/kernel/config/usb_gadget/rubusd

sudo su

echo 0x1d6b > idVendor
echo 0x0104 > idProduct
echo 0x0100 > bcdDevice
echo 0x0200 > bcdUSB
echo 0xEF > bDeviceClass
echo 0x02 > bDeviceSubClass
echo 0x01 > bDeviceProtocol

mkdir -p strings/0x409
echo "0" > strings/0x409/serialnumber
echo "TechnoElf" > strings/0x409/manufacturer
echo "rubusd" > strings/0x409/product

mkdir functions/hid.usb0
echo 0 > functions/hid.usb0/protocol
echo 0 > functions/hid.usb0/subclass
echo 3 > functions/hid.usb0/report_length

echo "05010904A1010509190129181500250175019518050181021581257F0901A1000930093109320933750895048102C0C0" | xxd -r -ps > functions/hid.usb0/report_desc

mkdir configs/c.1
mkdir configs/c.1/strings/0x409

echo 0x80 > configs/c.1/bmAttributes
echo 500 > configs/c.1/MaxPower
echo "rubusd configuration" > configs/c.1/strings/0x409/configuration

ln -s functions/hid.usb0 configs/c.1

ls /sys/class/udc > UDC

sleep 10
