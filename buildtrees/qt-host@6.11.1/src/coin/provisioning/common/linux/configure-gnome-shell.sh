#!/usr/bin/env bash
# Copyright (C) 2024 The Qt Company Ltd
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

# This script modifies GNOME based Linux configurations

set -ex

# Desktop
echo "Disable blank screen power saving (timeout 0 = never)"
gsettings set org.gnome.desktop.session idle-delay 0
echo "Disable Automatic screen lock when screensaver goes active"
gsettings set org.gnome.desktop.screensaver lock-enabled false
echo "Disable window animations."
gsettings set org.gnome.desktop.interface enable-animations false
echo "Disable hot corner feature"
gsettings set org.gnome.desktop.interface enable-hot-corners false
