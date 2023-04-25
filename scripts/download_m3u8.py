#!/usr/bin/env python3

import cgi
import sys
import typing
import urllib.parse
import urllib.request


def gather_links(m3u8_url: str) -> typing.List[str]:
    print('Downloading %s...' % m3u8_url)

    m3u8_response = urllib.request.urlopen(m3u8_url)
    m3u8_data = m3u8_response.read().decode()

    parsed_url = list(urllib.parse.urlsplit(m3u8_url))
    path = parsed_url[2]
    parsed_url[2] = path[:path.rfind('/') + 1]
    parsed_url[3] = ''  # clear query
    domain = urllib.parse.urlunsplit(parsed_url)

    links = []

    for line in m3u8_data.split('\n'):
        if line.startswith('#'):
            continue

        links.append(domain + line)

    return links


def download(link):
    try:
        response = urllib.request.urlopen(link)
        data = response.read()
        content_disposition = response.headers['content-disposition']

        if content_disposition:
            _, cd_params = cgi.parse_header(content_disposition)
            filename = cd_params['filename']
        elif response.url:
            filename = urllib.parse.urlsplit(response.url).path.split('/')[-1]
        else:
            raise RuntimeError('Could not obtain file name')

        with open(filename, 'wb') as f:
            written = f.write(data)

        print(f'Saved to {filename} [{written:,} bytes]')

    except Exception as ex:
        print(f'ERROR: {ex}, {link} skipped')


def main():
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} url ...')
        return 0

    for page_url in sys.argv[1:]:
        links = gather_links(page_url)
        total = len(links)
        current = 1

        for link in links:
            print('Downloading %s [%i of %i]...' % (link, current, total))
            download(link)

            current += 1


if '__main__' == __name__:
    main()
