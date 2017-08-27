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

_COMMON_FLAGS = ' -mmacosx-version-min=10.7 -isysroot /Volumes/Storage/Work/devbuilds/macos_sdk/MacOSX10.7.sdk'
_COMMON_COMPILE_FLAGS = '-I/usr/local/include' + _COMMON_FLAGS

ENVIRON = {
    'CFLAGS': _COMMON_COMPILE_FLAGS,
    'CXXFLAGS': _COMMON_COMPILE_FLAGS,
    'OBJCFLAGS': _COMMON_COMPILE_FLAGS,
    'CPPFLAGS': _COMMON_COMPILE_FLAGS,
    'OBJCXXFLAGS': _COMMON_COMPILE_FLAGS,
    'LDFLAGS': '-L/usr/local/lib' + _COMMON_FLAGS
}

# TODO: file hash

TARGETS = {
    'autoconf': {
        'url': 'https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.xz',
        'dep': (),
        'cmd': (
            ('./configure', ),
            ('make', 'install')
        )
    },

    'automake': {
        'url': 'https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.xz',
        'dep': (),
        'cmd': (
            ('./configure',),
            ('make', 'install')
        )
    },

    'ffi': {
        'url': 'https://sourceware.org/pub/libffi/libffi-3.2.1.tar.gz',
        'dep': (),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'flac': {
        'url': 'https://downloads.xiph.org/releases/flac/flac-1.3.2.tar.xz',
        'dep': ('ogg',),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'fluidsynth': {
        'url': 'https://github.com/FluidSynth/fluidsynth/archive/v1.1.6.tar.gz',
        'dep': ('pkg-config', 'autoconf', 'automake', 'libtool', 'glib', 'sndfile'),
        'cmd': (
            ('./autogen.sh', ),
            ('./configure', '--enable-static', '--disable-shared', '--without-readline'),
            ('make', 'install')
        ),
        'env': {
            'LDFLAGS': '-framework AudioToolbox -framework AudioUnit '
                       '-framework CoreAudio -framework CoreMIDI -framework CoreServices'
        }
    },

    'gettext': {
        'url': 'https://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.8.1.tar.xz',
        'dep': (),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'glib': {
        # The last version that supports Mac OS X 10.7 Lion
        'url': 'https://download.gnome.org/sources/glib/2.44/glib-2.44.1.tar.xz',
        'dep': ('ffi', 'gettext', 'pcre'),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'libtool': {
        'url': 'http://ftp-gnu-org.ip-connect.vn.ua/libtool/libtool-2.4.6.tar.xz',
        'dep': (),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'mpg123': {
        'url': 'https://www.mpg123.de/download/mpg123-1.25.6.tar.bz2',
        'dep': (),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'ogg': {
        'url': 'https://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.xz',
        'dep': (),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'pcre': {
        'url': 'https://ftp.pcre.org/pub/pcre/pcre-8.41.tar.bz2',
        'dep': (),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared', '--enable-unicode-properties'),
            ('make', 'install')
        )
    },

    'pkg-config': {
        'url': 'https://pkg-config.freedesktop.org/releases/pkg-config-0.29.2.tar.gz',
        'dep': (),
        'cmd': (
            ('./configure', '--with-internal-glib'),
            ('make', 'install')
        )
    },

    'sndfile': {
        'url': 'http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28.tar.gz',
        'dep': ('pkg-config', 'ogg', 'vorbis', 'flac'),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'vorbis': {
        'url': 'https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.xz',
        'dep': ('ogg',),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    }
}
