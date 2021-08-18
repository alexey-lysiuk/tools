#!/usr/bin/env python3

#
#    Module than handles HROT .pak files
#    Copyright (C) 2021 Alexey Lysiuk
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import argparse
import ctypes
import os
import struct
import sys


class HrotPakEntry:
    def __init__(self, name: str = None, offset: int = -1, size: int = -1):
        self.name = name
        self.offset = offset
        self.size = size


class HrotPakFile:
    HEADER_STRUCT = struct.Struct('<4sII')
    # HEADER_SIZE = 8
    ENTRY_STRUCT = struct.Struct('<120sII')

    def __init__(self):
        self.entries = []

    def load(self, path: str):
        # self.path = path
        self.entries.clear()

        with open(path, 'rb') as f:
            buf = f.read(self.HEADER_STRUCT.size)
            header = self.HEADER_STRUCT.unpack(buf)
            # print(header)

            signature = header[0]
            toc_pos = header[1]
            toc_size = header[2]

            if signature != b'HROT':
                raise RuntimeError('Not a HROT .pak file')
            elif toc_size % self.ENTRY_STRUCT.size != 0:
                raise RuntimeError('Invalid TOC size')
            elif f.seek(toc_pos, os.SEEK_SET) != toc_pos:
                raise RuntimeError('Invalid TOC position')

            for _ in range(toc_size // self.ENTRY_STRUCT.size):
                buf = f.read(self.ENTRY_STRUCT.size)
                raw = self.ENTRY_STRUCT.unpack(buf)
                name = ctypes.create_string_buffer(raw[0]).value
                entry = HrotPakEntry(name.decode('ascii'), raw[1], raw[2])
                self.entries.append(entry)


def _parse_args():
    parser = argparse.ArgumentParser(description='*ZDoom binary dependencies for macOS')
    parser.add_argument('file', type=str, help='.pak file')

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--list', action='store_true', help='list .pak file table of content')
    group.add_argument('--extract', action='store_true', help='extract content of .pak file')

    return parser.parse_args()


def _main():
    min_version = (3, 8, 0, 'final', 0)

    if sys.version_info < min_version:
        print(f'This module requires Python {min_version[0]}.{min_version[1]}.{min_version[2]} or newer')
        exit(1)

    arguments = _parse_args()

    pak = HrotPakFile()
    pak.load(arguments.file)

    if arguments.list:
        for entry in pak.entries:
            print(f'{entry.name}  {entry.size}')


if __name__ == '__main__':
    _main()
