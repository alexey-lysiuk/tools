#
#    Python Doom Tools
#    Copyright (C) 2015, 2016 Alexey Lysiuk
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

import sys
import time


class Profiler(object):
    def __init__(self, enable):
        self._enable = enable

        if self._enable:
            import cProfile

            self._profiler = cProfile.Profile()
            self._profiler.enable()
        else:
            # time.clock() doesn't return wall-time on UNIX,
            # but it has greater precision on Windows
            self._time_func = time.clock if 'win32' == sys.platform else time.time
            self._start_time = self._time_func()

    def close(self):
        if self._enable:
            self._profiler.disable()

            import io
            import pstats
            import sys

            profiling_stream = io.StringIO() if sys.hexversion >= 0x3000000 else io.BytesIO()
            stats = pstats.Stats(self._profiler,
                                 stream=profiling_stream).sort_stats('cumulative')
            stats.print_stats()

            print('\n')
            print(profiling_stream.getvalue())
        else:
            build_time = self._time_func() - self._start_time
            print('Completed in {0:.3f} seconds'.format(build_time))
