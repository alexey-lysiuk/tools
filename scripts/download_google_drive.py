#!/usr/bin/env python3

import cgi
import html.parser
import re
import os
import sys
import urllib.parse

import pyjsparser

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/development/devbuild')
sys.dont_write_bytecode = True

import requests


class FileIdParser(html.parser.HTMLParser):
    def __init__(self):
        super().__init__()

        self.file_ids = []
        self._in_script = False

    def handle_starttag(self, tag, attrs):
        if tag == 'script':
            self._in_script = True

    def handle_endtag(self, tag):
        self._in_script = False

    def handle_data(self, data: str):
        if not self._in_script:
            return

        if not data.startswith('AF_initDataCallback'):
            return

        ast = pyjsparser.parse(data)
        props = ast['body'][0]['expression']['arguments'][0]['properties']
        prop_key = None
        prop_data = None

        for prop in props:
            current_key = prop['key']['name']

            if current_key == 'key':
                prop_key = prop['value']['value']
            elif current_key == 'data':
                prop_data = prop['value']['elements']

        if prop_key != 'ds:4' or not prop_data:
            return

        for entry in prop_data[4]['elements']:
            file_id = entry['elements'][0]['value']
            self.file_ids.append(file_id)

    def error(self, message):
        pass


# Based on https://stackoverflow.com/a/39225039

def _get_file_id(request: str) -> str:
    url_patterns = (
        r'https://drive.google.com/file/d/([^/]+)/?.*',
        r'https://drive.google.com/open\?id=([^/]+)',
        r'https://drive.google.com/drive/folders/([^/]+)',
    )

    for pattern in url_patterns:
        url_match = re.match(pattern, request, re.IGNORECASE)
        if url_match:
            return url_match.group(1)

    return request


def _get_filename(cd_params: dict) -> str:
    encoded_filename_directive = 'filename*'

    if encoded_filename_directive in cd_params:
        filename = cd_params[encoded_filename_directive]
        encoding, _, filename = filename.split("'")
        return urllib.parse.unquote(filename, encoding)

    return cd_params['filename']


def _download(request: str) -> None:
    file_id = _get_file_id(request)

    print(f'Requesting {file_id}')

    url = 'https://docs.google.com/uc?export=download&id=' + file_id
    session = requests.Session()
    response = session.get(url, stream=True)

    if response.status_code != 200:
        _download_directory(file_id)
        return

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
    filename = _get_filename(cd_params)
    total = 0

    with open(filename, 'wb') as f:
        for chunk in response.iter_content(1024 * 1024):
            if chunk:
                f.write(chunk)
                total += len(chunk)

                sys.stdout.write(f'\rDownloading {filename}: {total:,} bytes')
                sys.stdout.flush()

    print('')


def _download_directory(dir_id: str) -> None:
    url = 'https://drive.google.com/drive/folders/' + dir_id
    session = requests.Session()
    response = session.get(url, stream=True)
    content = response.content.decode(response.encoding)

    parser = FileIdParser()
    parser.feed(content)

    for file_id in parser.file_ids:
        _download(file_id)


def main():
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} id-or-url ...')
        return

    for request in sys.argv[1:]:
        _download(request)


if __name__ == '__main__':
    main()
