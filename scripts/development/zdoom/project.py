#!/usr/bin/env python

import re

for line in open('CMakeLists.txt').readlines():
    match = re.search('project\s*\(\s*(\w+)\s*\)', line, re.IGNORECASE)
    if match:
        print match.group(1)
        exit(0)

exit(1)
