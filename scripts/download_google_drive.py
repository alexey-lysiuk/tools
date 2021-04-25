#!/usr/bin/env python3

import cgi
import re
import os
import sys
import urllib.parse

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/development/devbuild')
sys.dont_write_bytecode = True

import requests


# Based on https://stackoverflow.com/a/39225039

def get_file_id(request: str) -> str:
    url_patterns = (
        r'https://drive.google.com/file/d/([^/]+)/?.*',
        r'https://drive.google.com/open\?id=([^/]+)'
    )

    for pattern in url_patterns:
        url_match = re.match(pattern, request, re.IGNORECASE)
        if url_match:
            return url_match.group(1)

    return request


def get_filename(cd_params: dict) -> str:
    encoded_filename_directive = 'filename*'

    if encoded_filename_directive in cd_params:
        filename = cd_params[encoded_filename_directive]
        encoding, _, filename = filename.split("'")
        return urllib.parse.unquote(filename, encoding)

    return cd_params['filename']


def download(request: str) -> None:
    file_id = get_file_id(request)

    print(f'Requesting {file_id}')

    url = 'https://docs.google.com/uc?export=download&id=' + file_id
    session = requests.Session()
    response = session.get(url, stream=True)

    token = None

    for key, value in response.cookies.items():
        if key.startswith('download_warning'):
            token = value
            break

    if token:
        url = url + '&confirm=' + token
        response = session.get(url, stream=True)

    content_disposition = response.headers['content-disposition']
    _, cd_params = cgi.parse_header(content_disposition)
    filename = get_filename(cd_params)
    total = 0

    with open(filename, 'wb') as f:
        for chunk in response.iter_content(1024 * 1024):
            if chunk:
                f.write(chunk)
                total += len(chunk)

                sys.stdout.write(f'\rDownloading {filename}: {total:,} bytes')
                sys.stdout.flush()

    print('')


def main():
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} id-or-url ...')
        return

    for request in sys.argv[1:]:
        download(request)


if __name__ == '__main__':
    main()
