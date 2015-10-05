/* hexdump.h */
/*
    This file is part of the AVR-Crypto-Lib.
    Copyright (C) 2015 Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HEXDUMP_H_
#define HEXDUMP_H_

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

void hexdump_block(const void* data, size_t length, uint8_t indent, uint8_t width);

#define DUMP_LEN(label, data, length)           \
    do {                                        \
        printf("%s:\n", (label));               \
        hexdump_block((data), (length), 4, 8);  \
        puts("\n");                             \
    } while (0)

#define STR(x) #x
#define XSTR(x) STR(x)

#define DUMP(x) DUMP_LEN(XSTR(x), x, sizeof(x))


#endif /* HEXDUMP_H_ */
