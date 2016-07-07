# doomwad.py: Doom WAD file library
#
# Copyright (c) 2009 Jared Stafford (jspenguin@gmail.com)
# Copyright (c) 2015, 2016 Alexey Lysiuk
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

"""Read and write Doom WAD files"""

import hashlib
from io import BytesIO
import re
import string
import struct

import py3hacks


# ==============================================================================


_spriteanglechar = string.digits + string.ascii_uppercase[:7]

_header = struct.Struct('<4sII')
_dirent = struct.Struct('<II8s')

# Map members
specnames = {
    'THINGS', 'VERTEXES', 'LINEDEFS', 'SIDEDEFS', 'SEGS', 'SSECTORS',
    'NODES', 'SECTORS', 'REJECT', 'BLOCKMAP', 'BEHAVIOR', 'SCRIPTS'
}

spritemarker = 'S_START'


# ==============================================================================


def issrpitenamespace(namespace):
    return namespace == spritemarker or namespace[1:] == spritemarker


def issequentialsprite(name):
    if name.startswith('ARTI'):
        # The very special case for Heretic and Hexen
        # This is a non-sequential sprite
        return False
    else:
        size = len(name)

        # 6th and 8th characters represent sprite angles
        angle1 = 6 == size and name[5] in _spriteanglechar
        angle2 = 8 == size and name[5] in _spriteanglechar \
                           and name[7] in _spriteanglechar

        # Treat a sprite is a part of the sequence
        # if its name contains one or two valid angle characters
        return angle1 or angle2


# ==============================================================================


line_comment_regex = re.compile('//.*?$', re.MULTILINE)
block_comment_regex = re.compile('/\*.*?\*/', re.DOTALL)


def striplumpcomments(lump):
    """ Remove both block and line comments from text lump """
    for regex in (line_comment_regex, block_comment_regex):
        lump.data = regex.sub('', lump.data)


# ==============================================================================


class SoundMapping:
    LOGICAL_TO_LUMP = 0
    LUMP_TO_LOGICAL = 1
    LUMP_TO_CONTENT = 2


# ==============================================================================


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

    @property
    def name(self):
        return py3hacks.native_str(self._name)

    @name.setter
    def name(self, value):
        self._name = py3hacks.binary_str(value)

    @property
    def rawname(self):
        return self._name

    @property
    def data(self):
        return py3hacks.native_str(self._data)

    @data.setter
    def data(self, value):
        self._data = py3hacks.binary_str(value)
        self._hash = None

    @property
    def rawdata(self):
        return self._data

    def hash(self):
        if self._hash:
            return self._hash
        else:
            algo = hashlib.md5()
            algo.update(self._data)
            self._hash = algo.digest()
            return self._hash


