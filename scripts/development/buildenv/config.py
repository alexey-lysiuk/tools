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

_COMMON_FLAGS = ' -mmacosx-version-min=10.7 -isysroot /Volumes/Storage/Work/devbuilds/macos_sdk/MacOSX10.7.sdk'
_COMMON_COMPILE_FLAGS = '-I/usr/local/include' + _COMMON_FLAGS

os.environ['CFLAGS'] = _COMMON_COMPILE_FLAGS
os.environ['CXXFLAGS'] = _COMMON_COMPILE_FLAGS
os.environ['OBJCFLAGS'] = _COMMON_COMPILE_FLAGS
os.environ['CPPFLAGS'] = _COMMON_COMPILE_FLAGS
os.environ['OBJCXXFLAGS'] = _COMMON_COMPILE_FLAGS
os.environ['LDFLAGS'] = '-L/usr/local/lib' + _COMMON_FLAGS

_CONFIGURE = './configure'
_CONFIGURE_STATIC = _CONFIGURE + ' --enable-static --disable-shared'
_AND_MAKE = ' && make install'
_INSTALL = _CONFIGURE + _AND_MAKE
_INSTALL_STATIC = _CONFIGURE_STATIC + _AND_MAKE

# TODO: file hash

TARGETS = {
    'autoconf': {
        'url': 'https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.xz',
        'dep': (),
        'cmd': _INSTALL
    },

    'automake': {
        'url': 'https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.xz',
        'dep': (),
        'cmd': _INSTALL
    },

    'ffi': {
        'url': 'https://sourceware.org/pub/libffi/libffi-3.2.1.tar.gz',
        'dep': (),
        'cmd': _INSTALL_STATIC
    },

    'flac': {
        'url': 'https://downloads.xiph.org/releases/flac/flac-1.3.2.tar.xz',
        'dep': ('ogg',),
        'cmd': _INSTALL_STATIC
    },

    'fluidsynth': {
        'url': 'https://github.com/FluidSynth/fluidsynth/archive/v1.1.6.tar.gz',
        'dep': ('pkg-config', 'autoconf', 'automake', 'libtool', 'glib', 'sndfile'),
        'cmd': 'cd fluidsynth-1.1.6/fluidsynth && ./autogen.sh && '
               'LDFLAGS="${LDFLAGS} -framework AudioToolbox -framework AudioUnit '
               '-framework CoreAudio -framework CoreMIDI -framework CoreServices" '
               + _CONFIGURE_STATIC + " --without-readline" + _AND_MAKE,
        'nochdir': True
    },

    'gettext': {
        'url': 'https://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.8.1.tar.xz',
        'dep': (),
        'cmd': _INSTALL_STATIC
    },

    'glib': {
        # The last version that supports Mac OS X 10.7 Lion
        'url': 'https://download.gnome.org/sources/glib/2.44/glib-2.44.1.tar.xz',
        'dep': ('ffi', 'gettext', 'pcre'),
        'cmd': _INSTALL_STATIC
    },

    'libtool': {
        'url': 'http://ftp-gnu-org.ip-connect.vn.ua/libtool/libtool-2.4.6.tar.xz',
        'dep': (),
        'cmd': _INSTALL
    },

    'mpg123': {
        'url': 'https://www.mpg123.de/download/mpg123-1.25.6.tar.bz2',
        'dep': (),
        'cmd': _INSTALL_STATIC
    },

    'ogg': {
        'url': 'https://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.xz',
        'dep': (),
        'cmd': _INSTALL_STATIC
    },

    'pcre': {
        'url': 'https://ftp.pcre.org/pub/pcre/pcre-8.41.tar.bz2',
        'dep': (),
        'cmd': _CONFIGURE_STATIC + ' --enable-unicode-properties' + _AND_MAKE
    },

    'pkg-config': {
        'url': 'https://pkg-config.freedesktop.org/releases/pkg-config-0.29.2.tar.gz',
        'dep': (),
        'cmd': _CONFIGURE + ' --with-internal-glib' + _AND_MAKE
    },

    'sndfile': {
        'url': 'http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28.tar.gz',
        'dep': ('pkg-config', 'ogg', 'vorbis', 'flac'),
        'cmd': _INSTALL_STATIC
    },

    'vorbis': {
        'url': 'https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.xz',
        'dep': ('ogg',),
        'cmd': _INSTALL_STATIC
    }
}
