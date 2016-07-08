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

import distutils.core
import os.path
import sys

# noinspection PyUnresolvedReferences
import py2exe


if 1 == len(sys.argv):
    sys.argv = [sys.argv[0], 'py2exe']

root_path = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + os.path.pardir + os.path.sep
sys.path.insert(0, root_path)

script_path = root_path + 'udmf_round_vertex.py'
distutils.core.setup(console=[script_path])
