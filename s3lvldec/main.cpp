/*
 * Level decompiler for Amanita Design's Samorost 3
 * Copyright (C) 2016  Alexey Lysiuk
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

#include <cstdlib>
#include <iostream>

#include "s3level.h"


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " <file.lvl> ...\n" << std::endl;
		return EXIT_SUCCESS;
	}

	for (int i = 1; i < argc; ++i)
	{
		try
		{
			S3::Level level(argv[i]);

			// TODO: dump content
		}
		catch (const std::exception& ex)
		{
			std::cout << "ERROR: " << ex.what() << std::endl;
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
