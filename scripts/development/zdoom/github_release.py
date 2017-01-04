#!/usr/bin/env python

import sys
import os
import traceback
import github3

if 8 != len(sys.argv):
    print('Usage: github_release <user> <token> <repository> <tag> <name> <description> <file-to-upload>')
    sys.exit(1)

class Config(object):
    pass

config = Config()
config.user = sys.argv[1]
config.token = sys.argv[2]
config.repo = sys.argv[3]
config.tag = sys.argv[4]
config.name = sys.argv[5]
config.desc = sys.argv[6].replace(r'\n', '\n')
config.path = sys.argv[7]

try:
    gh = github3.login(config.user, token=config.token)

    print('Connecting to GitHub...')
    repo = gh.repository(config.user, config.repo)

    print('Creating GitHub release...')
    release = repo.create_release(config.tag, name=config.name, body=config.desc, prerelease=True)

    with open(config.path, 'rb') as asset_file:
        asset_name = os.path.basename(config.path)

        print('Uploading GitHub release asset...')
        release.upload_asset('application/octet-stream', asset_name, asset_file)

except Exception as ex:
    print('\nERROR: Failed to create GitHub release\n')
    traceback.print_exc(file=sys.stdout)
    sys.exit(1)
