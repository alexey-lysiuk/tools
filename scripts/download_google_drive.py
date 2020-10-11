#!/usr/bin/env python3

import cgi
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/development/devbuild')
sys.dont_write_bytecode = True

import requests


# Based on https://stackoverflow.com/a/39225039

def download(file_id: str):
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
    filename = cd_params['filename']
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
        print(f'Usage: {sys.argv[0]} file-id ...')
        return

    for file_id in sys.argv[1:]:
        download(file_id)


if __name__ == '__main__':
    main()
