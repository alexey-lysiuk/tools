#!/usr/bin/env python3

import cgi
import re
import sys
import urllib.parse
import urllib.request


LINK_TYPES = (
    'fb2',
    'download',  # other formats
)


def download(link):
    try:
        response = urllib.request.urlopen(link)
        data = response.read()
        content_disposition = response.headers['content-disposition']

        if not content_disposition:
            raise RuntimeError('Could not obtain file content')

        _, cd_params = cgi.parse_header(content_disposition)

        with open(cd_params['filename'], 'wb') as f:
            f.write(data)

    except Exception as ex:
        print(f'ERROR: {ex}, {link} skipped')


def main():
    if len(sys.argv) < 2:
        print('Usage: %s url ...' % sys.argv[0])
        return 0

    for page_url in sys.argv[1:]:
        print('Downloading %s...' % page_url)
        page_response = urllib.request.urlopen(page_url)
        page_data = page_response.read()
        links = []

        for link_type in LINK_TYPES:
            links += re.findall(r'<a href="(/b/\d+/%s)">' % link_type, str(page_data))

        parsed_url = list(urllib.parse.urlsplit(page_url))
        parsed_url[2] = ''  # clear path
        domain = urllib.parse.urlunsplit(parsed_url)

        total = len(links)
        current = 1

        for link in links:
            link = urllib.parse.urljoin(domain, link)

            print('Downloading %s [%i of %i]...' % (link, current, total))
            download(link)

            current += 1


if '__main__' == __name__:
    main()
