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

import config


def _dict_value(dictionary, key, default):
    return key in dictionary and dictionary[key] or default


def _mount_usr_local():
    basename = 'usr.local'
    filename = basename + '.sparseimage'

    if not os.path.exists(filename):
        subprocess.check_call(['hdiutil', 'create', '-size', '2g', '-type', 'SPARSE',
                               '-fs', 'HFS+', '-volname', basename, filename])

    hdi_info = subprocess.check_output(['hdiutil', 'info',])

    if -1 == hdi_info.find(filename):
        subprocess.check_call(['sudo', '-k', 'hdiutil', 'attach', '-mountpoint', '/usr/local', filename])


def _download(url):
    # TODO: python based download with progress
    subprocess.check_call(['curl', '-LO', url])


def _extract(filename):
    subprocess.check_call(['tar', '-xf', filename])


_GUESS_FILENAMES = (
    'configure',
    'Makefile',
    'autogen.sh',
    'CMakeLists.txt'
)


def _guess_work_dir(filename):
    files = subprocess.check_output(['tar', '-tf', filename])
    result = ''
    shortest = sys.maxint

    for name in files.split('\n'):
        parts = name.split('/')
        parts_count = len(parts)

        if parts[-1] in _GUESS_FILENAMES:
            if parts_count < shortest:
                result = '/'.join(parts[:-1])
                shortest = parts_count

    return result


def _merge_environ(dst, src):
    for e in src:
        if e in dst:
            dst[e] += ' ' + src[e]
        else:
            dst[e] = src[e]


def _build(name):
    target = config.TARGETS[name]
    url = target['url']
    splitted = url.rsplit('/', 1)
    filename = splitted[1]

    if not os.path.exists(filename):
        _download(url)
        _extract(filename)

    work_dir = _guess_work_dir(filename)
    environ = os.environ.copy()
    _merge_environ(environ, config.ENVIRON)
    _merge_environ(environ, _dict_value(target, 'env', {}))

    for command in target['cmd']:
        subprocess.check_call(command, cwd=work_dir, env=environ)


def _main():
    if len(sys.argv) < 2:
        print('Usage: build.py [target ...]')
        sys.exit(1)

    to_build = []

    def add_deps(root):
        if root in to_build:
            return

        deps = _dict_value(config.TARGETS[root], 'dep', ())

        for dep in deps:
            add_deps(dep)

        to_build.append(root)

    for name in sys.argv[1:]:
        add_deps(name)

    self_path = os.path.dirname(os.path.abspath(__file__))
    os.chdir(self_path)

    _mount_usr_local()

    for name in to_build:
        _build(name)


if __name__ == '__main__':
    _main()
