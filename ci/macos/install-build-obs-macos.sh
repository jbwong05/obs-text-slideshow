#!/bin/sh

OSTYPE=$(uname)

if [ "${OSTYPE}" != "Darwin" ]; then
    echo "[Error] macOS obs-studio build script can be run on Darwin-type OS only."
    exit 1
fi

HAS_CMAKE=$(type cmake 2>/dev/null)
HAS_GIT=$(type git 2>/dev/null)

if [ "${HAS_CMAKE}" = "" ]; then
    echo "[Error] CMake not installed - please run 'install-dependencies-macos.sh' first."
    exit 1
fi

if [ "${HAS_GIT}" = "" ]; then
    echo "[Error] Git not installed - please install Xcode developer tools or via Homebrew."
    exit 1
fi

echo "=> Downloading and unpacking OBS dependencies"
wget --quiet --retry-connrefused --waitretry=1 https://github.com/obsproject/obs-deps/releases/download/2021-03-25/macos-deps-x86_64-2021-03-25.tar.gz
tar -xf ./macos-deps-x86_64-2021-03-25.tar.gz -C /tmp
wget --quiet --retry-connrefused --waitretry=1 https://github.com/obsproject/obs-deps/releases/download/2021-03-25/macos-qt-5.15.2-x86_64-2021-03-25.tar.gz
tar -xf ./macos-qt-5.15.2-x86_64-2021-03-25.tar.gz -C /tmp
xattr -r -d com.apple.quarantine /tmp/obsdeps

# Build obs-studio
cd ..
echo "=> Cloning obs-studio from GitHub.."
git clone https://github.com/obsproject/obs-studio
cd obs-studio
OBSLatestTag=$(git describe --tags --abbrev=0)
git checkout $OBSLatestTag
mkdir build && cd build
echo "=> Building obs-studio.."
cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 -DDISABLE_PLUGINS=true -DENABLE_SCRIPTING=0 -DDepsPath="/tmp/obsdeps" -DQTDIR="/tmp/obsdeps" ..
make -j4
