/*
-----------------------------------------------------------------------------

BlockDiff is a tool to compare two binary files block by block and produce
diff (delta) file. Then diff file can be applied on the first file
to get the second one.

It's usefull with disk images or any files stored as set of block
with fixed size.
 
-----------------------------------------------------------------------------

Copyright 2013 Alexey Lysiuk

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

-----------------------------------------------------------------------------
*/


// TODO: custom block size
// TODO: stronger hash function, sha-1 or even sha-256
// TODO: faster map, need benchmarks
// TODO: compare hashes via SSE intrinsics
// TODO: add header with magic and version to .blockdiff (and hash type if needed)
// TODO: add block hashes to .blockdiff for verification
// TODO: add support for smaller last block
// TODO: compress .blockdiff


#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <string>
#include <vector>
#include <map>

extern "C"
{
#include "global.h"
#include "md5.h"
}


typedef uint32_t BlockIndex;
typedef uint8_t  OperationCode;


struct Checksum
{
	static const size_t LENGTH = 16;

	unsigned char raw[LENGTH];

	// TODO: constructor

	bool operator<(const Checksum& other) const
	{
		return memcmp(raw, other.raw, LENGTH) < 0;
	}
};


typedef std::map<Checksum, BlockIndex> ChecksumMap;



class File
{
public:
	explicit File(const char* const path, const char* const mode = "rb");
	~File();

	enum ErrorMode
	{
		ERROR_SKIP,
		ERROR_HALT
	};

	bool seek(const off_t position, const int origin, const ErrorMode errorMode = ERROR_HALT);

	size_t read (      void* const buffer, const size_t length, const ErrorMode errorMode = ERROR_HALT);
	size_t write(const void* const buffer, const size_t length, const ErrorMode errorMode = ERROR_HALT);

	bool isEoF() const;

	void close();

private:
	FILE* m_file;
	std::string m_path;

	void halt(const char* const operation);

	// Without implementation
	File(const File&);
	File& operator=(const File&);

};


File::File(const char* const path, const char* const mode)
{
	if (NULL == path)
	{
		return;
	}

	m_file = fopen(path, mode);

	if (NULL != m_file)
	{
		m_path = path;
	}
}

File::~File()
{
	close();
}

bool File::seek(const off_t position, const int origin, const ErrorMode errorMode)
{
	if (NULL == m_file)
	{
		return false;
	}

	const int result = fseeko(m_file, position, origin);

	if (0 != result && ERROR_HALT == errorMode)
	{
		halt("seek");
	}

	return 0 == result;
}

size_t File::read(void* const buffer, const size_t length, const ErrorMode errorMode)
{
	if (NULL == m_file || NULL == buffer || 0 == length)
	{
		return 0;
	}

	const size_t result = fread(buffer, 1, length, m_file);

	if (result != length && ERROR_HALT == errorMode)
	{
		halt("read");
	}

	return result;
}

size_t File::write(const void* const buffer, const size_t length, const ErrorMode errorMode)
{
	if (NULL == m_file || NULL == buffer || 0 == length)
	{
		return 0;
	}

	const size_t result = fwrite(buffer, 1, length, m_file);

	if (result != length && ERROR_HALT == errorMode)
	{
		halt("write");
	}

	return result;
}

bool File::isEoF() const
{
	return NULL == m_file ? true : feof(m_file);
}

void File::close()
{
	if (NULL != m_file)
	{
		fclose(m_file);
	}

	m_file = NULL;
	m_path.clear();
}

void File::halt(const char* const operation)
{
	char errorString[2048] = {0};

	snprintf(errorString, sizeof(errorString), "Error %i: Unable to %s file %s\nMessage", errno, operation, m_path.c_str());
	perror(errorString);

	exit(EXIT_FAILURE);
}


static const size_t BLOCK_SIZE = 2048;


static bool GetNextBlockChecksum(File& file, Checksum& checksum, unsigned char buffer[BLOCK_SIZE])
{
	if ( BLOCK_SIZE != file.read(buffer, BLOCK_SIZE, File::ERROR_SKIP) )
	{
		// TODO: support smaller last block
		return false;
	}

	MD5_CTX md5Context;

	MD5Init(&md5Context);
	MD5Update(&md5Context, buffer, static_cast<unsigned int>(BLOCK_SIZE));
	MD5Final(checksum.raw, &md5Context);

	return true;
}


enum ModeType
{
	MODE_UNDEFINED,
	MODE_HELP,
	MODE_ENCODING,
	MODE_DECODING
};


ModeType GetMode(const char* command)
{
	ModeType result = MODE_UNDEFINED;

	while ('-' == command[0])
	{
		++command;
	}

	if (0 == strcmp(command, "help"))
	{
		result = MODE_HELP;
	}
	else if (0 == strcmp(command, "encode"))
	{
		result = MODE_ENCODING;
	}
	else if (0 == strcasecmp(command, "decode"))
	{
		result = MODE_DECODING;
	}

	return result;
}


enum OperationType
{
	OPERATION_UNDEFINED,
	OPERATION_COPY,
	OPERATION_STORE,
};




void PrintUsageAndExit()
{
	puts("blockdiff\nUsage ...");

	// TODO ...

	exit(EXIT_SUCCESS);
}


const char* GetTargetFileMode(const ModeType mode)
{
	switch (mode)
	{
		case MODE_ENCODING:
			return "rb";

		case MODE_DECODING:
			return "wb";

		default:
			return "";
	}
}

