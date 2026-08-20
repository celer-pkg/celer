#!/usr/bin/env bash
# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

set -ex

# shellcheck source=../common/macos/install-simulator-runtime.sh
source "${BASH_SOURCE%/*}/../common/macos/install-simulator-runtime.sh"
version="17.4"
packageName=iOS_"$version"_Simulator_Runtime.dmg
url="http://ci-files01-hki.ci.qt.io/input/mac/$packageName"
sha1="7b930c544403806f3a4f99bb48e80f4ef1533f50"

InstallSimulatorRuntime $url $url $sha1 $packageName $version
