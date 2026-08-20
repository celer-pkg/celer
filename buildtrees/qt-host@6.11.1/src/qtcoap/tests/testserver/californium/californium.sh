#!/usr/bin/env bash
# Copyright (C) 2019 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

set -ex

java -jar /root/src/californium/demo-apps/run/cf-plugtest-server-3.8.0.jar &
