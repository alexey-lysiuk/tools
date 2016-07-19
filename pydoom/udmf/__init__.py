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

PARSER_DEFAULT = 0
PARSER_ANTLR4 = 1
PARSER_PYPARSING = 2


def load(data, parser=PARSER_DEFAULT):
    if PARSER_PYPARSING == parser:
        import udmf_pyparsing
        return udmf_pyparsing.load(data)
    else:
        import udmf_antlr4
        return udmf_antlr4.load(data)
