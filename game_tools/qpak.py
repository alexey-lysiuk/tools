#!/usr/bin/env python3
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

import struct
import sys


def mkpak(pakfile: str, inputfiles: list[str]):
    fnmaxlen = 56  # TODO: should filename be null-terminated?

    for name in inputfiles:
        length = len(name)

        if length > fnmaxlen:
            raise RuntimeError(f'File name {name} is too long, {length} > {fnmaxlen}')

    outfile = open(pakfile, 'wb')
    offset = outfile.seek(12)  # skip header

    directory = []

    for name in inputfiles:
        infile = open(name, 'rb')
        indata = infile.read()

        # TODO: relative path
        size = len(indata)
        directory.append((name, offset, size))

        padding = (4 - (size & 3)) & 3
        offset += size + padding

        outfile.write(indata)
        outfile.write(b'\0' * padding)

    for entry in directory:
        name = bytearray(entry[0], 'ascii')
        namelength = len(name)

        if namelength < fnmaxlen:
            name.extend([0 for _ in range(fnmaxlen - namelength)])

        binentry = struct.pack('<56sii', name, entry[1], entry[2])
        outfile.write(binentry)

    outfile.seek(0)
    outfile.write(b'PACK')

    header = struct.pack('<ii', offset, len(directory) * 64)
    outfile.write(header)


def _main() -> None:
    argv = sys.argv

    if len(argv) < 3:
        print(f'Usage: {argv[0]} pak-file input-file ...')
    else:
        mkpak(argv[1], argv[2:])


if __name__ == '__main__':
    _main()
