set -o errexit

ZDOOM_PROJECT_LOW=$(echo ${ZDOOM_PROJECT} | tr '[:upper:]' '[:lower:]')

SRC_BASE_DIR=/Volumes/Storage/Work/
SRC_DEPS_DIR=${SRC_BASE_DIR}zdoom-macos-deps/
SRC_ZDOOM_DIR=${SRC_BASE_DIR}devbuilds/${ZDOOM_PROJECT_LOW}/

BASE_DIR=/Volumes/ramdisk/${ZDOOM_PROJECT_LOW}-devbuild/
DEPS_DIR=${BASE_DIR}deps/
ZDOOM_DIR=${BASE_DIR}${ZDOOM_PROJECT_LOW}/
BUILD_DIR=${BASE_DIR}build/
DIST_DIR=${BASE_DIR}dist/

cd "${SRC_DEPS_DIR}"
git pull

cd "${SRC_ZDOOM_DIR}"
git pull

mkdir "${BASE_DIR}"

cd "${BASE_DIR}"
git clone -s "${SRC_DEPS_DIR}" "${DEPS_DIR}"
git clone -s "${SRC_ZDOOM_DIR}" "${ZDOOM_DIR}"

cd "${ZDOOM_DIR}"

if [ -n "$1" ]; then
	git checkout "$1"
fi

ZDOOM_VERSION=`git describe --tags`

mkdir "${BUILD_DIR}"
cd "${BUILD_DIR}"

FMOD_DIR=${DEPS_DIR}fmodex/
OPENAL_DIR=${DEPS_DIR}openal/
MPG123_DIR=${DEPS_DIR}mpg123/
SNDFILE_DIR=${DEPS_DIR}sndfile/
OTHER_LIBS=-L${DEPS_DIR}ogg/lib\ -logg\ -L${DEPS_DIR}vorbis/lib\ -lvorbis\ -lvorbisenc\ -L${DEPS_DIR}flac/lib\ -lFLAC
FRAMEWORKS=-framework\ AudioUnit\ -framework\ AudioToolbox\ -framework\ CoreAudio\ -framework\ ForceFeedback
LINKER_FLAGS=${OTHER_LIBS}\ ${FRAMEWORKS}

/Applications/CMake.app/Contents/bin/cmake               \
	-DCMAKE_BUILD_TYPE="RelWithDebInfo"                  \
	-DCMAKE_OSX_DEPLOYMENT_TARGET="${ZDOOM_OS_MIN_VER}"  \
	-DCMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS}"           \
	-DOSX_COCOA_BACKEND=YES                              \
	-DDYN_OPENAL=NO                                      \
	-DFMOD_INCLUDE_DIR="${FMOD_DIR}inc"                  \
	-DFMOD_LIBRARY="${FMOD_DIR}lib/libfmodex.dylib"      \
	-DOPENAL_INCLUDE_DIR="${OPENAL_DIR}include"          \
	-DOPENAL_LIBRARY="${OPENAL_DIR}lib/libopenal.a"      \
	-DMPG123_INCLUDE_DIR="${MPG123_DIR}include"          \
	-DMPG123_LIBRARIES="${MPG123_DIR}lib/libmpg123.a"    \
	-DSNDFILE_INCLUDE_DIR="${SNDFILE_DIR}include"        \
	-DSNDFILE_LIBRARY="${SNDFILE_DIR}lib/libsndfile.a"   \
	-DLLVM_DIR="${DEPS_DIR}llvm/lib/cmake/llvm"          \
	"${ZDOOM_DIR}"
make -j4

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

DMG_NAME=${ZDOOM_PROJECT}-${ZDOOM_VERSION}
DMG_PATH=${BASE_DIR}`echo ${DMG_NAME} | tr '[:upper:]' '[:lower:]'`.dmg

hdiutil create -srcfolder "${DIST_DIR}" -volname "${DMG_NAME}" \
	-format UDBZ -fs HFS+ -fsargs "-c c=64,a=16,e=16" "${DMG_PATH}"
