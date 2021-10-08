
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <unordered_map>
#include <vector>

struct cpio_odc_header
{
	char c_magic[6];
	char c_dev[6];
	char c_ino[6];
	char c_mode[6];
	char c_uid[6];
	char c_gid[6];
	char c_nlink[6];
	char c_rdev[6];
	char c_mtime[11];
	char c_namesize[6];
	char c_filesize[11];
};

static uint64_t convert_octal(const char* const str, const size_t size = 6)
{
	char termstr[size + 1];
	memcpy(termstr, str, size);
	termstr[size] = '\0';
	return strtol(termstr, nullptr, 8);
}

int main(int argc, const char * argv[])
{
	FILE* in = argc > 1 ? fopen(argv[1], "rb") : stdin;
	assert(in != nullptr);

	FILE* out = argc > 2 ? fopen(argv[2], "wb") : stdout;
	assert(out != nullptr);
	
	cpio_odc_header header;

	std::unordered_map<uint64_t, std::vector<uint8_t>> entries;
	std::string name;
	std::vector<uint8_t> content;
	size_t count;

	while (fread(&header, 1, sizeof header, in) == sizeof header)
	{
		const uint64_t namesize = convert_octal(header.c_namesize);
		assert(namesize >= 2);
		name.resize(namesize);

		count = fread(&name[0], 1, namesize, in);
		assert(count == namesize);
		
		const uint64_t filesize = convert_octal(header.c_filesize, 11);
		if (filesize > 0)
		{
			content.resize(filesize);

			count = fread(&content[0], 1, filesize, in);
			//assert(count == filesize);
			if (count != filesize)
				return 1;
			
			const uint64_t nlink = convert_octal(header.c_nlink);
			if (nlink > 1)
			{
				const uint64_t dev = convert_octal(header.c_dev);
				const uint64_t ino = convert_octal(header.c_ino);
				const uint64_t key = dev << 32 | ino;
				const auto iter = entries.find(key);

				if (iter == entries.end())
				{
					entries[key] = content;
				}
				else
				{
					assert(content.size() == iter->second.size());
					content = iter->second;
				}
			}
		}

		count = fwrite(&header, 1, sizeof header, out);
		assert(count == sizeof header);
		
		count = fwrite(&name[0], 1, name.size(), out);
		assert(count == name.size());

		if (filesize > 0)
		{
			count = fwrite(&content[0], 1, filesize, out);
			assert(count == filesize);
		}
		
		// TODO: write header
		// TODO: write name
		// TODO: write content if not empty
		
	}
	
	return 0;
}
