#!/bin/sh
# Copyright (C) 2021 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generate the CA key
openssl genrsa -out rootCA.key 2048
# Generate the CA cert
openssl req -x509 -key rootCA.key -out rootCA.pem -sha256 -nodes -subj "/CN=QtRO CA" -days 836

# genFiles stem [extra args to signing]
genFiles () {
    stem=$1
    shift
    # Generate key
    openssl genrsa -out $stem.key 2048
    # Generate certificate-signing request
    openssl req -new -key $stem.key -out $stem.csr -subj "/CN=127.0.0.1"
    # Generate and sign the certificate
    # Apple requires that TLS certificates have a validity period of at most 825 days:
    # https://support.apple.com/en-us/103769
    openssl x509 -req -in $stem.csr -out $stem.crt \
                 -CA rootCA.pem -CAkey rootCA.key -CAcreateserial -days 825 -sha256 "$@"
    # Delete the signing request, no longer needed
    rm $stem.csr
}
genFiles server -extfile server-req.ext
genFiles client

dest1="../../../../examples/remoteobjects/ssl/sslserver/cert/"
dest2="../../../../examples/remoteobjects/websockets/common/cert/"

cp -f "client.crt" $dest1
cp -f "client.crt" $dest2
cp -f "client.key" $dest1
cp -f "client.key" $dest2

cp -f "server.crt" $dest1
cp -f "server.crt" $dest2
cp -f "server.key" $dest1
cp -f "server.key" $dest2

cp -f "rootCA.pem" $dest1
cp -f "rootCA.key" $dest1
cp -f "rootCA.srl" $dest1
cp -f "rootCA.pem" $dest2
cp -f "rootCA.key" $dest2
cp -f "rootCA.srl" $dest2
