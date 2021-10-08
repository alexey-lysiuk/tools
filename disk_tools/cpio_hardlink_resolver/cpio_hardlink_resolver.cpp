
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

#define expect(condition) \
	if (!(condition)) { printf("ERROR: '%s' test failed at line %i\n", #condition, __LINE__); return __LINE__; }

int main(int argc, const char * argv[])
{
	FILE* in = argc > 1 ? fopen(argv[1], "rb") : stdin;
	expect(in != nullptr);

	FILE* out = argc > 2 ? fopen(argv[2], "wb") : stdout;
	expect(out != nullptr);
	
	cpio_odc_header header;

	std::unordered_map<uint64_t, std::vector<uint8_t>> entries;
	std::string name;
	std::vector<uint8_t> content;
	size_t count;

	while (true)
	{
		count = fread(&header, 1, sizeof header, in);
		expect(count == sizeof header);

		const uint64_t namesize = convert_octal(header.c_namesize);
		expect(namesize >= 2);
		name.resize(namesize);

		count = fread(&name[0], 1, namesize, in);
		expect(count == namesize);
		
		const uint64_t filesize = convert_octal(header.c_filesize, 11);
		if (filesize > 0)
		{
			content.resize(filesize);

			count = fread(&content[0], 1, filesize, in);
			expect(count == filesize);
			
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
					expect(content.size() == iter->second.size());
					content = iter->second;
				}
			}
		}

		count = fwrite(&header, 1, sizeof header, out);
		expect(count == sizeof header);
		
		count = fwrite(&name[0], 1, name.size(), out);
		expect(count == name.size());

		if (filesize > 0)
		{
			count = fwrite(&content[0], 1, filesize, out);
			expect(count == filesize);
		}

		if (memcmp(&name[0], "TRAILER!!!", 10) == 0)
		{
			expect(filesize == 0);

			content.resize(4096);

			while (true)
			{
				count = fread(&content[0], 1, content.size(), in);
				count = fwrite(&content[0], 1, count, out);
				
				if (count != content.size())
					break;
			}

			break;
		}
	}
	
	return 0;
}
