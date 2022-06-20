#!/usr/bin/env python3

import cgi
import re
import sys
import typing
import urllib.parse
import urllib.request


def _gather_links_kind1(page_url: str, page_data: bytearray) -> typing.List[str]:
    parsed_url = list(urllib.parse.urlsplit(page_url))
    parsed_url[2] = ''  # clear path
    domain = urllib.parse.urlunsplit(parsed_url)

    link_suffices = (
        'fb2',
        'download',  # other formats
    )

    links = []

    for link_suffix in link_suffices:
        links += re.findall(r'<a href="(/b/\d+/%s)">' % link_suffix, str(page_data))

    return [urllib.parse.urljoin(domain, link) for link in links]


def _gather_links_kind2(page_url: str, page_data: bytearray) -> typing.List[str]:
    subpages = re.findall(r'<a href="(%s[^"]+)/" ' % page_url, str(page_data))
    links = []

    for subpage in subpages:
        subpage_response = urllib.request.urlopen(subpage)
        subpage_data = subpage_response.read()

        links += re.findall(r'<a href="(%s[^"]+)">' % page_url, str(subpage_data))

    def known_ext(link: str) -> bool:
        for ext in ('.djvu', '.epub', '.pdf', '.zip'):
            if link.endswith(ext):
                return True
        return False

    return [link for link in links if known_ext(link)]


def gather_links(page_url: str) -> typing.List[str]:
    print('Downloading %s...' % page_url)
    page_response = urllib.request.urlopen(page_url)
    page_data = page_response.read()

    gatherers = (_gather_links_kind1, _gather_links_kind2)
    links = []

    for gatherer in gatherers:
        links += gatherer(page_url, page_data)

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
        print('Usage: %s url ...' % sys.argv[0])
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
