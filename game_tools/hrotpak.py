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
    def __init__(self, filename: str = None, offset: int = -1, size: int = -1):
        self.filename = filename
        self.offset = offset
        self.size = size
        self.data = None


class HrotPakFile:
    HEADER_STRUCT = struct.Struct('<4sII')
    ENTRY_STRUCT = struct.Struct('<120sII')
    SIGNATURE = b'HROT'

    def __init__(self, path: str = None, with_data=True):
        self.entries = []

        if path:
            self.read(path, with_data)

    def read(self, path: str, with_data=True):
        self.entries.clear()

        with open(path, 'rb') as f:
            self._load_toc(f)

            if with_data:
                self._load_data(f)

    def write(self, path: str):
        with open(path, 'wb') as f:
            toc_size = len(self.entries) * self.ENTRY_STRUCT.size
            toc_pos = self.HEADER_STRUCT.size

            for entry in self.entries:
                toc_pos += entry.size

            header = self.HEADER_STRUCT.pack(self.SIGNATURE, toc_pos, toc_size)
            f.write(header)

            for entry in self.entries:
                f.write(entry.data)

            for entry in self.entries:
                raw = self.ENTRY_STRUCT.pack(entry.filename.encode('ascii'), entry.offset, entry.size)
                f.write(raw)

    def list(self, output_path: str = None):
        output = open(output_path, 'w') if output_path else sys.stdout

        print('      Size      Offset  Filename', file=output)
        print('-' * 80, file=output)

        for entry in self.entries:
            print(f'{entry.size:10}  {hex(entry.offset):>10}  {entry.filename}', file=output)

    def extract(self, output_path: str = None):
        if output_path:
            os.makedirs(output_path, exist_ok=True)
        else:
            output_path = os.getcwd()

        for entry in self.entries:
            abs_path = os.path.join(output_path, entry.filename)
            with open(abs_path, 'wb') as f:
                f.write(entry.data)

    def _load_toc(self, f):
        buf = f.read(self.HEADER_STRUCT.size)
        header = self.HEADER_STRUCT.unpack(buf)

        signature = header[0]
        toc_pos = header[1]
        toc_size = header[2]

        if signature != self.SIGNATURE:
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

    def _load_data(self, f):
        for entry in self.entries:
            f.seek(entry.offset)
            entry.data = f.read(entry.size)


def _parse_args():
    parser = argparse.ArgumentParser(description='*ZDoom binary dependencies for macOS')
    parser.add_argument('file', type=str, help='.pak file')

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--list', action='store_true', help='list .pak file table of content')
    group.add_argument('--extract', action='store_true', help='extract content of .pak file')

    group = parser.add_argument_group()
    group.add_argument('--output', metavar='path', help='path to write output')

    return parser.parse_args()


def _main():
    min_version = (3, 8, 0, 'final', 0)

    if sys.version_info < min_version:
        print(f'This module requires Python {min_version[0]}.{min_version[1]}.{min_version[2]} or newer')
        exit(1)

    arguments = _parse_args()

    pak = HrotPakFile(arguments.file, arguments.extract)

    if arguments.list:
        pak.list(arguments.output)
    elif arguments.extract:
        pak.extract(arguments.output)


if __name__ == '__main__':
    _main()
