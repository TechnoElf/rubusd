#
# rubusd - A deamon for communicating with Joy-Con devices
# Copyright (C) 2020 TechnoElf
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#


NAME = rubusd
SOURCES = main.c joycon.c hid.c vg_uinput.c vg_usb.c
HEADERS = util.h joycon.h hidapi.h vg_uinput.h vg_common.h vg_usb.h

LIBS = -lm -ludev
FLAGS = -g

all: $(NAME)

run: all
	./$(NAME)

$(NAME): $(SOURCES) $(HEADERS)
	gcc $(LIBS) $(FLAGS) -o $@ $(SOURCES)
