#!/usr/bin/env bash
# Copyright (C) 2017 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

# This script installs CMake

# CMake is needed for autotests that verify that Qt can be built with CMake

# shellcheck source=./InstallAppFromCompressedFileFromURL.sh
source "${BASH_SOURCE%/*}/InstallAppFromCompressedFileFromURL.sh"
# shellcheck source=../unix/SetEnvVar.sh
source "${BASH_SOURCE%/*}/../unix/SetEnvVar.sh"

majorminorversion="3.30.5"
version="3.30.5"
PrimaryUrl="http://ci-files01-hki.ci.qt.io/input/cmake/cmake-$version-macos-universal.tar.gz"
AltUrl="https://cmake.org/files/v$majorminorversion/cmake-$version-macos-universal.tar.gz"
SHA1="c81c2f7482a901c4a32a81034920c23c22233a0f"
appPrefix="cmake-$version-macos-universal"

InstallAppFromCompressedFileFromURL "$PrimaryUrl" "$AltUrl" "$SHA1" "$appPrefix"

SetEnvVar "PATH" "/Applications/CMake.app/Contents/bin:\$PATH"

echo "CMake = $version" >> ~/versions.txt
