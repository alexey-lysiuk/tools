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

#include "gm80.hpp"

bool Gm80::FindGameData(GmkStream* const exeHandle)
{
	exeHandle->SetPosition(0);

	for (size_t pos = 0; pos < exeHandle->GetLength(); pos += 16)
	{
		if (   exeHandle->ReadDword() == GMK_MAGIC
			&& exeHandle->ReadDword() == GMK_VERSION
			&& exeHandle->ReadDword() == 0
			&& exeHandle->ReadDword() == GMK_VERSION)
		{
			// Position read pointer right after the magic
			exeHandle->SetPosition(exeHandle->GetPosition() - 12);
			return true;
		}
	}

	return false;
}
