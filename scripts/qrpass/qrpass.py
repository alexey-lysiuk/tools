#!/usr/bin/env python3

import random
import string
import sys

sys.dont_write_bytecode = True

import qrcode  # noqa: E402
from qrcode.image.pure import PyPNGImage  # noqa: E402


def _main():
    characters = string.digits + string.ascii_letters + '_'
    password = random.sample(characters, len(characters))
    password = ''.join(password)
    print('Password:', password)

    image = qrcode.make(password, image_factory=PyPNGImage)
    image.save('qrpass.png')


if __name__ == '__main__':
    _main()
