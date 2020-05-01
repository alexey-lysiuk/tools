#!/usr/bin/env python3

import re

for line in open('CMakeLists.txt').readlines():
    match = re.search(r'project\s*\(\s*(\w+)\s*\)', line, re.IGNORECASE)
    if match:
        print(match.group(1))
        exit(0)

exit(1)
