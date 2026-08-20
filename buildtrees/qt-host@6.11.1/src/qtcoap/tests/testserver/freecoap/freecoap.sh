#!/usr/bin/env bash
# Copyright (C) 2019 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

set -ex

(cd /root/src/FreeCoAP/sample/time_server && ./time_server 0.0.0.0 5685 &)
