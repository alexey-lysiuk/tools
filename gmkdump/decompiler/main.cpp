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

#include <cstdlib>
#include <cstdio>
#include <cerrno>

#ifdef _MSC_VER
#	include <direct.h>
#	include "msstdint.h"
#	define PATH_MAX 1024
#	define mkdir(NAME, DUMMY) mkdir(NAME)
#	if _MSC_VER < 1900
#		define snprintf _snprintf
#	endif // _MSC_VER < 1900
#else // !_MSC_VER
#	include <cstdint>
#	include <climits>
#	include <sys/stat.h>
#endif // _MSC_VER

#include "exe.hpp"

#define OpenFile()                                    \
	FILE* const file = fopen(fileName, "wb");         \
	if (NULL == file)                                 \
	{                                                 \
		printf("Failed to open file %s\n", fileName); \
		return false;                                 \
	}

#define WriteFile(DATA, SIZE)                             \
	if (1 != fwrite((DATA), (SIZE), 1, file))             \
	{                                                     \
		printf("Failed to write to file %s\n", fileName); \
		fclose(file);                                     \
		return false;                                     \
	}

#define CloseFile()                                    \
	if (0 != fclose(file))                             \
	{                                                  \
		printf("Failed to close file %s\n", fileName); \
		return false;                                  \
	}

#define MakeDirectory(NAME)                                  \
	{                                                        \
		const int result = mkdir((NAME), 0777);              \
		if (0 != result && -1 == result && EEXIST != errno)  \
		{                                                    \
			printf("Failed to create directory %s", (NAME)); \
			return false;                                    \
		}                                                    \
	}

namespace
{

bool SaveScripts(const Gmk& gmkHandle)
{
	MakeDirectory("scripts");

	for (size_t i = 0, count = gmkHandle.scripts.size(); i < count; ++i)
	{
		const Script* const script = gmkHandle.scripts[i];
		if (NULL == script)
		{
			continue;
		}

		char fileName[PATH_MAX];
		snprintf(fileName, sizeof fileName, "scripts/%s.txt", script->name.c_str());
		printf("Saving %s...\n", fileName);

		OpenFile();
		WriteFile(script->value.c_str(), script->value.size());
		CloseFile();
	}

	return true;
}

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

	TGAHeader header = {};
	header.img_type = 2; // uncompressed true-color image
	header.width = width;
	header.height = height;
	header.bpp = 32;

	OpenFile();
	WriteFile(&header, sizeof header);

	const char* buffer = (const char*)image->data->GetBuffer();
	const size_t rowSize = width * 4;

	for (int y = height - 1; y >= 0; --y)
	{
		WriteFile(&buffer[y * rowSize], rowSize);
	}

	const char FOOTER[26] = "\0\0\0\0\0\0\0\0TRUEVISION-XFILE.";
	WriteFile(FOOTER, sizeof FOOTER);

	CloseFile();

	return true;
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

template <typename DataType>
bool SaveResources(const std::vector<DataType*>& resources, const char* const resourceType)
{
	MakeDirectory(resourceType);

	for (size_t i = 0, count = resources.size(); i < count; ++i)
	{
		const DataType* const resource = resources[i];
		if (NULL == resource)
		{
			continue;
		}

		char fileName[PATH_MAX];
		snprintf(fileName, sizeof fileName, "%s/%s", resourceType, resource->fileName.c_str());
		printf("Saving %s...\n", fileName);

		if (!resource->data->Save(fileName, FMODE_BINARY))
		{
			return false;
		}
	}

	return true;
}

bool SaveResources(const Gmk& gmkHandle)
{
	return SaveScripts(gmkHandle)
		&& SaveResources(gmkHandle.includes, "includes")
		&& SaveBackgrounds(gmkHandle)
		&& SaveSprites(gmkHandle)
		&& SaveResources(gmkHandle.sounds, "sounds");
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
