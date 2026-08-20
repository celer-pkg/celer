# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if (NOT ANDROID)
    add_subdirectory(directconnectdynamicclient)
    add_subdirectory(registryconnectedclient)
    add_subdirectory(registryconnectedserver)
endif()
