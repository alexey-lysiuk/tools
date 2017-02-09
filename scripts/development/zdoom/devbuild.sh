set -o errexit

SCRIPT_DIR=$(cd "${0%/*}"; pwd)/

ZDOOM_PROJECT_LOW=$(echo ${ZDOOM_PROJECT} | tr '[:upper:]' '[:lower:]')

if [ -z "${ZDOOM_OS_MIN_VER}" ]; then
	ZDOOM_OS_MIN_VER=10.7
fi

SRC_BASE_DIR=/Volumes/Storage/Work/devbuilds/
SRC_DEPS_DIR=${SRC_BASE_DIR}zdoom-macos-deps/
SRC_ZDOOM_DIR=${SRC_BASE_DIR}${ZDOOM_PROJECT_LOW}/

BASE_DIR=/Volumes/ramdisk/${ZDOOM_PROJECT_LOW}-devbuild/
DEPS_DIR=${BASE_DIR}deps/
ZDOOM_DIR=${BASE_DIR}${ZDOOM_PROJECT_LOW}/
BUILD_DIR=${BASE_DIR}build/
DIST_DIR=${BASE_DIR}dist/

# -----------------------------------------------------------------------------
# Prepare build environment
# ----------------------------------------------------------------------------

cd "${SRC_DEPS_DIR}"
git fetch

cd "${SRC_ZDOOM_DIR}"
git fetch

mkdir "${BASE_DIR}"

cd "${BASE_DIR}"
git clone -s "${SRC_DEPS_DIR}" "${DEPS_DIR}"
git clone -s "${SRC_ZDOOM_DIR}" "${ZDOOM_DIR}"

cd "${ZDOOM_DIR}"

if [ -n "$1" ]; then
	git checkout "$1"
fi

ZDOOM_VERSION=$(git describe --tags)
ZDOOM_COMMIT=$(git log --pretty=format:'%h' -n 1)

# -----------------------------------------------------------------------------
# Do a build
# -----------------------------------------------------------------------------

mkdir "${BUILD_DIR}"
cd "${BUILD_DIR}"

FMOD_DIR=${DEPS_DIR}fmodex/
OPENAL_DIR=${DEPS_DIR}openal/
MPG123_DIR=${DEPS_DIR}mpg123/
SNDFILE_DIR=${DEPS_DIR}sndfile/
FSYNTH_DIR=${DEPS_DIR}fluidsynth/
FSYNTH_LIB_PREFIX=${FSYNTH_DIR}lib/lib
FSYNTH_LIBS=${FSYNTH_LIB_PREFIX}fluidsynth.a\;${FSYNTH_LIB_PREFIX}glib-2.0.a\;${FSYNTH_LIB_PREFIX}intl.a
OTHER_LIBS=-liconv\ -L${DEPS_DIR}ogg/lib\ -logg\ -L${DEPS_DIR}vorbis/lib\ -lvorbis\ -lvorbisenc\ -L${DEPS_DIR}flac/lib\ -lFLAC
FRAMEWORKS=-framework\ AudioUnit\ -framework\ AudioToolbox\ -framework\ CoreAudio\ -framework\ CoreMIDI
LINKER_FLAGS=${OTHER_LIBS}\ ${FRAMEWORKS}

/Applications/CMake.app/Contents/bin/cmake               \
	-DCMAKE_BUILD_TYPE="Release"                         \
	-DCMAKE_OSX_DEPLOYMENT_TARGET="${ZDOOM_OS_MIN_VER}"  \
	-DCMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS}"           \
	-DOSX_COCOA_BACKEND=YES                              \
	-DDYN_OPENAL=NO                                      \
	-DDYN_FLUIDSYNTH=NO                                  \
	-DFORCE_INTERNAL_ZLIB=YES                            \
	-DFORCE_INTERNAL_JPEG=YES                            \
	-DFORCE_INTERNAL_BZIP2=YES                           \
	-DFORCE_INTERNAL_GME=YES                             \
	-DFMOD_INCLUDE_DIR="${FMOD_DIR}inc"                  \
	-DFMOD_LIBRARY="${FMOD_DIR}lib/libfmodex.dylib"      \
	-DOPENAL_INCLUDE_DIR="${OPENAL_DIR}include"          \
	-DOPENAL_LIBRARY="${OPENAL_DIR}lib/libopenal.a"      \
	-DMPG123_INCLUDE_DIR="${MPG123_DIR}include"          \
	-DMPG123_LIBRARIES="${MPG123_DIR}lib/libmpg123.a"    \
	-DSNDFILE_INCLUDE_DIR="${SNDFILE_DIR}include"        \
	-DSNDFILE_LIBRARY="${SNDFILE_DIR}lib/libsndfile.a"   \
	-DFLUIDSYNTH_INCLUDE_DIR="${FSYNTH_DIR}include"      \
	-DFLUIDSYNTH_LIBRARIES="${FSYNTH_LIBS}"              \
	-DLLVM_DIR="${DEPS_DIR}llvm/lib/cmake/llvm"          \
	"${ZDOOM_DIR}"
