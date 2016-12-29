/*
 * Unpack and prettify ZDoom saved games
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

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/memorystream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/error/en.h"
#include "unzip.h"

class ZipFile
{
public:
	explicit ZipFile(const char* const filename)
	: m_file(unzOpen(filename))
	{
	}

	~ZipFile()
	{
		if (nullptr != m_file)
		{
			unzClose(m_file);
		}
	}

	ZipFile(const ZipFile&) = delete;
	ZipFile& operator=(const ZipFile&) = delete;

	operator bool() const
	{
		return nullptr != m_file;
	}

	operator unzFile() const
	{
		return m_file;
	}

private:
	unzFile m_file;
};

class ZippedFile
{
public:
	explicit ZippedFile(unzFile zipFile)
	: m_file(zipFile)
	{
		assert(nullptr != m_file);

		const int status = unzOpenCurrentFile(zipFile);

		if (UNZ_OK != status)
		{
			printf("ERROR: unzOpenCurrentFile() failed with error %d\n", status);
			m_file = nullptr;
		}
	}

	~ZippedFile()
	{
		if (nullptr != m_file)
		{
			unzCloseCurrentFile(m_file);
		}
	}

	operator bool() const
	{
		return nullptr != m_file;
	}

private:
	unzFile m_file;
};

static bool ProcessJSON(const char* const zippedName, const std::vector<char>& inputBuffer, FILE* const outputFile)
{
	using namespace rapidjson;

	Reader reader;
	MemoryStream is(&inputBuffer[0], inputBuffer.size());

	char writeBuffer[65536];
	FileWriteStream os(outputFile, writeBuffer, sizeof(writeBuffer));
	PrettyWriter<FileWriteStream> writer(os);

	if (!reader.Parse<kParseValidateEncodingFlag>(is, writer))
	{
		printf("ERROR: Failed to process file %s at offset %d with error %s\n", zippedName,
			   static_cast<unsigned>(reader.GetErrorOffset()), GetParseError_En(reader.GetParseErrorCode()));
		return false;
	}

	return true;
}

static bool ProcessEntry(unzFile zipFile, const char* const zippedName, unz_file_info fileInfo, const char* const output)
{
	assert(zipFile);
	assert(nullptr != zippedName);
	assert(nullptr != output);

	const ZippedFile zippedFile(zipFile);

	if (!zippedFile)
	{
		return false;
	}

	const size_t bufferSize = fileInfo.uncompressed_size;
	std::vector<char> buffer;
	buffer.resize(bufferSize);

	if (bufferSize != unzReadCurrentFile(zipFile, &buffer[0], unsigned(bufferSize)))
	{
		printf("ERROR: unzReadCurrentFile() failed for file %s\n", zippedName);
		return false;
	}

	std::string outputPath = output;
	outputPath += zippedName;

	FILE* outputFile = fopen(outputPath.c_str(), "wb");
	if (nullptr == outputFile)
	{
		printf("ERROR: Failed to open file %s for writing\n", outputPath.c_str());
		return false;
	}

	const size_t nameLength = strlen(zippedName);
	bool result = true;

	if (nameLength > 5 && 0 == strcmp(zippedName + nameLength - 5, ".json"))
	{
		result = ProcessJSON(zippedName, buffer, outputFile);
	}
	else
	{
		result = bufferSize != fwrite(&buffer[0], bufferSize, 1, outputFile);

		if (!result)
		{
			printf("ERROR: Failed to write file %s\n", outputPath.c_str());
		}
	}

	if (0 != fclose(outputFile))
	{
		printf("ERROR: Failed to close file %s\n", outputPath.c_str());
		result = false;
	}

	return result;
}

static bool ProcessZDS(const char* const input, const char* const output)
{
	assert(nullptr != input);
	assert(nullptr != output);

	const ZipFile zipFile(input);
	if (!zipFile)
	{
		printf("ERROR: Cannot load file %s\n", input);
		return false;
	}

	unz_global_info zipInfo = {};

	int status = unzGetGlobalInfo(zipFile, &zipInfo);
	if (UNZ_OK != status)
	{
		printf("ERROR: unzGetGlobalInfo() failed with error %d for file %s\n", status, input);
		return false;
	}

	for (uLong i = 0; i < zipInfo.number_entry; ++i)
	{
		if (UNZ_OK != status)
		{
			printf("ERROR: unzGoToNextFile() failed with error %d for file %s\n", status, input);
			return false;
		}

		char zippedName[256] = {};
		unz_file_info fileInfo = {};

		status = unzGetCurrentFileInfo(zipFile, &fileInfo, zippedName, sizeof zippedName, nullptr, 0, nullptr, 0);
		if (UNZ_OK != status)
		{
			printf("ERROR: unzGetCurrentFileInfo() failed with error %d for file %s\n", status, input);
			return false;
		}

		if (!ProcessEntry(zipFile, zippedName, fileInfo, output))
		{
			return false;
		}

		status = unzGoToNextFile(zipFile);
	}

	return true;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("Usage: %s file.zds [output.dir]\n", argv[0]);
		return EXIT_SUCCESS;
	}

	const char* const input = argv[1];
	std::string output;

	if (argc > 2)
	{
		output = argv[2];
		output += '/';
	}

	return ProcessZDS(input, output.c_str())
		? EXIT_SUCCESS
		: EXIT_FAILURE;
}
