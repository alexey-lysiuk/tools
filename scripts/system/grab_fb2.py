#!/usr/bin/env python3

import cgi
import re
import sys
import urllib.parse
import urllib.request


def main():
    if len(sys.argv) < 2:
        print('Usage: %s url ...' % sys.argv[0])
        return 0

    for page_url in sys.argv[1:]:
        print('Downloading %s...' % page_url)
        page_response = urllib.request.urlopen(page_url)
        page_data = page_response.read()
        links = re.findall(r'<a href="(/b/\d+/fb2)">', str(page_data))

        parsed_url = list(urllib.parse.urlsplit(page_url))
        parsed_url[2] = ''  # clear path
        domain = urllib.parse.urlunsplit(parsed_url)

        total = len(links)
        current = 1

        for link in links:
            fb2_url = urllib.parse.urljoin(domain, link)

            print('Downloading %s [%i of %i]...' % (fb2_url, current, total))
            fb2_response = urllib.request.urlopen(fb2_url)
            fb2_data = fb2_response.read()

            content_disposition = fb2_response.headers['content-disposition']
            _, cd_params = cgi.parse_header(content_disposition)

            with open(cd_params['filename'], 'wb') as f:
                f.write(fb2_data)

            current += 1


if '__main__' == __name__:
    main()