const char* GetDiffFileMode(const ModeType mode)
{
	switch (mode)
	{
		case MODE_ENCODING:
			return "wb";

		case MODE_DECODING:
			return "rb";

		default:
			return "";
	}
}

FILE* OpenFile(const char* const path, const const char* const mode)
{
	FILE* result = fopen(path, mode);

	if (NULL == result)
	{
		char errorString[2048] = {0};

		snprintf(errorString, sizeof(errorString), "Error %i: Unable to open file %s\nMessage", errno, path);
		perror(errorString);
		
		exit(EXIT_FAILURE);
	}

	return result;
}

size_t ReadFile(void* buffer, const size_t size, FILE* file)
{
	const size_t result = fread(buffer, 1, size, file);

	if (result != size)
	{
		char errorString[2048] = {0};

		//snprintf(errorString, sizeof(errorString), "Error %i: Unable to read file %s\nMessage", errno, path);
		perror(errorString);

		exit(EXIT_FAILURE);
	}

	return result;
}


int DoEncoding(File& sourceFile, File& targetFile, File& diffFile)
{
	ChecksumMap originalBlocks;

	unsigned char buffer[BLOCK_SIZE] = {0};

	// Build blocks' checksums and indices map

	for (BlockIndex blockIndex = 0; /* NONE */; ++blockIndex)
	{
		Checksum checksum = {0};

		if (!GetNextBlockChecksum(sourceFile, checksum, buffer))
		{
			// TODO: support smaller last block
			break;
		}

		originalBlocks[checksum] = blockIndex;

//		if (originalBlocks.end() == originalBlocks.find(checksum))
//		{
//			originalBlocks.insert(std::make_pair(checksum, blockIndex));
//		}
	}

	// Scan target file with checksums/indices map
	// and produce .blockdiff file

	for (BlockIndex blockIndex = 0; /* NONE */; ++blockIndex)
	{
		Checksum checksum = {0};

		if (!GetNextBlockChecksum(targetFile, checksum, buffer))
		{
			// TODO: support smaller last block
			break;
		}

		const ChecksumMap::const_iterator it = originalBlocks.find(checksum);

		const OperationCode operation = static_cast<uint8_t>(
			originalBlocks.end() == it
				? OPERATION_STORE
				: OPERATION_COPY );

		diffFile.write(&operation, sizeof(operation));

		if (operation == OPERATION_COPY)
		{
			diffFile.write(&it->second, sizeof(it->second));
		}
		else if (operation == OPERATION_STORE)
		{
			diffFile.write(buffer, BLOCK_SIZE);
		}
	}

	return EXIT_SUCCESS;
}



struct Command
{
	OperationType operation;
	void* data;
};

typedef std::vector<Command> CommandList;


int DoDecoding(File& sourceFile, File& targetFile, File& diffFile)
{
	CommandList commands;

	// Read .blockdiff file and build command sequence

	while ( true )
	{
		OperationCode operationCode = OPERATION_UNDEFINED;
		diffFile.read( &operationCode, sizeof(operationCode), File::ERROR_SKIP );

		if ( diffFile.isEoF() )
		{
			break;
		}

		Command command;
		command.operation = static_cast<OperationType>(operationCode);

		switch (operationCode)
		{
			case OPERATION_COPY:
				command.data = new BlockIndex;
				diffFile.read( command.data, sizeof(BlockIndex) );
				break;

			case OPERATION_STORE:
				command.data = new unsigned char[BLOCK_SIZE];
				diffFile.read( command.data, BLOCK_SIZE );
				break;

			default:
				// TODO ...
				return EXIT_FAILURE;
		}

		commands.push_back(command);
	}

	if (commands.empty())
	{
		// TODO:
		return EXIT_FAILURE;
	}

	// Write target file using loaded commands

	unsigned char buffer[BLOCK_SIZE];

	for (const Command& command : commands)
	{
		switch (command.operation)
		{
			case OPERATION_COPY:
			{
				const BlockIndex blockIndex = *static_cast<BlockIndex*>(command.data);
				const off_t offset = static_cast<off_t>(blockIndex) * BLOCK_SIZE;

				sourceFile.seek(offset, SEEK_SET);
				sourceFile.read(buffer, BLOCK_SIZE);
				targetFile.write(buffer, BLOCK_SIZE);

				break;
			}

			case OPERATION_STORE:
				targetFile.write(command.data, BLOCK_SIZE);
				break;

			default:
				// TODO ...
				return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}


int main(int argc, const char * argv[])
{
	if (argc < 2)
	{
		PrintUsageAndExit();
	}

	const ModeType mode = GetMode( argv[1] );

	if (MODE_UNDEFINED == mode)
	{
		// TODO: error: unknown mode
		return EXIT_FAILURE;
	}
	else if (MODE_HELP == mode)
	{
		PrintUsageAndExit();
	}
	else if (argc < 5) // <command> <source-file> <target-file> <diff-file>
	{
		PrintUsageAndExit();
	}

	File sourceFile( argv[2] );
	File targetFile( argv[3], GetTargetFileMode(mode) );
	File   diffFile( argv[4], GetDiffFileMode(mode) );

	switch (mode)
	{
		case MODE_ENCODING:
			return DoEncoding(sourceFile, targetFile, diffFile);

		case MODE_DECODING:
			return DoDecoding(sourceFile, targetFile, diffFile);

		default:
			return EXIT_FAILURE;
	}

    return EXIT_FAILURE;
}
