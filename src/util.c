// ttf2psf - A simple, working utility to generate Linux console fonts
// Copyright (C) 2023  Nate Choe
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILIY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>

#include <util.h>

uint32_t htop32(uint32_t n) {
	uint8_t ret[sizeof n];
	for (int i = 0; i < sizeof n; ++i) {
		ret[i] = n & 0xff;
		n >>= 8;
	}
	return * (uint32_t *) ret;
}
