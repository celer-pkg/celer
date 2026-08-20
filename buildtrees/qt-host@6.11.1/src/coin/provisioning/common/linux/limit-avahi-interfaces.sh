#!/usr/bin/env bash
# Copyright (C) 2024 The Qt Company Ltd
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

set -ex

# Deny avahi access to the VM network.
# Avahi is only needed for local test services on the VM.

interface=$(ip -br a | grep 10.215 | awk '{print $1}')
sudo sed -i "s/#deny-interfaces=eth1/deny-interfaces=${interface}/g" /etc/avahi/avahi-daemon.conf
