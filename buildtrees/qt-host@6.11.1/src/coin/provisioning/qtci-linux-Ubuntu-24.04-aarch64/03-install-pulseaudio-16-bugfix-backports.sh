#!/usr/bin/env bash
# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

# backport of:
# https://gitlab.freedesktop.org/pulseaudio/pulseaudio/-/merge_requests/745
# https://gitlab.freedesktop.org/pulseaudio/pulseaudio/-/merge_requests/764
# compare: https://doc-snapshots.qt.io/qt6-6.8/qtmultimedia-gstreamer.html#limitations-and-known-issues

# shellcheck source=../common/unix/DownloadURL.sh
source "${BASH_SOURCE%/*}/../common/unix/DownloadURL.sh"

set -ex

DownloadDeb () {
    deb="$1"
    checksum="$2"

    url="https://launchpad.net/~tim-klingt/+archive/ubuntu/pulseaudio-16-bugfixes/+files/${deb}"
    url_cached="http://ci-files01-hki.ci.qt.io/input/pulseaudio-16-bugfixes/noble_arm64/${deb}"

    DownloadURL $url_cached $url $checksum $deb
}

DownloadDeb libpulse0_16.1+dfsg1-2ubuntu10timesmootherfix~noble_arm64.deb                   4fa467972542a3851aad892833dc0149efe5c6f3
DownloadDeb libpulse-dev_16.1+dfsg1-2ubuntu10timesmootherfix~noble_arm64.deb                20045425b0522ec39adc0231d6727146ce910dd4
DownloadDeb libpulse-mainloop-glib0_16.1+dfsg1-2ubuntu10timesmootherfix~noble_arm64.deb     cd94fd91e6f5a4b67ccd148c50e93a9dc9a59b33

sudo dpkg -i \
    libpulse0_16.1+dfsg1-2ubuntu10timesmootherfix~noble_arm64.deb \
    libpulse-dev_16.1+dfsg1-2ubuntu10timesmootherfix~noble_arm64.deb \
    libpulse-mainloop-glib0_16.1+dfsg1-2ubuntu10timesmootherfix~noble_arm64.deb
