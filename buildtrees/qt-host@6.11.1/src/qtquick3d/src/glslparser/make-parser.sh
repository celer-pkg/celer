#!/bin/sh
# Copyright (C) 2018 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

me=$(dirname $0)
qlalr --qt --no-debug $me/glsl.g

