#
#    Python Doom Tools
#    Copyright (C) 2015, 2016 Alexey Lysiuk
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


_STRING_CODEC = 'iso-8859-1'


def native_str(data):
    need_decode = sys.hexversion >= 0x3000000 \
        and isinstance(data, (bytes, bytearray))
    return data.decode(_STRING_CODEC) if need_decode else data


def binary_str(data):
    need_encode = sys.hexversion >= 0x3000000 \
        and isinstance(data, str)
    return data.encode(_STRING_CODEC) if need_encode else data
