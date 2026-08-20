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
    url_cached="http://ci-files01-hki.ci.qt.io/input/pulseaudio-16-bugfixes/noble_amd64/${deb}"

    DownloadURL $url_cached $url $checksum $deb
}

DownloadDeb libpulse0_16.1+dfsg1-2ubuntu10timesmootherfix~noble_amd64.deb                   65458740897d0e939d07cf1c892060c381e68441
DownloadDeb libpulse-dev_16.1+dfsg1-2ubuntu10timesmootherfix~noble_amd64.deb                a998bd30f4ca13ad9e66a2a8fb4b9bfe9aade8e2
DownloadDeb libpulse-mainloop-glib0_16.1+dfsg1-2ubuntu10timesmootherfix~noble_amd64.deb     326baf3c11a4f490dac6dad2d4e56c0821b31b80

sudo dpkg -i \
    libpulse0_16.1+dfsg1-2ubuntu10timesmootherfix~noble_amd64.deb \
    libpulse-dev_16.1+dfsg1-2ubuntu10timesmootherfix~noble_amd64.deb \
    libpulse-mainloop-glib0_16.1+dfsg1-2ubuntu10timesmootherfix~noble_amd64.deb
