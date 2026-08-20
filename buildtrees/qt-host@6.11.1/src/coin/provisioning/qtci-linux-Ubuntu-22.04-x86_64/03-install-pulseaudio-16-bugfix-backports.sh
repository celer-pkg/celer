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
    url_cached="http://ci-files01-hki.ci.qt.io/input/pulseaudio-16-bugfixes/jammy_amd64/${deb}"

    DownloadURL $url_cached $url $checksum $deb
}

DownloadDeb libpulse0_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb                   5607f464fc73d09e2067a12b7e20d3b175d17e94
DownloadDeb libpulse-dev_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb                674de26bb5a5148479d0c9e81c22462e2ffefa42
DownloadDeb libpulsedsp_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb                 0e06bea12c267cdeade4dce48ceba709f3a57036
DownloadDeb libpulse-mainloop-glib0_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb     6fbb84697ac3e46664f3a9aff63bb8c04666c0a2
DownloadDeb pulseaudio_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb                  0988e8357a7024b8bf55c6fe0f51c0c0a72813e7
DownloadDeb pulseaudio-utils_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb            667a6a570e9bf03317de6e548443ea058ad5195d
DownloadDeb pulseaudio-module-bluetooth_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb fd22382092cc45b7d717895f342fa5f5b6bc22e0

sudo dpkg -i \
    libpulse0_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb \
    libpulse-dev_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb \
    libpulsedsp_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb \
    libpulse-mainloop-glib0_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb \
    pulseaudio_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb \
    pulseaudio-module-bluetooth_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb \
    pulseaudio-utils_15.99.1+dfsg1-1ubuntu2.2timesmootherfix~jammy_amd64.deb
