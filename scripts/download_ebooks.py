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
            ebook_url = urllib.parse.urljoin(domain, link)

            print('Downloading %s [%i of %i]...' % (ebook_url, current, total))
            ebook_response = urllib.request.urlopen(ebook_url)
            ebook_data = ebook_response.read()

            content_disposition = ebook_response.headers['content-disposition']
            _, cd_params = cgi.parse_header(content_disposition)

            with open(cd_params['filename'], 'wb') as f:
                f.write(ebook_data)

            current += 1


if '__main__' == __name__:
    main()
