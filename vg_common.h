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


#ifndef VG_COMMON_H
#define VG_COMMON_H


#include <stdint.h>
#include <stdio.h>


#define VG_RESULT uint8_t
#define VG_OK 1
#define VG_ERR 0

#define VG_DEBUG_LOG(...) { printf("[\033[34mDEBUG (VG)\033[0m] (%s:%d): ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); }
#define VG_ERROR_LOG(...) { printf("[\033[31mERROR (VG)\033[0m] (%s:%d): ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); }

#define VG_UNWRAP(n, f) { n = f; if ((uintptr_t) n == VG_ERR) { VG_ERROR_LOG("Operation \"" #f "\" failed"); exit(1); } }
#define VG_EXPECT(f) { VG_RESULT temp = (VG_RESULT) f; if (temp == VG_ERR) { VG_ERROR_LOG("Operation \"" #f "\" failed"); exit(1); } }
#define VG_TRY_UNWRAP(n, f) { n = f; if ((uintptr_t) n == VG_ERR) { return VG_ERR; } }
#define VG_TRY(f) { VG_RESULT temp = (VG_RESULT) f; if (temp == VG_ERR) { return VG_ERR; } }


#endif
