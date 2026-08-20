#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cd coin/openapi-generator-cli || exit
# The dir is set in provisioning script..
cp /opt/qt-openapi/openapi-generator-cli.jar .
version=$(java -jar ./openapi-generator-cli.jar version)
sed -i "s/CLI_VERSION/$version/g" debian/changelog
dpkg-buildpackage -us -uc
cd ..
mkdir -p /home/qt/work/output/debian_packages/
# To build qtopenapi
cp ./*.deb /home/qt/work/debian_packages/
# For artifacts
mv ./*.deb /home/qt/work/output/debian_packages/
