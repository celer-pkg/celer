#!/usr/bin/env bash
# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
# shellcheck source=../unix/DownloadURL.sh
source "${BASH_SOURCE%/*}/../unix/DownloadURL.sh"
set -ex

function InstallSimulatorRuntime {
    url=$1
    url_alt=$2
    expectedSha1=$3
    packageName=$4
    version=$5

    DownloadURL "$url" "$url_alt" "$expectedSha1" "/tmp/$packageName"
    echo "Installing"
    # macOS 14 / Xcode 15 has a different install command
    if [[ $OSTYPE == "darwin23" ]]; then
        xcrun simctl runtime add "/tmp/$packageName"
    else
        xcodebuild -importPlatform "/tmp/$packageName"
    fi

    echo "Simulator Runtime = $version" >> ~/versions.txt
}
