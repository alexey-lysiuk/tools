#!/bin/sh

set -o errexit

if [ -z "${ZDOOM_PROJECT}" ]; then
	echo "\nThis script should not be used directly\n"
	exit 1
fi

SCRIPT_DIR=$(cd "${0%/*}"; pwd)/

ZDOOM_PROJECT_LOW=$(echo ${ZDOOM_PROJECT} | tr '[:upper:]' '[:lower:]')

if [ -z "${ZDOOM_OS_MIN_VER}" ]; then
	ZDOOM_OS_MIN_VER=10.9
fi

SRC_BASE_DIR=/Volumes/Storage/Work/devbuilds/
SRC_DEPS_DIR=${SRC_BASE_DIR}zdoom-macos-deps/
SRC_ZDOOM_DIR=${SRC_BASE_DIR}${ZDOOM_PROJECT_LOW}/
SRC_WIDEPIX_DIR=${SRC_BASE_DIR}WidePix/

BASE_DIR=/Volumes/ramdisk/${ZDOOM_PROJECT_LOW}-devbuild/
DEPS_DIR=${BASE_DIR}deps/
ZDOOM_DIR=${BASE_DIR}${ZDOOM_PROJECT_LOW}/
BUILD_DIR=${BASE_DIR}build/
DIST_DIR=${BASE_DIR}dist/

# -----------------------------------------------------------------------------
# Prepare build environment
# ----------------------------------------------------------------------------

cd "${SRC_DEPS_DIR}"
git fetch --all

cd "${SRC_ZDOOM_DIR}"
git fetch --all --tags

cd "${SRC_WIDEPIX_DIR}"
git fetch --all

mkdir "${BASE_DIR}"

cd "${BASE_DIR}"
git clone -s "${SRC_DEPS_DIR}" "${DEPS_DIR}"
git clone -s "${SRC_ZDOOM_DIR}" "${ZDOOM_DIR}"

cd "${DEPS_DIR}"
git checkout "${ZDOOM_OS_MIN_VER}"

DEPS_COMMIT=$(git log --pretty=format:'%h' -n 1)

cd "${ZDOOM_DIR}"

if [ -n "$1" ]; then
	git checkout "$1"
fi

git submodule update --init --recursive --reference "${SRC_WIDEPIX_DIR}"

ZDOOM_VERSION=$(git describe --tags)
ZDOOM_COMMIT=$(git log --pretty=format:'%h' -n 1)

# -----------------------------------------------------------------------------
# Do a build
# -----------------------------------------------------------------------------

"${DEPS_DIR}/build.py" \
	--source-path="${ZDOOM_DIR}" \
	--build-path="${ZDOOM_DIR}/build" \
	--os-version="${ZDOOM_OS_MIN_VER}" \
	--sdk-path="${SRC_BASE_DIR}/macos_sdk/MacOSX${ZDOOM_OS_MIN_VER}.sdk"

# -----------------------------------------------------------------------------
# Create disk image
# -----------------------------------------------------------------------------

BUNDLE_PATH=${DIST_DIR}${ZDOOM_PROJECT}.app
INFO_PLIST_PATH=${BUNDLE_PATH}/Contents/Info.plist
MOLTENVK_DIR=${DEPS_DIR}deps/moltenvk/

mkdir "${DIST_DIR}"
cp -R "${ZDOOM_DIR}/build/${ZDOOM_PROJECT_LOW}.app" "${BUNDLE_PATH}"
ln -s /Applications "${DIST_DIR}/Applications"

if [ -e "${ZDOOM_DIR}docs/licenses" ]; then
	cp -R "${ZDOOM_DIR}docs/licenses" "${DIST_DIR}Licenses"
fi

if [ ! -z "${ZDOOM_VULKAN}" ]; then
	cp "${MOLTENVK_DIR}lib/libMoltenVK.dylib" "${BUNDLE_PATH}/Contents/MacOS/"
	cp "${MOLTENVK_DIR}apache2.txt" "${DIST_DIR}Licenses"
fi

plutil -replace LSMinimumSystemVersion -string "${ZDOOM_OS_MIN_VER}" "${INFO_PLIST_PATH}"
plutil -replace CFBundleName -string "${ZDOOM_PROJECT}" "${INFO_PLIST_PATH}"
plutil -replace CFBundleShortVersionString -string "${ZDOOM_VERSION}" "${INFO_PLIST_PATH}"
plutil -replace CFBundleIdentifier -string "${ZDOOM_IDENTIFIER}" "${INFO_PLIST_PATH}"
plutil -replace ZDependenciesRepositoryCommit -string "${DEPS_COMMIT}" "${INFO_PLIST_PATH}"

