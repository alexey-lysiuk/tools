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

from antlr4 import *
from UDMFLexer import UDMFLexer
from UDMFParser import UDMFParser
from UDMFListener import UDMFListener


class TestPrinter(UDMFListener):
    def exitAssignmentExpression(self, ctx):
        print ctx.Identifier().getText() + ' = ' + ctx.value().getText() + ';'

    def enterBlockExpression(self, ctx):
        print ctx.Identifier().getText() + '\n{'

    def exitBlockExpression(self, ctx):
        print '}\n'


def _print_udmf(filename):
    input = FileStream(filename)
    lexer = UDMFLexer(input)

    stream = CommonTokenStream(lexer)
    parser = UDMFParser(stream)
    tree = parser.udmf()

    printer = TestPrinter()
    walker = ParseTreeWalker()
    walker.walk(printer, tree)


if __name__ == '__main__':
    if 1 == len(sys.argv):
        print('Usage: PrintTest.py textmap.txt ...')
        sys.exit(0)

    for filename in sys.argv[1:]:
        _print_udmf(filename)
