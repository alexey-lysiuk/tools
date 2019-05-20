# Useless BEHAVIOR lump removal tool
# Based on doomwad.py: Doom WAD file library
# Requires Python 2.7, it doesn't work with 3.x
#
# Copyright (c) 2009 Jared Stafford (jspenguin@gmail.com)
# Copyright (C) 2015-2018 Alexey Lysiuk
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#   derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import io
import re
import struct


_header = struct.Struct('<4sII')
_dirent = struct.Struct('<II8s')

# Map members
specnames = {
    'THINGS', 'VERTEXES', 'LINEDEFS', 'SIDEDEFS', 'SEGS', 'SSECTORS',
    'NODES', 'SECTORS', 'REJECT', 'BLOCKMAP', 'BEHAVIOR', 'SCRIPTS'
}


# TODO: add other start/end marker names
_marker_lumpname_regex = re.compile(r'^(E\dM\d|MAP\d\d|S?S_START|S?S_END)$')


class Lump(object):
    def __init__(self, name, data, index=None):
        self.name = name
        self.data = data
        self.index = index
        self.namespace = ''
        self.marker = 0 == len(data) and self.name not in specnames \
            or _marker_lumpname_regex.match(self.name)

    def __repr__(self):
        return "Lump('{0}', {1})".format(self.name, len(self.data))


class WadFile(object):
    def __init__(self, data_or_file=None):
        if not data_or_file:
            self.sig = b'PWAD'
            self.lumps = []
            self.filename = ''
            return

        if hasattr(data_or_file, 'read') and \
                hasattr(data_or_file, 'seek'):
            file = data_or_file
        else:
            file = io.BytesIO(data_or_file)

        self.filename = hasattr(file, 'name') and file.name or ''

        sig, numentries, offset = _header.unpack(file.read(12))

        if b'IWAD' != sig and b'PWAD' != sig:
            raise ValueError('not a WAD file')

        self.sig = sig

        file.seek(offset, 0)
        direct = file.read(16 * numentries)

        lumps = []

        for i in range(numentries):
            pos = i * 16
            offset, size, name = _dirent.unpack(direct[pos: pos + 16])
            idx = name.find(b'\0')
            if idx != -1:
                name = name[:idx]

            if size:
                file.seek(offset, 0)
                data = file.read(size)
            else:
                data = ""

            lumps.append(Lump(name.upper(), data, i))

        self.lumps = lumps
        self._assignnamespaces()

    def __repr__(self):
        return "WadFile('{}', {})".format(self.filename, self.lumps)

    def _assignnamespaces(self):
        namespace = ''
        ismap = False

        for lump in self:
            ismapcur = lump.name in specnames

            if lump.marker:
                if lump.name.endswith('_END'):
                    # end marker found, reset namespace
                    namespace = ''
                elif '' == namespace or lump.name.endswith('_START'):
                    # some other marker found,
                    # use it as namespace start if it's top-level
                    # or if it's explicitly named so
                    namespace = lump.name
                ismapcur = False
            else:
                if not ismapcur and ismap:
                    # end of map found, reset namespace
                    namespace = ''
                    ismapcur = False

                lump.namespace = namespace

            ismap = ismapcur

    def writeto(self, file):
        directory = []
        pos = 12

        for lump in self:
            lsize = len(lump.data)
            directory.append((lump.name, pos, lsize))
            pos += lsize

        file.write(_header.pack(self.sig, len(self), pos))
        for lump in self:
            file.write(lump.data)

        for name, pos, size in directory:
            file.write(_dirent.pack(pos, size, name))

    # Simple linear search works fine: there are usually
    # only a few hundred lumps in a file.
    def find(self, name, marker=None):
        idx = 0
        if marker:
            idx = marker.index + 1

        name = name.upper()
        end = len(self.lumps)
        while idx < end:
            lump = self.lumps[idx]
            if lump.name == name:
                return lump

            # Is this another marker lump?
            if marker and lump.marker:
                return None
            idx += 1
        return None

    def findmarker(self, lump):
        idx = lump.index
        while idx >= 0:
            lump = self.lumps[idx]
            if lump.marker:
                return lump

            idx -= 1
        return None

    def _reindex(self, start=0):
        for i in range(start, len(self.lumps)):
            self.lumps[i].index = i

    def __getitem__(self, name):
        if isinstance(name, int):
            return self.lumps[name]

        names = name.split('/')
        lump = None
        for n in names:
            lump = self.find(n, lump)
        return lump

    def __len__(self):
        return len(self.lumps)

    def __iter__(self):
        return iter(self.lumps)

    def filter(self, function):
        """ Keep only those lumps which function returns true """
        self.lumps = list(filter(function, self))
        self._reindex()


if __name__ == '__main__':
    import sys
    import os

    if 1 == len(sys.argv):
        print('Usage: {0} directory'.format(__file__))
        exit(1)

    os.chdir(sys.argv[1])

    files = [f for f in os.listdir('.') if os.path.isfile(f) and f.lower().endswith('wad')]

    for filename in files:
        wad_file = open(filename, 'rb')
        wad_data = wad_file.read()
        wad_file.close()

        wad = WadFile(wad_data)
        count = len(wad.lumps)

        wad.filter(lambda lump: lump.name != 'BEHAVIOR' or len(lump.data) >= 32)

        if count != len(wad.lumps):
            os.rename(filename, filename + '.bak')
            with open(filename, 'wb') as f:
                wad.writeto(f)
            print('Updated ' + filename)
