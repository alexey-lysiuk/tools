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

import sys
import omg.mapedit

if 2 != len(sys.argv):
    print('Usage: ' + __file__ + ' strife1.wad')
    exit(1)

wad = omg.wad.WAD()
wad.from_file(sys.argv[1])

LINE_SPECIALS = (
    187,
    193,
    215,
    216,
    227,
    230,
)

for map_name in wad.maps:
    try:
        map_data = wad.maps[map_name]
        editor = omg.mapedit.MapEditor(map_data)

        print(map_name + ':\n' + '-' * 15)

        for i in range(len(editor.linedefs)):
            linedef = editor.linedefs[i]
            action = linedef.action

            if action in LINE_SPECIALS:
                row_offset = '-' if 215 == action else \
                    str(editor.sidedefs[linedef.front].off_y)
                print('%04i:  %i  %s' % (i, action, row_offset))

        print('')

    except:
        continue