DMG_NAME=${ZDOOM_PROJECT}-${ZDOOM_VERSION}
DMG_FILENAME=$(echo ${DMG_NAME}.dmg | tr '[:upper:]' '[:lower:]')
DMG_PATH=${BASE_DIR}${DMG_FILENAME}
TMP_DMG_PATH=${BASE_DIR}${ZDOOM_PROJECT}-tmp.dmg

hdiutil makehybrid -o "${TMP_DMG_PATH}" "${DIST_DIR}" -hfs -hfs-volume-name "${DMG_NAME}"
hdiutil convert -format UDBZ -imagekey bzip2-level=9 "${TMP_DMG_PATH}" -o "${DMG_PATH}"
rm "${TMP_DMG_PATH}"

if [ -n "$1" ]; then
	# create .tar.bz2 containing app bundle for "special" builds
	cd "${DIST_DIR}"
	tar -c ${ZDOOM_PROJECT}.app | bzip2 -1 > "${BASE_DIR}${ZDOOM_PROJECT_LOW}.app.tar.bz2"
fi

# -----------------------------------------------------------------------------
# Prepare deployment environment
# -----------------------------------------------------------------------------

DEPLOY_CONFIG_PATH=${SRC_BASE_DIR}.deploy_config

if [ ! -e "${DEPLOY_CONFIG_PATH}" ]; then
	tput setaf 1
	tput bold
	echo "\nDeployment configuration file was not found!\n"
	tput sgr0
	exit 1
fi

DEPLOY_CONFIG=$(cat "${DEPLOY_CONFIG_PATH}/..namedfork/rsrc")
eval `python3 -c "import base64,sys,zlib;print((b'*'+base64.b16encode(zlib.compress(bytes(sys.argv[1].lower(),'ascii')))if'*'!=sys.argv[1][0]else zlib.decompress(base64.b16decode(sys.argv[1][1:],True))).decode('ascii'))" "${DEPLOY_CONFIG}"`

cd "${SRC_ZDOOM_DIR}"
ZDOOM_REPO=$(git remote get-url origin)
ZDOOM_REPO=${ZDOOM_REPO/https:\/\/github.com\//}
ZDOOM_REPO=${ZDOOM_REPO/.git/}

# -----------------------------------------------------------------------------
# Update devbuilds Git repository
# -----------------------------------------------------------------------------

TMP_CHECKSUM=$(shasum -a 256 "${DMG_PATH}")
DMG_CHECKSUM=${TMP_CHECKSUM:0:64}

if [ "${ZDOOM_PROJECT_LOW}" = "lzdoom" ]; then
	ZDOOM_DEVBUILDS=gzdoom-macos-devbuilds
else
	ZDOOM_DEVBUILDS=${ZDOOM_PROJECT_LOW}-macos-devbuilds
fi

DEVBUILDS_DIR=${SRC_BASE_DIR}${ZDOOM_DEVBUILDS}/

REPO_URL=https://github.com/alexey-lysiuk/${ZDOOM_DEVBUILDS}
DOWNLOAD_URL=${REPO_URL}/releases/download/${ZDOOM_VERSION}/${DMG_FILENAME}

cd "${DEVBUILDS_DIR}"
awk "/\|---\|---\|/ { print; print \"|[\`${ZDOOM_VERSION}\`](${DOWNLOAD_URL})|\`${DMG_CHECKSUM}\`|\"; next }1" README.md > README.tmp
rm README.md
mv README.tmp README.md

git add .
# TODO: use token
git commit -m "+ ${ZDOOM_VERSION}"
git push

# -----------------------------------------------------------------------------
# Create GitHub release
# -----------------------------------------------------------------------------

python3 -B ${SCRIPT_DIR}github_release.py \
	"${GITHUB_USER}" \
	"${GITHUB_TOKEN}" \
	"${ZDOOM_DEVBUILDS}" \
	"${ZDOOM_VERSION}" \
	"${ZDOOM_PROJECT} ${ZDOOM_VERSION}" \
	"Development build at ${ZDOOM_REPO}@${ZDOOM_COMMIT}\nSHA-256: ${DMG_CHECKSUM}" \
	"${DMG_PATH}"

# -----------------------------------------------------------------------------
# Upload to DRD Team
# -----------------------------------------------------------------------------

SSHPASS_DIR=${SCRIPT_DIR}/sshpass
SSHPASS_EXE=${SSHPASS_DIR}/sshpass

if [ ! -e "${SSHPASS_EXE}" ]; then
	cd ${SSHPASS_DIR}
	cc -o sshpass -DHAVE_CONFIG_H=1 main.c
fi

"${SSHPASS_EXE}" -p ${SFTP_PASS} sftp -oBatchMode=no -b - ${SFTP_LOGIN}@${SFTP_HOST} <<EOF
	cd $(printf \"${SFTP_DIR}\" ${ZDOOM_PROJECT_LOW})
	put -P "${DMG_PATH}"
	bye
EOF
