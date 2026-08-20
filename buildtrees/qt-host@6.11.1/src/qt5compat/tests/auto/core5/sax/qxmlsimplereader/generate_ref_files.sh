#!/bin/bash
# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

for k in `find xmldocs -name \*.xml`; do echo $k...; ./parser/parser $k; done
