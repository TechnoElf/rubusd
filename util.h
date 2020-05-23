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


#ifndef UTIL_H
#define UTIL_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#define DEBUG 1

#define DEBUG_LOG(...) { printf("[\033[34mDEBUG\033[0m] (%s:%d): ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); }
#define ERROR_LOG(...) { printf("[\033[31mERROR\033[0m] (%s:%d): ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); }

#define UNWRAP(n, f) { n = f; if (n < 0) { ERROR_LOG("Operation \"" #f "\" failed"); exit(1); } }
#define EXPECT(f) { int temp = f; if (temp < 0) { ERROR_LOG("Operation \"" #f "\" failed"); exit(1); } }
#define TRY_UNWRAP(n, f) { n = f; if (n < 0) { return -1; } }
#define TRY(f) { int temp = f; if (temp < 0) { return -1; } }


#endif
