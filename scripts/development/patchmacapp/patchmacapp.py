#!/usr/bin/env python3

import os
import plistlib
import re
import subprocess
import sys
import typing


class MacAppPatcher(object):
    def __init__(self, path: typing.Optional[str] = None):
        self.target = None
        self.architectures = []
        self.disassembly = {}

        if path:
            self.open(path)

    def open(self, path: str):
        assert path

        if os.path.isdir(path):
            contents_path = path + '/Contents/'

            with open(contents_path + 'Info.plist', 'rb') as plist_file:
                plist = plistlib.load(plist_file)

            self.target = contents_path + 'MacOS/' + plist['CFBundleExecutable']
        else:
            self.target = path

        args = ('file', self.target)
        file_output = subprocess.run(args, check=True, capture_output=True, text=True).stdout
        self.architectures = re.findall(r'\sMach-O 64-bit executable (\w+)', file_output)
        assert len(self.architectures) >= 1

    def disassemble(self):
        assert self.target

        for architecture in self.architectures:
            args = ('objdump', '--disassemble', f'--arch={architecture}', self.target)
            output = subprocess.run(args, check=True, capture_output=True).stdout.decode('latin-1')
            self.disassembly[architecture] = output.split(os.linesep)

    def sign(self, identity: str = '-'):
        assert self.target
        args = ('codesign', '--sign', identity, '--force', self.target)
        subprocess.run(args, check=True)

    def unsign(self):
        assert self.target
        args = ('codesign', '--remove', self.target)
        subprocess.run(args, check=True)

    def __str__(self):
        architectures = ', '.join(self.architectures)
        disassembly = ', '.join(self.disassembly.keys()) if len(self.disassembly) > 0 else '<none>'

        return f'Target: {self.target}\n' \
               f'Architectures: {architectures}\n' \
               f'Disassembly: {disassembly}'


def _main():
    argv = sys.argv

    if len(argv) < 2:
        print(f'Usage: {argv[0]} target')
        exit(1)

    patcher = MacAppPatcher(argv[1])
    patcher.disassemble()
    print(patcher)


if __name__ == '__main__':
    _main()
