#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

DAYS_VALID=36500
SUBJECT="/C=DE/ST=Berlin/L=Berlin/O=The Qt Company GmbH/OU=R&D/CN=localhost/emailAddress=dennis.oberst@qt.io"

# Generate Root CA
openssl genrsa -out root.key 2048
openssl req -x509 -new -nodes -key root.key -sha256 -days $DAYS_VALID -out root.crt -subj "$SUBJECT"

# Generate Server Certificate
openssl genrsa -out localhost.key 2048
openssl req -new -key localhost.key -out localhost.csr -subj "$SUBJECT"
openssl x509 -req -in localhost.csr -CA root.crt -CAkey root.key -CAcreateserial -out localhost.crt -days $DAYS_VALID -sha256

# Cleanup
rm localhost.csr root.srl root.key
