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

from collections import OrderedDict

from antlr4 import InputStream, CommonTokenStream, ParseTreeWalker
from UDMFLexer import UDMFLexer
from UDMFParser import UDMFParser
from UDMFListener import UDMFListener

from udmf_map import UDMFMap


class UDMFDataExtractor(UDMFListener):
    def __init__(self, udmf_map):
        self.map = udmf_map
        self.block_name = None
        self.block = None

    def exitAssignmentExpression(self, ctx):
        keyword = ctx.Identifier().getText()
        value = ctx.value().getText()

        if self.block_name:
            self.block[keyword] = value
        else:
            self.map.append(keyword, value)

    def enterBlockExpression(self, ctx):
        self.block_name = ctx.Identifier().getText()
        self.block = OrderedDict()

    def exitBlockExpression(self, ctx):
        self.map.append(self.block_name, self.block)
        self.block_name = None
        self.block = None


def load(data):
    input_stream = InputStream(data)
    lexer = UDMFLexer(input_stream)

    token_stream = CommonTokenStream(lexer)
    parser = UDMFParser(token_stream)
    tree = parser.udmf()

    udmf_map = UDMFMap()
    extractor = UDMFDataExtractor(udmf_map)
    ParseTreeWalker().walk(extractor, tree)

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
