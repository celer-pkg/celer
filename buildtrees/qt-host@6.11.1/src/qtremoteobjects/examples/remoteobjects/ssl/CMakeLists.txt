# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qtremoteobjects_ssl)
find_package(Qt6 REQUIRED COMPONENTS Core RemoteObjects)
qt_standard_project_setup()
add_subdirectory(sslserver)
add_subdirectory(sslcppclient)
