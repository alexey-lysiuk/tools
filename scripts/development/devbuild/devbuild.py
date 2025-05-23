#!/usr/bin/env python3

import collections
import hashlib
import os
import plistlib
import subprocess
import shutil
import sys
import tarfile
import time

os.environ['PYTHONDONTWRITEBYTECODE'] = '1'
sys.dont_write_bytecode = True

import config  # noqa: E402
import github3  # noqa: E402
import ptyprocess  # noqa: E402


class BuildState:
    Target = collections.namedtuple('Target', 'name identifier')

    _TARGETS_LIST = (
        Target('GZDoom', 'org.drdteam.gzdoom'),
        Target('QZDoom', 'org.drdteam.qzdoom'),
        Target('LZDoom', 'org.zdoom.lzdoom'),
        Target('Raze', 'org.zdoom.raze'),
    )

    TARGETS = {target.name.lower(): target for target in _TARGETS_LIST}

    def __init__(self, target: str, checkout: str):
        self.target_name_lower = target.lower()
        self.target = BuildState.TARGETS[self.target_name_lower]
        self.checkout = checkout
        self.zip_package = True

        # Source directories
        self.script_dir = os.path.dirname(os.path.abspath(__file__)) + os.sep
        self.src_base_dir = '/Volumes/Storage/Work/devbuilds/'
        self.src_deps_dir = self.src_base_dir + 'zdoom-macos-deps' + os.sep
        self.src_target_dir = self.src_base_dir + self.target_name_lower + os.sep

        # Build directories
        self.base_dir = f'/Volumes/ramdisk/{self.target_name_lower}-devbuild/'
        self.deps_dir = self.base_dir + 'deps' + os.sep
        self.target_dir = self.base_dir + self.target_name_lower + os.sep
        self.build_dir = self.base_dir + 'build' + os.sep
        self.dist_dir = self.base_dir + 'dist' + os.sep

        self.bundle_name = self.target.name + '.app'
        self.bundle_path = self.dist_dir + self.bundle_name

        self.volume_name = ''
        self.package_filename = ''
        self.package_path = ''
        self.package_content = ''
        self.package_checksum = ''

        self.deps_commit = ''
        self.target_version = ''
        self.target_commit = ''

        self.deployment_config = {}

        # LZDoom doesn't have separate devbuilds repository
        self.target_devbuilds = 'gzdoom' if self.target_name_lower == 'lzdoom' else self.target_name_lower
        self.target_devbuilds += '-macos-devbuilds'

    @staticmethod
    def _run(args, cwd: str) -> str:
        return subprocess.check_output(args, cwd=cwd).decode('utf-8').strip('\n')

    def prepare_directories(self):
        args = ('git', 'fetch', '--all', '--tags')
        subprocess.check_call(args, cwd=self.src_deps_dir)
        subprocess.check_call(args, cwd=self.src_target_dir)

        os.makedirs(self.base_dir, exist_ok=True)

        if os.path.exists(self.dist_dir):
            shutil.rmtree(self.dist_dir)

        clone_args = ('git', 'clone', '--shared')

        if not os.path.exists(self.deps_dir):
            args = clone_args + (self.src_deps_dir, self.deps_dir)
            subprocess.check_call(args, cwd=self.base_dir)

            args = ('git', 'submodule', 'update', '--init', '--recursive',
                '--reference', self.src_base_dir + 'aedi-core')
            subprocess.check_call(args, cwd=self.deps_dir)

        if not os.path.exists(self.target_dir):
            args = clone_args + (self.src_target_dir, self.target_dir)
            subprocess.check_call(args, cwd=self.base_dir)

        # TODO: get default branch name from repository
        commit = self.checkout or 'master'
        args = ('git', 'checkout', commit)
        subprocess.check_call(args, cwd=self.target_dir)

        if not self.checkout:
            args = ('git', 'pull', '--ff-only')
            subprocess.check_call(args, cwd=self.target_dir)

    def setup_target(self):
        log_commit_args = ('git', 'log', '--pretty=format:%h', '-n', '1')
        self.deps_commit = BuildState._run(log_commit_args, self.deps_dir)

        args = ('git', 'describe', '--tags')
        self.target_version = BuildState._run(args, self.target_dir)
        self.target_commit = BuildState._run(log_commit_args, self.target_dir)

        self.volume_name = self.target.name + '-' + self.target_version
        self.package_filename = self.volume_name.lower() + ('.zip' if self.zip_package else '.dmg')
        self.package_path = self.base_dir + self.package_filename

    def build_target(self):
        args = (
            './build.py',
            '--source=' + self.target_dir,
            '--output-path=' + self.dist_dir,
            '--static-moltenvk',
            '--quasi-glib',
        )
        subprocess.check_call(args, cwd=self.deps_dir)

    def prepare_app_bundle(self):
        output_path = self.dist_dir + os.sep + self.target_name_lower + os.sep
        os.rename(output_path + self.target_name_lower + '.app', self.bundle_path)
        os.rmdir(output_path)

        if not self.zip_package:
            apps = '/Applications'
            os.symlink(apps, self.dist_dir + apps)

        src_licenses = self.target_dir + 'docs/licenses'
        if os.path.exists(src_licenses):
            dst_target_licenses = self.dist_dir + 'Licenses'
            shutil.copytree(src_licenses, dst_target_licenses)

            if os.path.exists(self.bundle_path + '/Contents/MacOS/libMoltenVK.dylib'):
                shutil.copy(self.deps_dir + 'deps/moltenvk/apache2.txt', dst_target_licenses)

        info_plist = self.bundle_path + '/Contents/Info.plist'
        with open(info_plist, 'rb') as f:
            target_plist = plistlib.load(f)

        args = ('python3', '-c', 'import aedi.utility; print(aedi.utility.OS_VERSION_X86_64)')
        minver = BuildState._run(args, self.deps_dir + 'core')

        target_plist['LSMinimumSystemVersion'] = minver
        target_plist['CFBundleName'] = self.target.name
        target_plist['CFBundleShortVersionString'] = self.target_version
        target_plist['CFBundleIdentifier'] = self.target.identifier
        target_plist['ZDependenciesRepositoryCommit'] = self.deps_commit

        with open(info_plist, 'wb') as f:
            plistlib.dump(target_plist, f)

    def _create_zip_package(self):
        args = ('ditto', '-c', '-k', '--zlibCompressionLevel', '9', self.dist_dir, self.package_path)
        subprocess.check_call(args)

    def _sign_bunble(self):
        args = ('codesign', '--sign', '-', '--deep', '--force', self.bundle_path)
        subprocess.check_call(args)

    def _compress_bundle(self):
        # create .tar.bz2 containing app bundle for "special" builds
        if self.zip_package or not self.checkout:
            return

        archive_path = f'{self.base_dir}{self.target_name_lower}-{self.target_commit}.tar.bz2'

        with tarfile.open(archive_path, 'w:bz2') as archive:
            old_cwd = os.getcwd()
            os.chdir(self.dist_dir)

            archive.add(self.bundle_name)

            os.chdir(old_cwd)

    def _create_disk_image(self):
        tmp_dmg_path = self.package_path + '-tmp.dmg'

        if os.path.exists(tmp_dmg_path):
            os.remove(tmp_dmg_path)

        args = (
            'hdiutil', 'create',
            '-srcfolder', self.dist_dir,
            '-format', 'UDRO',
            '-volname', self.volume_name,
            tmp_dmg_path,
        )
        subprocess.check_call(args)

        args = (
            'hdiutil', 'convert',
            tmp_dmg_path,
            '-format', 'UDBZ',
            '-imagekey', 'bzip2-level=9',
            '-o', self.package_path,
        )
        subprocess.check_call(args)

    def _read_package(self):
        hasher = hashlib.sha256()

        with open(self.package_path, 'rb') as f:
            self.package_content = f.read()
            hasher.update(self.package_content)

        self.package_checksum = hasher.hexdigest()

    def create_package(self):
        if os.path.exists(self.package_path):
            os.remove(self.package_path)

        if self.zip_package:
            self._sign_bunble()
            self._create_zip_package()
        else:
            self._compress_bundle()
            self._sign_bunble()
            self._create_disk_image()

        self._read_package()

    def update_devbuilds_repository(self):
        devbuilds_path = self.src_base_dir + self.target_devbuilds + os.sep
        readme_filename = 'README.md'
        devbuilds_readme = devbuilds_path + readme_filename

        with open(devbuilds_readme, 'r') as f:
            readme_content = f.readlines()

        repo_url = 'https://github.com/alexey-lysiuk/' + self.target_devbuilds
        download_url = f'{repo_url}/releases/download/{self.target_version}/{self.package_filename}'

        table_top = readme_content.index('|---|---|\n') + 1
        new_entry = f'|[`{self.target_version}`]({download_url})|`{self.package_checksum}`|\n'
        readme_content.insert(table_top, new_entry)

        with open(devbuilds_readme, 'w') as f:
            f.writelines(readme_content)

        args = ('git', 'add', readme_filename)
        subprocess.check_call(args, cwd=devbuilds_path)

        args = ('git', 'commit', '-m', '+ ' + self.target_version)
        subprocess.check_call(args, cwd=devbuilds_path)

        args = ('git', 'push')
        subprocess.check_call(args, cwd=devbuilds_path)

    def load_deployment_config(self):
        self.deployment_config = config.load(self.src_base_dir)

    def make_github_release(self):
        user = self.deployment_config['GITHUB_USER']
        token = self.deployment_config['GITHUB_TOKEN']

        print('Connecting to GitHub...')
        session = github3.login(user, token=token)
        repository = session.repository(user, self.target_devbuilds)

        args = ('git', 'remote', 'get-url', 'origin')
        target_repository_url = BuildState._run(args, self.src_target_dir)
        target_repository_url = target_repository_url.replace('https://github.com/', '')
        target_repository_url = target_repository_url.replace('.git', '')

        print('Creating GitHub release...')
        release_name = self.target.name + ' ' + self.target_version
        release_description = \
            f'Development build at {target_repository_url}@{self.target_commit}\nSHA-256: {self.package_checksum}'
        release = repository.create_release(
            self.target_version, name=release_name, body=release_description, prerelease=True)

        print('Uploading GitHub release asset...')
        release.upload_asset('application/octet-stream', self.package_filename, self.package_content)

    def upload_to_drdteam(self):
        print('Connecting to DRD Team...')
        login = self.deployment_config['SFTP_LOGIN']
        host = self.deployment_config['SFTP_HOST']

        args = ['sftp', login + '@' + host]
        sftp = ptyprocess.PtyProcessUnicode.spawn(args)

        def wait_for_output(message: str):
            while not sftp.read().endswith(message):
                time.sleep(0.1)

        def wait_for_prompt():
            wait_for_output('sftp> ')

        wait_for_output('password: ')

        password = self.deployment_config['SFTP_PASS']
        sftp.write(password + '\n')
        wait_for_prompt()

        directory_pattern = self.deployment_config['SFTP_DIR']
        directory = directory_pattern % self.target_name_lower
        sftp.write(f'cd {directory}\n')
        wait_for_prompt()

        print('Uploading build to DRD Team...')
        sftp.write(f'put -P "{self.package_path}"\n')
        wait_for_prompt()

        sftp.write('bye\n')
        sftp.read()
        sftp.wait()


def _main():
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} target [commit]')
        exit(1)

    target = sys.argv[1]
    checkout = sys.argv[2] if len(sys.argv) > 2 else ''

    state = BuildState(target, checkout)
    state.prepare_directories()
    state.setup_target()

    state.build_target()
    state.prepare_app_bundle()
    state.create_package()

    state.update_devbuilds_repository()
    state.load_deployment_config()
    state.make_github_release()
    # state.upload_to_drdteam()


if __name__ == '__main__':
    _main()
