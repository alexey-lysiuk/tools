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


def _cmake():
    import subprocess

    try:
        subprocess.call('cmake')
        return 'cmake'
    except OSError:
        pass

    return '/Applications/CMake.app/Contents/bin/cmake'


# TODO: name aliases: 'libogg' -> 'ogg'

TARGETS = {
    'autoconf': {
        'url': 'https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.xz',
        'chk': '64ebcec9f8ac5b2487125a86a7760d2591ac9e1d3dbd59489633f9de62a57684',
        'cmd': (
            ('./configure', ),
            ('make', 'install')
        )
    },

    'automake': {
        'url': 'https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.xz',
        'chk': 'af6ba39142220687c500f79b4aa2f181d9b24e4f8d8ec497cea4ba26c64bedaf',
        'cmd': (
            ('./configure',),
            ('make', 'install')
        )
    },

    'ffi': {
        'url': 'https://sourceware.org/pub/libffi/libffi-3.2.1.tar.gz',
        'chk': 'd06ebb8e1d9a22d19e38d63fdb83954253f39bedc5d46232a05645685722ca37',
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'flac': {
        'url': 'https://downloads.xiph.org/releases/flac/flac-1.3.2.tar.xz',
        'chk': '91cfc3ed61dc40f47f050a109b08610667d73477af6ef36dcad31c31a4a8d53f',
        'dep': ('ogg',),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'fluidsynth': {
        'url': 'https://downloads.sourceforge.net/project/fluidsynth/fluidsynth-1.1.6/fluidsynth-1.1.6.tar.gz',
        'chk': '50853391d9ebeda9b4db787efb23f98b1e26b7296dd2bb5d0d96b5bccee2171c',
        'dep': ('pkg-config', 'glib', 'sndfile'),
        'cmd': (
            (
                _cmake(), '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_SHARED_LIBS=NO', '-DLIB_SUFFIX=',
                '-Denable-framework=NO', '-Denable-readline=NO', '.'
            ),
            ('make', 'install')
        )
        ,
        'env': {
            'LDFLAGS': '-framework AudioToolbox -framework AudioUnit -framework CoreAudio '
                       '-logg -lvorbis -lvorbisenc -lFLAC'
        }
    },

    'gettext': {
        'url': 'https://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.8.1.tar.xz',
        'chk': '105556dbc5c3fbbc2aa0edb46d22d055748b6f5c7cd7a8d99f8e7eb84e938be4',
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'glib': {
        # The last version that supports Mac OS X 10.7 Lion
        'url': 'https://download.gnome.org/sources/glib/2.44/glib-2.44.1.tar.xz',
        'chk': '8811deacaf8a503d0a9b701777ea079ca6a4277be10e3d730d2112735d5eca07',
        'dep': ('ffi', 'gettext', 'pcre'),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'libtool': {
        'url': 'http://ftp-gnu-org.ip-connect.vn.ua/libtool/libtool-2.4.6.tar.xz',
        'chk': '7c87a8c2c8c0fc9cd5019e402bed4292462d00a718a7cd5f11218153bf28b26f',
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'mpg123': {
        'url': 'https://www.mpg123.de/download/mpg123-1.25.6.tar.bz2',
        'chk': '0f0458c9b87799bc2c9bf9455279cc4d305e245db43b51a39ef27afe025c5a8e',
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'ogg': {
        'url': 'https://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.xz',
        'chk': '3f687ccdd5ac8b52d76328fbbfebc70c459a40ea891dbf3dccb74a210826e79b',
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'openal': {
        'url': 'http://kcat.strangesoft.net/openal-releases/openal-soft-1.18.1.tar.bz2',
        'chk': '2d51a6529526ef22484f51567e31a5c346a599767991a3dc9d4dcd9d9cec71dd',
        'cmd': (
            (_cmake(), '-DLIBTYPE=STATIC', '-DCMAKE_BUILD_TYPE=Release', '-DALSOFT_EMBED_HRTF_DATA=YES', '.'),
            ('make', 'install')
        )
    },

    'pcre': {
        'url': 'https://ftp.pcre.org/pub/pcre/pcre-8.41.tar.bz2',
        'chk': 'e62c7eac5ae7c0e7286db61ff82912e1c0b7a0c13706616e94a7dd729321b530',
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared', '--enable-unicode-properties'),
            ('make', 'install')
        )
    },

    'pkg-config': {
        'url': 'https://pkg-config.freedesktop.org/releases/pkg-config-0.29.2.tar.gz',
        'chk': '6fc69c01688c9458a57eb9a1664c9aba372ccda420a02bf4429fe610e7e7d591',
        'cmd': (
            ('./configure', '--with-internal-glib'),
            ('make', 'install')
        )
    },

    'sndfile': {
        'url': 'http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28.tar.gz',
        'chk': '1ff33929f042fa333aed1e8923aa628c3ee9e1eb85512686c55092d1e5a9dfa9',
        'dep': ('pkg-config', 'ogg', 'vorbis', 'flac'),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    },

    'vorbis': {
        'url': 'https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.xz',
        'chk': '54f94a9527ff0a88477be0a71c0bab09a4c3febe0ed878b24824906cd4b0e1d1',
        'dep': ('ogg',),
        'cmd': (
            ('./configure', '--enable-static', '--disable-shared'),
            ('make', 'install')
        )
    }
}
