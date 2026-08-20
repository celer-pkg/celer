#!/usr/bin/sh
# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

openssl req -x509 -nodes -newkey rsa:4096 -keyout localhost.key -out localhost.cert -sha512 -days 365 -subj "/C=NO/ST=Oslo/L=Oslo/O=The Qt Project/OU=WebSockets/CN=localhost"
