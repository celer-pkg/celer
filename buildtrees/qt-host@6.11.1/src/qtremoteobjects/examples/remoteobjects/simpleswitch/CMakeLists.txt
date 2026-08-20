# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qtremoteobjects_simpleswitch)
find_package(Qt6 REQUIRED COMPONENTS Core RemoteObjects)
qt_standard_project_setup()
add_subdirectory(directconnectclient)
add_subdirectory(directconnectserver)
