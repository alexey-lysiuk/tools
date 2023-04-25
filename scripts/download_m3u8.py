#!/usr/bin/env python3

import sys
import typing
import urllib.parse
import urllib.request


def gather_chunks(m3u8_url: str) -> typing.List[str]:
    print('Downloading %s...' % m3u8_url)

    m3u8_response = urllib.request.urlopen(m3u8_url)
    m3u8_data = m3u8_response.read().decode()

    base_url = ''
    links = []

    for line in m3u8_data.split('\n'):
        if not line or line.startswith('#'):
            continue

        if not line.startswith('http'):
            parsed_url = list(urllib.parse.urlsplit(m3u8_url))
            path = parsed_url[2]
            parsed_url[2] = path[:path.rfind('/') + 1]  # remove filename from path
            parsed_url[3] = ''  # clear query
            base_url = urllib.parse.urlunsplit(parsed_url)

        links.append(base_url + line)

    return links


def download(link) -> typing.Optional[bytes]:
    result = None

    try:
        response = urllib.request.urlopen(link)
        data = response.read()
        size = len(data)

        if size > 10 * 1024:
            print(f'Chunk retrieved [{size:,} bytes]')
            result = data
        else:
            print(f'Chunk skipped because of its size [{size} bytes]')

    except Exception as ex:
        print(f'ERROR: {ex}, {link} skipped')

    return result


def main():
    if len(sys.argv) < 3:
        print(f'Usage: {sys.argv[0]} url output')
        return 0

    output_file = open(sys.argv[2], 'wb')

    chunks = gather_chunks(sys.argv[1])
    total = len(chunks)
    current = 1

    for chunk in chunks:
        print('Downloading %s [%i of %i]...' % (chunk, current, total))

        if data := download(chunk):
            output_file.write(data)

        current += 1


if '__main__' == __name__:
    main()
