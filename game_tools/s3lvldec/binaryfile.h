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

#pragma once
#ifndef S3LVLDEC_BINARYFILE_H_INCLUDED
#define S3LVLDEC_BINARYFILE_H_INCLUDED

#include <cstdint>
#include <string>
#include <vector>

enum EndianType
{
	LE,
	BE,

#if defined _M_IX86 || defined _M_X64 || defined __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	NE = LE,
	NNE = BE
#else // Big Endian
	NE = BE,
	NNE = LE
#endif // Little Endian
};

namespace Endian
{

#define ENDIAN_NO_CONVERT(TYPE, ENDIAN)            \
	template <>                                    \
	inline TYPE convert<TYPE, ENDIAN>(TYPE value)  \
	{                                              \
		return value;                              \
	}

#define ENDIAN_STORAGE_CONVERT(TYPE, STORAGE, ENDIAN)                                     \
	template <>                                                                           \
	inline TYPE convert<TYPE, ENDIAN>(const TYPE value)                                   \
	{                                                                                     \
		const STORAGE* const unsignedPointer = reinterpret_cast<const STORAGE*>(&value);  \
		const STORAGE unsignedValue = convert<STORAGE, ENDIAN>(*unsignedPointer);         \
		return *reinterpret_cast<const TYPE*>(&unsignedValue);                            \
	}

	template <typename T, EndianType = LE>
	T convert(T value);
	
	template <>
	inline uint16_t convert<uint16_t, NNE>(const uint16_t value)
	{
		return (value >> 8) 
			|  (value << 8);
	}

	template <>
	inline uint32_t convert<uint32_t, NNE>(uint32_t value)
	{
		return (value >> 24) 
			| ((value<<8) & 0x00FF0000)
			| ((value>>8) & 0x0000FF00)
			|  (value << 24);
	}

	ENDIAN_STORAGE_CONVERT(int16_t, uint16_t, NNE)
	ENDIAN_STORAGE_CONVERT(int32_t, uint32_t, NNE)
	ENDIAN_STORAGE_CONVERT(  float, uint32_t, NNE)

	ENDIAN_NO_CONVERT(uint16_t, NE)
	ENDIAN_NO_CONVERT( int16_t, NE)
	ENDIAN_NO_CONVERT(uint32_t, NE)
	ENDIAN_NO_CONVERT( int32_t, NE)
	ENDIAN_NO_CONVERT(   float, NE)

#undef ENDIAN_STORAGE_CONVERT
#undef ENDIAN_NO_CONVERT

} // namespace Endian

namespace S3
{

typedef std::vector<uint8_t> ByteArray;

class BinaryFile
{
public:
	explicit BinaryFile(const char* filename = nullptr)
	{
		if (nullptr != filename)
		{
			open(filename);
		}
	}

	~BinaryFile()
	{
		close();
	}

	void open(const char* filename);
	void close();

	bool endOfFile() const
	{
		return 0 != feof(m_file);
	}

	template <typename T>
	T read()
	{
		T result;
		read(&result, sizeof result);

		return result;
	}
	
	void read(void* buffer, size_t size);

	void read(ByteArray& buffer)
	{
		if (!buffer.empty())
		{
			read(&buffer[0], buffer.size());
		}
	}

	ByteArray read(size_t size)
	{
		ByteArray result(size);
		read(result);

		return result;
	}

	uint8_t readU8() { return read<uint8_t>(); }
	int8_t  readS8() { return read< int8_t>(); }

#define READ_FUNCTION(NAME, TYPE)                            \
	template <EndianType endian = LE>                        \
	TYPE read##NAME()                                        \
	{                                                        \
		return Endian::convert<TYPE, endian>(read<TYPE>());  \
	}

	READ_FUNCTION(  U16, uint16_t)
	READ_FUNCTION(  S16,  int16_t)
	READ_FUNCTION(  U32, uint32_t)
	READ_FUNCTION(  S32,  int32_t)
	READ_FUNCTION(Float,    float)

#undef READ_FUNCTION

	std::string readUTF();

	template <EndianType endian = LE>
	ByteArray readBuffer()
	{
		ByteArray result(readU32<endian>());
		read(result);

		return result;
	}

	long pos() const;

private:
	FILE* m_file = nullptr;
};

void SaveToFile(const char* filename, const ByteArray& buffer);
void SaveToFile(const std::string& filename, const ByteArray& buffer);

} // namespace S3

#endif // S3LVLDEC_BINARYFILE_H_INCLUDED
