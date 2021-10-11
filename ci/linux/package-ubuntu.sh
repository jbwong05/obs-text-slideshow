#!/bin/bash

set -e

script_dir=$(dirname "$0")
source "$script_dir/../ci_includes.generated.sh"

export GIT_HASH=$(git rev-parse --short HEAD)

cd ./build

PAGER="cat" sudo checkinstall -y --type=debian --fstrans=no --nodoc \
	--backup=no --deldoc=yes --install=no \
	--pkgname="$PLUGIN_NAME" --pkgversion="$PLUGIN_VERSION" \
	--pkglicense="GPLv2.0" --maintainer="$LINUX_MAINTAINER_EMAIL" \
	--pkggroup="video" \
	--requires="obs-studio \(\>= 27.0.0\), libqt5core5a \(\>= 5.14\), libqt5widgets5 \(\>= 5.14\)" \
	--pakdir="../package"

sudo chmod ao+r ../package/*
