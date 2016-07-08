#
#    Python Doom Tools
#    Copyright (C) 2016 Alexey Lysiuk
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


def _process_wad(filename):
    import udmf
    import doomwad
    import os.path

    print('Processing file %s...' % filename)

    fin = open(filename, 'rb')
    wad = doomwad.WadFile(fin)

    for lump in wad:
        namespace = lump.namespace

        if 'TEXTMAP' != lump.name:
            continue

        print('Processing map %s...' % namespace)
        udmfmap = udmf.UDMFMap(lump.data)

        for vertex in udmfmap.vertices:
            vertex['x'] = round(vertex['x'])
            vertex['y'] = round(vertex['y'])

        lump.data = udmfmap.astext()

    split_filename = os.path.splitext(filename)
    output_filename = split_filename[0] + '.iv' + split_filename[1]
    fout = open(output_filename, 'wb')
    wad.writeto(fout)


if '__main__' == __name__:
    import sys

    if 1 == len(sys.argv):
        try:
            self_name = __file__
        except NameError:
            self_name = sys.argv[0]

        print('Usage: %s file.wad ...' % self_name)
        sys.exit(0)

    import profiling
    profiler = profiling.Profiler(False)

    for filename in sys.argv[1:]:
        _process_wad(filename)

    profiler.close()
