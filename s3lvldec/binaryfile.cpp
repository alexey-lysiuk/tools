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

#include "binaryfile.h"

#include <cassert>

#ifdef _MSC_VER
#include <direct.h>
#else
#include <sys/stat.h>
inline int _mkdir(const char* const name) { return mkdir(name, 0755); }
#endif

static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 floating point format is required");

namespace S3
{

void BinaryFile::open(const char* const filename)
{
	if (nullptr != m_file)
	{
		close();
	}

	assert(nullptr != filename);
	m_file = fopen(filename, "rb");

	if (nullptr == m_file)
	{
		throw std::runtime_error("Failed to open file");
	}
}

void BinaryFile::close()
{
	if (nullptr != m_file)
	{
		fclose(m_file);
		m_file = nullptr;
	}
}

void BinaryFile::read(void* const buffer, const size_t size)
{
	if (nullptr == m_file)
	{
		throw std::runtime_error("File is not opened");
	}

	if (1 != fread(buffer, size, 1, m_file))
	{
		throw std::runtime_error("Failed to read from file");
	}
}

std::string BinaryFile::readUTF()
{
	const uint16_t length = readU16();
	std::string result(length, '\0');

	for (size_t i = 0; i < length; i++)
	{
		result[i] = read<char>();
	}

	return result;
}

long BinaryFile::pos() const
{
	return ftell(m_file);
}


void SaveToFile(const char* const filename, const ByteArray& buffer)
{
	assert(nullptr != filename);

	std::vector<std::string> nameParts;
	std::string currentNamePart;

	for (size_t i = 0, count = strlen(filename); i < count; ++i)
	{
		const char ch = filename[i];

		if ('\\' == ch || '/' == ch)
		{
			if (currentNamePart.empty())
			{
				continue;
			}

			nameParts.push_back(currentNamePart);
			currentNamePart.clear();
		}
		else
		{
			currentNamePart += ch;
		}
	}

	std::string dirPath;

	for (const std::string& namePart : nameParts)
	{
		_mkdir(namePart.c_str());
		dirPath += namePart + '/';
	}

	FILE* const file = fopen(filename, "wb");
	if (nullptr == file)
	{
		throw std::runtime_error("Failed to open file");
	}

	if (!buffer.empty()
		&& 1 != fwrite(&buffer[0], buffer.size(), 1, file))
	{
		throw std::runtime_error("Failed to write to file");
	}

	if (0 != fclose(file))
	{
		throw std::runtime_error("Failed to close to file");
	}
}

void SaveToFile(const std::string& filename, const ByteArray& buffer)
{
	SaveToFile(filename.c_str(), buffer);
}

} // namespace S3
