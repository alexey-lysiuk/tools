#!/usr/bin/env python3

import sys

def dos2unix(path):
    content = open(path, 'rb').read().replace(b'\r\n', b'\n')
    open(path, 'wb').write(content)

def _main():
    for path in sys.argv[1:]:
        dos2unix(path)

if __name__ == '__main__':
    _main()
