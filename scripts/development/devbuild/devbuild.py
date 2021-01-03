#!/usr/bin/env python3

import base64
import collections
import hashlib
import importlib
import os
import plistlib
import subprocess
import shutil
import sys
import tarfile
import time
import zlib

import github3
import ptyprocess


class BuildState:
    Target = collections.namedtuple('Target', 'name identifier')

    _TARGETS_LIST = (
        Target('GZDoom', 'org.drdteam.gzdoom'),
        Target('QZDoom', 'org.drdteam.qzdoom'),
        Target('LZDoom', 'org.zdoom.lzdoom'),
        Target('Raze', 'org.zdoom.raze'),
    )

    TARGETS = {target.name.lower(): target for target in _TARGETS_LIST}

    def __init__(self, target: str, commit: str):
        target = target.lower()
        self.target = BuildState.TARGETS[target]

        self.zdoom_project = self.target.name  # TODO: rename to 'target'
        self.commit = commit
        self.zdoom_os_min_ver = '10.9'  # TODO: rename to 'target_min_ver'

        # TODO: rename to 'target_lower'
        self.zdoom_project_low = target

        # Source directories
        self.script_dir = os.path.dirname(os.path.abspath(__file__)) + os.sep
        self.src_base_dir = '/Volumes/Storage/Work/devbuilds/'
        self.src_deps_dir = self.src_base_dir + 'zdoom-macos-deps' + os.sep
        # TODO: rename to 'src_target_dir'
        self.src_zdoom_dir = self.src_base_dir + self.zdoom_project_low + os.sep
        self.src_widepix_dir = self.src_base_dir + 'WidePix' + os.sep

        # Build directories
        self.base_dir = f'/Volumes/ramdisk/{self.zdoom_project_low}-devbuild/'
        self.deps_dir = self.base_dir + 'deps' + os.sep
        # TODO: rename to 'target_dir'
        self.zdoom_dir = self.base_dir + self.zdoom_project_low + os.sep
        self.build_dir = self.base_dir + 'build' + os.sep
        self.dist_dir = self.base_dir + 'dist' + os.sep

        self.bundle_name = self.zdoom_project + '.app'
        self.bundle_path = self.dist_dir + self.bundle_name

        self.disk_image_name = None
        self.disk_image_filename = None
        self.disk_image_path = None
        self.disk_image_content = None
        self.disk_image_checksum = None

        self.deps_commit = None
        self.zdoom_version = None
        self.zdoom_commit = None

        self.deployment_config = {}

        # LZDoom doesn't have separate devbuilds repository
        # TODO: rename to 'target_devbuilds'
        self.zdoom_devbuilds = 'gzdoom' if self.zdoom_project_low == 'lzdoom' else self.zdoom_project_low
        self.zdoom_devbuilds += '-macos-devbuilds'

    @staticmethod
    def _run(args, cwd: str) -> str:
        return subprocess.check_output(args, cwd=cwd).decode('utf-8').strip('\n')

    def prepare_directories(self):
        args = ('git', 'fetch', '--all', '--tags')
        subprocess.check_call(args, cwd=self.src_deps_dir)
        subprocess.check_call(args, cwd=self.src_zdoom_dir)
        subprocess.check_call(args, cwd=self.src_widepix_dir)

        os.makedirs(self.base_dir)

        clone_args = ('git', 'clone', '-s')
        args = clone_args + (self.src_deps_dir, self.deps_dir)
        subprocess.check_call(args, cwd=self.base_dir)
        args = clone_args + (self.src_zdoom_dir, self.zdoom_dir)
        subprocess.check_call(args, cwd=self.base_dir)

        args = ('git', 'checkout', self.zdoom_os_min_ver)
        subprocess.check_call(args, cwd=self.deps_dir)

        if self.commit:
            args = ('git', 'checkout', self.commit)
            subprocess.check_call(args, cwd=self.zdoom_dir)

        args = ('git', 'submodule', 'update', '--init', '--recursive', '--reference', self.src_widepix_dir)
        subprocess.check_call(args, cwd=self.zdoom_dir)

    def setup_target(self):
        log_commit_args = ('git', 'log', '--pretty=format:%h', '-n', '1')
        self.deps_commit = BuildState._run(log_commit_args, self.deps_dir)

        args = ('git', 'describe', '--tags')
        # TODO: rename
        self.zdoom_version = BuildState._run(args, self.zdoom_dir)
        # TODO: rename
        self.zdoom_commit = BuildState._run(log_commit_args, self.zdoom_dir)

        self.disk_image_name = self.zdoom_project + '-' + self.zdoom_version
        self.disk_image_filename = self.disk_image_name.lower() + '.dmg'
        self.disk_image_path = self.base_dir + self.disk_image_filename

    def build_target(self):
        sys.path.append(self.deps_dir)

        build_module = importlib.import_module('build')
        builder = build_module.__dict__['Builder']

        args = (
            f'--source-path={self.zdoom_dir}',
            f'--output-path={self.dist_dir}',
            f'--sdk-path-x64={self.src_base_dir}/macos_sdk/MacOSX{self.zdoom_os_min_ver}.sdk',
            # ARM64 target uses the latest 11.x SDK for now
        )
        builder(args).run()

    def prepare_app_bundle(self):
        output_path = self.dist_dir + os.sep + self.zdoom_project_low + os.sep
        os.rename(output_path + self.zdoom_project_low + '.app', self.bundle_path)
        os.rmdir(output_path)

        apps = '/Applications'
        os.symlink(apps, self.dist_dir + apps)

        src_licenses = self.zdoom_dir + 'docs/licenses'
        if os.path.exists(src_licenses):
            dst_target_licenses = self.dist_dir + 'Licenses'
            shutil.copytree(src_licenses, dst_target_licenses)

            if os.path.exists(self.bundle_path + '/Contents/MacOS/libMoltenVK.dylib'):
                shutil.copy(self.deps_dir + 'deps/moltenvk/apache2.txt', dst_target_licenses)

        info_plist = self.bundle_path + '/Contents/Info.plist'
        with open(info_plist, 'rb') as f:
            target_plist = plistlib.load(f)

        target_plist['LSMinimumSystemVersion'] = self.zdoom_os_min_ver
        target_plist['CFBundleName'] = self.zdoom_project
        target_plist['CFBundleShortVersionString'] = self.zdoom_version
        target_plist['CFBundleIdentifier'] = self.target.identifier
        target_plist['ZDependenciesRepositoryCommit'] = self.deps_commit

        with open(info_plist, 'wb') as f:
            plistlib.dump(target_plist, f)

    def compress_bundle(self):
        # create .tar.bz2 containing app bundle for "special" builds
        if not self.commit:
            return

        archive_path = f'{self.base_dir}{self.zdoom_project_low}-{self.commit}.tar.bz2'

        with tarfile.open(archive_path, 'w:bz2') as archive:
            old_cwd = os.getcwd()
            os.chdir(self.dist_dir)

            archive.add(self.bundle_name)

            os.chdir(old_cwd)

    def create_disk_image(self):
        args = ('codesign', '--sign', '-', '--deep', self.bundle_path)
        subprocess.run(args, check=True)

        tmp_dmg_path = self.disk_image_path + '-tmp.dmg'

        args = (
            'hdiutil', 'create',
            '-srcfolder', self.dist_dir,
            '-format', 'UDRO',
            '-volname', self.disk_image_name,
            tmp_dmg_path,
        )
        subprocess.check_call(args)

        args = (
            'hdiutil', 'convert',
            tmp_dmg_path,
            '-format', 'UDBZ',
            '-imagekey', 'bzip2-level=9',
            '-o', self.disk_image_path,
        )
        subprocess.check_call(args)

    def read_disk_image(self):
        hasher = hashlib.sha256()

        with open(self.disk_image_path, 'rb') as f:
            self.disk_image_content = f.read()
            hasher.update(self.disk_image_content)

        self.disk_image_checksum = hasher.hexdigest()

    def update_devbuilds_repository(self):
        devbuilds_path = self.src_base_dir + self.zdoom_devbuilds + os.sep
        readme_filename = 'README.md'
        devbuilds_readme = devbuilds_path + readme_filename

        with open(devbuilds_readme, 'r') as f:
            readme_content = f.readlines()

        repo_url = 'https://github.com/alexey-lysiuk/' + self.zdoom_devbuilds
        download_url = f'{repo_url}/releases/download/{self.zdoom_version}/{self.disk_image_filename}'

        table_top = readme_content.index('|---|---|\n') + 1
        new_entry = f'|[`{self.zdoom_version}`]({download_url})|`{self.disk_image_checksum}`|\n'
        readme_content.insert(table_top, new_entry)

        with open(devbuilds_readme, 'w') as f:
            f.writelines(readme_content)

        args = ('git', 'add', readme_filename)
        subprocess.check_call(args, cwd=devbuilds_path)

        args = ('git', 'commit', '-m', '+ ' + self.zdoom_version)
        subprocess.check_call(args, cwd=devbuilds_path)

        args = ('git', 'push')
        subprocess.check_call(args, cwd=devbuilds_path)

    def load_deployment_config(self):
        deployment_config_path = self.src_base_dir + '.deploy_config/..namedfork/rsrc'

        with open(deployment_config_path) as f:
            deployment_config = f.read().strip('\n')

        code = b'789C4B2C2E4E2D2A5150D75257B0B55548492DC8C9AFCC4DCD2B894FCECF4BCB4C8F3688E54A' \
            b'494DCE4F494D51B055484A2C4E3533D14B323483886960AA37B48AD55108292A4DD5E4C290049A5' \
            b'0959399A407D29C5B50945A5CAC01355B530F6AA07A62717266A63A48738E0254920B006B9C393D'
        code = base64.b16decode(code, True)
        code = zlib.decompress(code).decode('ascii')

        local_vars = locals().copy()
        exec(code, globals(), local_vars)

        deployment_config = local_vars['deployment_config']
        deployment_config = deployment_config.split('\n')

        for assignment in deployment_config:
            assignment = assignment.split('=', 1)
            if len(assignment) != 2:
                continue

            name = assignment[0]
            value = assignment[1]
            self.deployment_config[name] = value

    def make_github_release(self):
        user = self.deployment_config['GITHUB_USER']
        token = self.deployment_config['GITHUB_TOKEN']

        print('Connecting to GitHub...')
        session = github3.login(user, token=token)
        repository = session.repository(user, self.zdoom_devbuilds)

        args = ('git', 'remote', 'get-url', 'origin')
        target_repository_url = BuildState._run(args, self.src_zdoom_dir)
        target_repository_url = target_repository_url.replace('https://github.com/', '')
        target_repository_url = target_repository_url.replace('.git', '')

        print('Creating GitHub release...')
        release_name = self.zdoom_project + ' ' + self.zdoom_version
        release_description = \
            f'Development build at {target_repository_url}@{self.zdoom_commit}\nSHA-256: {self.disk_image_checksum}'
        release = repository.create_release(
            self.zdoom_version, name=release_name, body=release_description, prerelease=True)

        print('Uploading GitHub release asset...')
        release.upload_asset('application/octet-stream', self.disk_image_filename, self.disk_image_content)

    def upload_to_drdteam(self):
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
        directory = directory_pattern % self.zdoom_project_low
        sftp.write(f'cd {directory}\n')
        wait_for_prompt()

        sftp.write(f'put -P "{self.disk_image_path}"\n')
        wait_for_prompt()

        sftp.write('bye\n')
        sftp.read()
        sftp.wait()


def _main():
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} target [commit]')
        exit(1)

    sys.dont_write_bytecode = True

    target = sys.argv[1]
    commit = sys.argv[2] if len(sys.argv) > 2 else None

    state = BuildState(target, commit)
    state.prepare_directories()
    state.setup_target()

    state.build_target()
    state.prepare_app_bundle()
    state.compress_bundle()
    state.create_disk_image()

    state.read_disk_image()
    state.update_devbuilds_repository()
    state.load_deployment_config()
    state.make_github_release()
    state.upload_to_drdteam()


if __name__ == '__main__':
    _main()
