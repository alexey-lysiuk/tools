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

#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#ifdef _MSC_VER
#include <direct.h>
#define PATH_MAX 1024
#define mkdir(NAME, DUMMY) mkdir(NAME)
#else // !_MSC_VER
#include <limits.h>
#include <sys/stat.h>
#endif // _MSC_VER

#include "exe.hpp"

#define WriteFile(DATA, SIZE)                             \
	if (1 != fwrite((DATA), (SIZE), 1, file))             \
	{                                                     \
		printf("Failed to write to file %s\n", fileName); \
		fclose(file);                                     \
		return false;                                     \
	}

#define MakeDirectory(NAME)                                 \
	{                                                       \
		const int result = mkdir(NAME, 0777);               \
		if (0 != result && -1 == result && EEXIST != errno) \
		{                                                   \
			puts("Failed to create directory " NAME);       \
			return false;                                   \
		}                                                   \
	}

namespace
{

template <typename ImageType>
bool SaveImage(const char* const fileName, const ImageType* const image)
{
	const int width = image->width;
	const int height = image->height;

	if (0 >= width || 0 >= height)
	{
		// Valid case, at least for backgrounds
		return true;
	}

#pragma pack(1)
	struct TGAHeader
	{
		uint8_t id_len;
		uint8_t has_cm;
		uint8_t img_type;
		int16_t cm_first;
		int16_t cm_length;
		uint8_t cm_size;

		int16_t x_origin;
		int16_t y_origin;
		int16_t width;
		int16_t height;
		uint8_t bpp;
		uint8_t img_desc;
	};
#pragma pack()

	FILE* const file = fopen(fileName, "wb");
	if (NULL == file)
	{
		return false;
	}

	TGAHeader header = {};
	header.img_type = 2; // uncompressed true-color image
	header.width = width;
	header.height = height;
	header.bpp = 32;

	WriteFile(&header, sizeof header);

	const char* buffer = (const char*)image->data->GetBuffer();
	const size_t rowSize = width * 4;

	for (int y = height - 1; y >= 0; --y)
	{
		WriteFile(&buffer[y * rowSize], rowSize);
	}

	return 0 == fclose(file);
}

bool SaveBackgrounds(const Gmk& gmkHandle)
{
	MakeDirectory("backgrounds");

	for (size_t b = 0, backCount = gmkHandle.backgrounds.size(); b < backCount; ++b)
	{
		const Background* const background = gmkHandle.backgrounds[b];
		if (NULL == background)
		{
			continue;
		}

		char fileName[PATH_MAX];
		snprintf(fileName, sizeof fileName, "backgrounds/%s.tga", background->name.c_str());
		printf("Saving %s...\n", fileName);

		if (!SaveImage(fileName, background))
		{
			return false;
		}
	}

	return true;
}

bool SaveSprites(const Gmk& gmkHandle)
{
	MakeDirectory("sprites");

	for (size_t s = 0, spriteCount = gmkHandle.sprites.size(); s < spriteCount; ++s)
	{
		const Sprite* const sprite = gmkHandle.sprites[s];
		if (NULL == sprite)
		{
			continue;
		}

		for (size_t i = 0, imageCount = sprite->images.size(); i < imageCount; ++i)
		{
			const SubImage* const subImage = sprite->images[i];
			if (NULL == subImage)
			{
				continue;
			}

			char fileName[PATH_MAX];
			snprintf(fileName, sizeof fileName, "sprites/%s#%i.tga", sprite->name.c_str(), i);
			printf("Saving %s...\n", fileName);

			if (!SaveImage(fileName, subImage))
			{
				return false;
			}
		}
	}

	return true;
}

bool SaveSounds(const Gmk& gmkHandle)
{
	MakeDirectory("sounds");

	for (size_t i = 0, count = gmkHandle.sounds.size(); i < count; ++i)
	{
		const Sound* const sound = gmkHandle.sounds[i];
		if (NULL == sound)
		{
			continue;
		}

		char fileName[PATH_MAX];
		snprintf(fileName, sizeof fileName, "sounds/%s", sound->fileName.c_str());
		printf("Saving %s...\n", fileName);

		if (!sound->data->Save(fileName, FMODE_BINARY))
		{
			return false;
		}
	}

	return true;
}

bool SaveResources(const Gmk& gmkHandle)
{
	return SaveBackgrounds(gmkHandle)
		&& SaveSprites(gmkHandle)
		&& SaveSounds(gmkHandle);
}

} // unnamed namespace


int main(int argc, char* argv[])
{
	if (2 != argc)
	{
		printf("Usage: %s file.exe\n", argv[0]);
		return EXIT_SUCCESS;
	}

	GmExe gmExe;
	Gmk gmkHandle;
	gmkHandle.SetDefaults();

	if (!gmExe.Load(argv[1], &gmkHandle, 0))
	{
		printf("Failed to load file %s, exiting\n", argv[1]);
		return EXIT_FAILURE;
	}

	return SaveResources(gmkHandle) ? EXIT_SUCCESS : EXIT_FAILURE;
}