make -j2

# -----------------------------------------------------------------------------
# Create disk image
# -----------------------------------------------------------------------------

BUNDLE_PATH=${DIST_DIR}${ZDOOM_PROJECT}.app
INFO_PLIST_PATH=${BUNDLE_PATH}/Contents/Info.plist

mkdir "${DIST_DIR}"
cp -R ${ZDOOM_PROJECT_LOW}.app "${BUNDLE_PATH}"
cp -R "${ZDOOM_DIR}docs" "${DIST_DIR}Docs"
ln -s /Applications "${DIST_DIR}/Applications"

plutil -replace LSMinimumSystemVersion -string "${ZDOOM_OS_MIN_VER}" "${INFO_PLIST_PATH}"
plutil -replace CFBundleVersion -string "${ZDOOM_VERSION}" "${INFO_PLIST_PATH}"
plutil -replace CFBundleShortVersionString -string "${ZDOOM_VERSION}" "${INFO_PLIST_PATH}"
plutil -replace CFBundleLongVersionString -string "${ZDOOM_VERSION}" "${INFO_PLIST_PATH}"
plutil -replace CFBundleIdentifier -string "${ZDOOM_IDENTIFIER}" "${INFO_PLIST_PATH}"

DMG_NAME=${ZDOOM_PROJECT}-${ZDOOM_VERSION}
DMG_FILENAME=$(echo ${DMG_NAME}.dmg | tr '[:upper:]' '[:lower:]')
DMG_PATH=${BASE_DIR}${DMG_FILENAME}
TMP_DMG_PATH=${BASE_DIR}${ZDOOM_PROJECT}-tmp.dmg

hdiutil makehybrid -o "${TMP_DMG_PATH}" "${DIST_DIR}" -hfs -hfs-volume-name "${DMG_NAME}"
hdiutil convert -format UDBZ -imagekey bzip2-level=9 "${TMP_DMG_PATH}" -o "${DMG_PATH}"
rm "${TMP_DMG_PATH}"

if [ -n "$1" ]; then
	# create .tar.bz2 containing app bundle for "special" builds
	tar -c ${ZDOOM_PROJECT_LOW}.app | bzip2 -1 > "${BASE_DIR}${ZDOOM_PROJECT_LOW}.app.tar.bz2"
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
eval `python -c "import base64,sys,zlib;print('*'+base64.b16encode(zlib.compress(sys.argv[1])).lower())if'*'!=sys.argv[1][0]else zlib.decompress(base64.b16decode(sys.argv[1][1:],True))" "${DEPLOY_CONFIG}"`

cd "${SRC_ZDOOM_DIR}"
ZDOOM_REPO=$(git remote get-url origin)
ZDOOM_REPO=${ZDOOM_REPO/https:\/\/github.com\//}
ZDOOM_REPO=${ZDOOM_REPO/.git/}

# -----------------------------------------------------------------------------
# Update devbuilds Git repository
# -----------------------------------------------------------------------------

TMP_CHECKSUM=$(shasum -a 256 "${DMG_PATH}")
DMG_CHECKSUM=${TMP_CHECKSUM:0:64}

ZDOOM_DEVBUILDS=${ZDOOM_PROJECT_LOW}-macos-devbuilds
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

python -B ${SCRIPT_DIR}github_release.py \
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
	put "${DMG_PATH}"
	bye
EOF
