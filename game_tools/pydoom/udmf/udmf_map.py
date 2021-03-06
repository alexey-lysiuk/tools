#
#    Python Doom Tools
#    Copyright (C) 2016 Alexey Lysiuk
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

from collections import OrderedDict


def _unquote(string):
    length = len(string)
    can_unquote = length >= 2 and string.startswith('"') and string.endswith('"')
    return string[1:length - 1] if can_unquote else None


_TRUE_KEYWORD = 'true'
_FALSE_KEYWORD = 'false'


def _string2value(string):
    string = string.strip()

    unquoted_string = _unquote(string)
    if unquoted_string is not None:
        return unquoted_string

    lowercase_string = string.lower()
    if _TRUE_KEYWORD == lowercase_string:
        return True
    elif _FALSE_KEYWORD == lowercase_string:
        return False

    try:
        return int(string)
    except ValueError:
        pass

    try:
        return float(string)
    except ValueError:
        pass

    return string


def _value2string(value):
    if isinstance(value, basestring):
        return '"' + value + '"'
    elif isinstance(value, bool):
        return _TRUE_KEYWORD if value else _FALSE_KEYWORD
    elif isinstance(value, float):
        return '%.03f' % float(value)
    else:
        return str(value)


class UDMFMap(object):
    def __init__(self):
        self.namespace = ''
        self.things = []
        self.vertices = []
        self.linedefs = []
        self.sidedefs = []
        self.sectors = []

    def append(self, keyword, value):
        if isinstance(value, list) or isinstance(value, tuple):
            value_dict = OrderedDict()
            for item in value:
                value_dict[item[0]] = _string2value(item[1])
            value = value_dict
        elif isinstance(value, dict):
            for item in value:
                value[item] = _string2value(value[item])

        list_attr = self._getlist(keyword)

        if list_attr is not None:
            list_attr.append(value)
        else:
            setattr(self, keyword, _string2value(value))

    def astext(self):
        result = 'namespace = "{}";'.format(self.namespace)
        lists = self._getlists()

        for list_name in lists:
            index = 0

            for values in lists[list_name]:
                result += '\n{} // {}\n{{\n'.format(list_name, index)

                for value_name in values:
                    value = _value2string(values[value_name])
                    result += '{} = {};\n'.format(value_name, value)

                result += '}\n'

                index += 1

        # TODO: add unknown values

        return result + '\n'

    def _getlists(self):
        result = OrderedDict()
        result['thing'] = self.things
        result['vertex'] = self.vertices
        result['linedef'] = self.linedefs
        result['sidedef'] = self.sidedefs
        result['sector'] = self.sectors
        return result

    def _getlist(self, keyword):
        lists = self._getlists()
        return lists[keyword] if keyword in lists else None
