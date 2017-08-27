#!/usr/bin/env python

#
#    Yet another build environment for macOS
#    Copyright (C) 2017 Alexey Lysiuk
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

import os
import subprocess
import sys

try:
    # Python 2
    from urllib2 import urlopen
except ImportError:
    # Python 3
    from urllib.request import urlopen

from config import TARGETS


def mount_usr_local():
    pass


def download(url):
    subprocess.check_call(['curl', '-LO', url])


def extract(filename):
    subprocess.check_call(['tar', '-xf', filename])


def build(name):
    target = TARGETS[name]
    url = target['url']
    splitted = url.rsplit('/', 1)
    filename = splitted[1]

    if not os.path.exists(filename):
        download(url)
        extract(filename)

    chdir = 'nochdir' not in target or not target['nochdir']

    if chdir:
        # TODO: detect dirname
        # TODO: extract only when dirname doesn't exist
        dirname = filename
        dirname = dirname.replace('.tar.gz', '')
        dirname = dirname.replace('.tar.bz2', '')
        dirname = dirname.replace('.tar.xz', '')
        dirname = dirname.replace('.tgz', '')

        os.chdir(dirname)

    command = target['cmd']
    subprocess.check_call([command], shell=True)

    if chdir:
        os.chdir('..')


def main():
    if len(sys.argv) < 2:
        print('Usage: build.py [target ...]')
        sys.exit(1)

    to_build = []

    def add_deps(root):
        if root in to_build:
            return

        for dep in TARGETS[root]['dep']:
            add_deps(dep)

        to_build.append(root)

    for name in sys.argv[1:]:
        add_deps(name)

    self_path = os.path.dirname(os.path.abspath(__file__))
    os.chdir(self_path)

    mount_usr_local()

    for name in to_build:
        build(name)


if __name__ == '__main__':
    main()
