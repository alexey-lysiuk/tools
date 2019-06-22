/*
    Incomplete clickstream installer extractor
    Copyright (C) 2016 Martin Koegler <martin.koegler@chello.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <zlib.h>
#include <bzlib.h>
#include <ctype.h>
#include <libgen.h>
//#include <sys/stat.h>

#ifdef _MSC_VER
#include <direct.h>
#else
#include <sys/stat.h>
inline int _mkdir(const char* const name) { return mkdir(name, 0755); }
#endif

#include <string>
#include <vector>

void
die (const char *format, ...)
{
  va_list a;
  va_start (a, format);
  printf ("Fehler: \n");
  vprintf (format, a);
  printf ("\n");
  va_end (a);
  exit (1);
}

size_t
decompress_Block (char *dst, size_t dst_len, char *&src, size_t & src_len)
{
  if (!src_len)
    die ("decompress len=0");
  char type = *src;
  src++;
  src_len--;
  if (type == 0x00)
    {
      src++;
      unsigned l = src_len;
      if (l > dst_len)
	l = dst_len;
      memcpy (dst, src, l);
      src_len -= l;
      src += l;
      return l;
    }
  else if (type == 1)
    {
      z_stream strm;
      strm.zalloc = Z_NULL;
      strm.zfree = Z_NULL;
      strm.opaque = Z_NULL;
      strm.avail_in = 0;
      strm.next_in = Z_NULL;
      if (inflateInit (&strm) != Z_OK)
	die ("zlib init error");
      strm.avail_in = src_len;
      strm.next_in = (Bytef *) src;
      strm.avail_out = dst_len;
      strm.next_out = (Bytef *) dst;
      int r = inflate (&strm, Z_NO_FLUSH);
      if (r != Z_STREAM_END && r != Z_OK)
	die ("Uncompress zlib error");
      inflateEnd (&strm);
      src = (char *) strm.next_in;
      src_len = strm.avail_in;
      return strm.next_out - (Bytef *) dst;
    }
  else if (type == 2)
    {
      bz_stream strm;
      memset (&strm, 0, sizeof (strm));
      strm.next_in = src;
      strm.avail_in = src_len;
      strm.next_out = dst;
      strm.avail_out = dst_len;
      if (BZ2_bzDecompressInit (&strm, 0, 0) != BZ_OK)
	die ("bzinit failed");
      int r = BZ2_bzDecompress (&strm);
      if (r != BZ_STREAM_END)
	die ("bz2decomp failed");
      BZ2_bzDecompressEnd (&strm);
      src = strm.next_in;
      src_len = strm.avail_in;
      return strm.next_out - dst;
    }
  else
  {
    die ("Unknown type %02x", type);
	  return 0;
  }
}

//void
//mkpath (char *name)
void mkpath (char *filename)
{
//  if (!name || !strcmp (name, "/") || !strcmp (name, "."))
//    return;
//  char *dir = strdup (name);
//	dir = dirname (dir);
//  mkpath (dir);
//  free (dir);
//  mkdir (name, 0777);

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
		dirPath += namePart + '/';
		_mkdir(dirPath.c_str());
	}
}

void
write_file (const char *name, void *data, unsigned len)
{
  FILE *out = fopen (name, "w");
  if (!out)
    die ("Failed to write %s", name);
  fwrite (data, 1, len, out);
  fclose (out);
}

int
main (int ac, const char *ag[])
{
  if (ac != 2)
    die ("Usage %s file", ag[0]);
  FILE *in = fopen (ag[1], "r");
  if (!in)
    die ("Can't open %s", ag[1]);
  fseek (in, 0, SEEK_END);
  size_t len = ftell (in);
  fseek (in, 0, SEEK_SET);
  char *buf = (char *) malloc (len);
  if (!buf)
    die ("malloc failed");
  if (fread (buf, 1, len, in) != len)
    die ("Read failed");
  fclose (in);
  const char sig[] = { 0x77, 0x77, 0x67, 0x54, 0x29, 0x48 };

  unsigned pos = 0;
  while (pos < len - sizeof (sig))
    {
      if (!memcmp (sig, buf + pos, sizeof (sig)))
	break;
      pos++;
    }
  if (memcmp (sig, buf + pos, sizeof (sig)))
    die ("Signature not found");
  pos += sizeof (sig);

  char *filedata = NULL;
  size_t filedatalen = 0;
  char *index = NULL;
  size_t indexlen = 0;

  while (pos + 12 < len)
    {
      unsigned id = *(uint16_t *) (buf + pos);
      unsigned flag = *(uint16_t *) (buf + pos + 2);
      size_t blen = *(uint32_t *) (buf + pos + 4);
      size_t ulen = *(uint32_t *) (buf + pos + 8);

      if (id == 0x143a)
	{
	  indexlen = ulen;
	  index = (char *) malloc (indexlen);
	  char *src = buf + pos + 12;
	  size_t src_len = blen;
	  size_t l = decompress_Block (index, indexlen, src, src_len);
	  if (l != ulen)
	    die ("Uncompress error", l, ulen);
	}
      else if (id == 0x7f7f)
	{
	  filedata = buf + pos + 12;
	  filedatalen = blen;
	}

      pos += blen + 8;
    }
  if (pos + 4 != len)
    die ("Len error %d %d", pos, len);
  if (!filedata)
    die ("Data block not found");
  if (!index)
    die ("index not found");

  if (indexlen < 4)
    die ("Indexlen %d", len);
  unsigned cnt = *(uint32_t *) index;
  index += 4;
  indexlen -= 4;
  for (unsigned i = 0; i < cnt; i++)
    {
      unsigned size = *(uint16_t *) index;
      if (indexlen < size || size < 54)
	die ("Size %d %d", indexlen, size);

      unsigned flag = *(uint32_t *) (index + 2);
      size_t offset = *(uint32_t *) (index + 6);
      size_t clen = *(uint32_t *) (index + 10);
      size_t ulen = *(uint32_t *) (index + 18);
      unsigned flag2 = *(uint32_t *) (index + 14);
      unsigned flag5 = *(uint32_t *) (index + 30);
      unsigned flag6 = *(uint32_t *) (index + 34);
      if (flag2)
	die ("Unknown value flag2 %x", flag2);
      if (flag5 != 0 && flag5 != 0x800)
	die ("Unknown value flag5 %x", flag5);
      if (flag6 != 0)
	die ("Unknown value flag6 %x", flag6);
      if (flag != 0 && flag != 2)
	die ("Unknown value flag %x", flag);
      //char *name = strdup (index + (flag ? 38 : 62));
		char *name = strdup (index + (flag ? 44 : 68));
      for (char *a = name; *a; a++)
	if (*a == '\\')
	  *a = '/';

      char *src = filedata + offset;
		if (clen > 0)
		{
			size_t src_len = clen;
			if (clen > filedatalen)
				die ("Compress position");
			char *dst = (char *) malloc (ulen);
			if (!dst)
				die ("Malloc error");
			size_t l = decompress_Block (dst, ulen, src, src_len);
			if (l != ulen)
				die ("Uncompress data error");
			printf ("Name %s\n", name);
			char *dir = strdup (name);
			//mkpath (dirname (dir));
			mkpath(dir);
			write_file (name, dst, ulen);
			free (dst);
			free (dir);
		}

      free (name);

      indexlen -= size;
      index += size;
    }
  if (indexlen)
    die ("RemainIndex %d", indexlen);
  return 0;
}
