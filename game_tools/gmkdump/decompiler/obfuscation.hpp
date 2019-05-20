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

#ifndef __OBFUSCATION_HPP
#define __OBFUSCATION_HPP

#include <iostream>

class GmObfuscation {
private:
	int swapTable[2][256];
	int seed;

	void GenerateTable();

public:
	GmObfuscation(int _seed) : seed(_seed) { GenerateTable(); }

	void SetSeed(int _seed) { seed = _seed; GenerateTable(); }
	const int GetSeed() const { return seed; }

	const unsigned char GetByte(size_t i) const { return swapTable[1][i]; }
};

#endif
