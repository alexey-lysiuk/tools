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

import pyparsing
from udmf_map import UDMFMap


_KEYWORD = pyparsing.CharsNotIn('{}();"\'\n\t')
_QUOTED_STRING = pyparsing.QuotedString('"')
_INTEGER = pyparsing.Regex(r'[+-]?[1-9]+[0-9]* | 0[0-9]+ | 0x[0-9A-Fa-f]+')
_FLOAT = pyparsing.Regex(r'[+-]?[0-9]+\'.\'[0-9]*([eE][+-]?[0-9]+)?')
_IDENTIFIER = pyparsing.Regex(r'[A-Za-z_]+[A-Za-z0-9_]*')

_VALUE = _INTEGER | _FLOAT | _QUOTED_STRING | _KEYWORD
_ASSIGNMENT_EXPRESSION = pyparsing.Group(_IDENTIFIER + pyparsing.Suppress('=') + _VALUE + pyparsing.Suppress(';'))
_EXPRESSION_LIST = pyparsing.OneOrMore(_ASSIGNMENT_EXPRESSION)
_BLOCK = pyparsing.Group(_IDENTIFIER + pyparsing.Suppress('{') + _EXPRESSION_LIST + pyparsing.Suppress('}'))
_GLOBAL_EXPRESSION = _BLOCK | _ASSIGNMENT_EXPRESSION
_GLOBAL_EXPRESSION_LIST = pyparsing.OneOrMore(_GLOBAL_EXPRESSION)
_TRANSLATION_UNIT = _GLOBAL_EXPRESSION_LIST
_TRANSLATION_UNIT.ignore(pyparsing.cppStyleComment)


def load(data):
    parse_result = _TRANSLATION_UNIT.parseString(data, parseAll=True)
    udmf_map = UDMFMap()

    for expression in parse_result.asList():
        assert isinstance(expression, list)
        assert len(expression) >= 2

        keyword = expression[0]
        value = expression[1:] if isinstance(expression[1], list) else expression[1]

        udmf_map.append(keyword, value)

    return udmf_map


if __name__ == '__main__':
    import sys

    if 1 == len(sys.argv):
        print('Usage: %s textmap.txt ...' % __file__)
        sys.exit(0)

    import profiling
    profiler = profiling.Profiler(False)

    for filename in sys.argv[1:]:
        with open(filename) as f:
            print(load(f.read()).astext())

    profiler.close()