# ==============================================================================


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
            file = BytesIO(data_or_file)

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
                if lump.name.endswith('_END') or 'ENDMAP' == lump.name:
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
            lsize = len(lump.rawdata)
            directory.append((lump.rawname, pos, lsize))
            pos += lsize

        file.write(_header.pack(self.sig, len(self), pos))
        for lump in self:
            file.write(lump.rawdata)

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

    def removelump(self, lump):
        idx = lump.index
        self.lumps.remove(lump)
        self._reindex(idx)

    def insert(self, lump, before=None):
        idx = (before.index if before else len(self.lumps))
        lump.index = idx
        self.lumps.insert(idx, lump)
        self._reindex(idx + 1)

    def append(self, lump):
        self.insert(lump)

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

    def namespaces(self):
        """ Return sorted list of namespace names.
            Namespace is named by its start marker.
            An empty string designates the global namespace """
        namespaces = set()

        for lump in self:
            if lump.namespace not in namespaces:
                namespaces.add(lump.namespace)

        return sorted(namespaces)

    def namespacelumps(self, namespace):
        """ Return list of lumps belong to a namespace with the given name
            lumps ordering is preserved and duplicates are included """
        lumps = []

        for lump in self:
            if lump.namespace == namespace:
                lumps.append(lump)

        return lumps

    def uniquenamespacelumps(self, namespace):
        """ Return list of lumps belong to a namespace with the given name
            lumps ordering is preserved and duplicates are removed """
        lumps = []
        names = set()

        for lump in self.namespacelumps(namespace):
            if lump.name not in names:
                lumps.append(lump)
                names.add(lump.name)

        return lumps

    def spritelumps(self):
        """ Return list of sprite lumps """
        lumps = []

        for lump in self:
            if issrpitenamespace(lump.namespace):
                lumps.append(lump)

        return lumps

    def spritenames(self):
        """ Return sorted list of sprite names """
        names = set()

        for lump in self.spritelumps():
            name = lump.name
            names.add(issequentialsprite(name) and name[:4] or name)

        return sorted(names)

    def spritemapping(self):
        """
        Return dictionary with sprite/frame mapping in format:
        { sprite: { frame: hash,
                    frame: hash,
                    ... },
          sprite: { ... },
          ... }
        """
        result = {}

        for lump in self.spritelumps():
            lumpname = lump.name
            lumphash = lump.hash()

            if issequentialsprite(lumpname):
                name  = lumpname[:4]
                frame = lumpname[4:]

                if name in result:
                    result[name][frame] = lumphash
                else:
                    result[name] = { frame: lumphash }
            else:
                result[lumpname] = { lumpname: lumphash }

        return result

    def removesprite(self, sprite):
        """ Remove sprite lumps by sprite name """
        def should_keep_lump(lump):
            return not issrpitenamespace(lump.namespace) \
                or not lump.name.startswith(sprite)

        self.filter(should_keep_lump)

    def soundmapping(self, mapping_type):
        """ Return dictionary with sound mapping in various mapping formats """
        sndinfo = self.find('SNDINFO')

        if not sndinfo:
            return {}

        striplumpcomments(sndinfo)

        sounds = sndinfo.data.split('\n')
        result = {}

        for line in sounds:
            line = line.strip()

            if 0 == len(line):
                continue

            if line.startswith('$'):
                if SoundMapping.LOGICAL_TO_LUMP == mapping_type:
                    line = line.lower()

                    if line.startswith(('$alias', '$random')):
                        try:
                            _, logical, value = line.split(None, 2)
                            result[logical] = value
                        except:
                            # ill-formed command, report error?
                            pass
                continue

            try:
                logical_name, lump_name = line.split()

                logical_name = logical_name.lower()
                lump_name = lump_name.upper()

                if SoundMapping.LOGICAL_TO_LUMP == mapping_type:
                    result[logical_name] = lump_name
                elif SoundMapping.LUMP_TO_LOGICAL == mapping_type:
                    result[lump_name] = logical_name
                elif SoundMapping.LUMP_TO_CONTENT == mapping_type:
                    lump = self.find(lump_name)
                    result[lump_name] = lump and lump.hash() or None
                else:
                    assert(not 'Unsupported sound mapping type')
            except:
                # ill-formed sound assignment, report error?
                pass

        return result

    def filter(self, function):
        """ Keep only those lumps which function returns true """
        self.lumps = list(filter(function, self))
        self._reindex()


if __name__ == '__main__':
    import sys

    if 1 == len(sys.argv):
        print('Usage: {0} file.wad ...'.format(__file__))
        exit(1)

    allsprites = {}

    for filename in sys.argv[1:]:
        wad_file = open(filename, 'rb')
        wad_data = wad_file.read()
        wad_file.close()

        wad = WadFile(wad_data)

        for sprite_name, frames in wad.spritemapping().items():
            if sprite_name in allsprites:
                template = allsprites[sprite_name] == frames         \
                    and '[.] Identical sprite {0} was found'         \
                    or '[!] Sprite collision for name {0} was found'
                print(template.format(sprite_name))
            else:
                allsprites[sprite_name] = frames
