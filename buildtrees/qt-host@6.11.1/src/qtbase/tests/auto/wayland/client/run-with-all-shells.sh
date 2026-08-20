#!/usr/bin/env bash
# Copyright (C) 2018 The Qt Company Ltd
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

set -ex
$@
env QT_WAYLAND_SHELL_INTEGRATION=wl-shell $@
env QT_WAYLAND_SHELL_INTEGRATION=ivi-shell $@
