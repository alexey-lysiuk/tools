#!/usr/bin/env python

import sys
import tasm

if 2 != len(sys.argv):
    print('Usage: tasmrecover file.asm')
    sys.exit(0)

parser = tasm.parser()
context = parser.parse(sys.argv[1])
parser.link()
generator = tasm.cpp(context, '')
