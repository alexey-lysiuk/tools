#!/usr/bin/env python3

import os
import sys
import zipfile


def justify_epub(path: str):
    fin = zipfile.ZipFile(path, mode='r')

    filename, extension = os.path.splitext(path)
    justified_path = filename + '--justify' + extension
    fout = zipfile.ZipFile(justified_path, mode='w')

    for entry in fin.infolist():
        data = fin.read(entry)

        if entry.filename.endswith('.css'):
            justify = b'p { text-align:justify; }\n'

            if not data.startswith(justify):
                data = justify + data

        fout.writestr(entry, data)


def _main():
    for path in sys.argv[1:]:
        justify_epub(path)


if __name__ == '__main__':
    _main()
