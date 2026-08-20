#!/usr/bin/env bash
# Copyright (C) 2020 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

# This script installs QEMU Guest Agent

set -ex
sudo zypper addrepo --no-gpgcheck https://download.opensuse.org/repositories/Virtualization/15.5/Virtualization.repo
sudo zypper ref -f
sudo zypper -nq install qemu-guest-agent
