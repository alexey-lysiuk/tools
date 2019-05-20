/*
 * Resource dumper for YoYo Games' GameMaker executables
 * Copyright (C) 2011  Zach Reedy
 * Copyright (C) 2015  Alexey Lysiuk
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Credit to IsmAvatar for GMKrypt documentation

#include <iostream>
#include "obfuscation.hpp"

void GmObfuscation::GenerateTable() {
	int a, b, j, t;

	a = (seed % 250) + 6;
	b = seed / 250;

	for(int i = 0; i < 256; i++)
		swapTable[0][i] = i;

	for(int i = 1; i < 10001; i++) {
		j = 1 + ((i * a + b) % 254);
		t = swapTable[0][j];
		swapTable[0][j] = swapTable[0][j + 1];
		swapTable[0][j + 1] = t;
	}

	for(int i = 0; i < 256; i++)
		swapTable[1][swapTable[0][i]] = i;
}
