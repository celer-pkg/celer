# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(cmake)
add_subdirectory(websockets)
if(TARGET Qt::Quick AND TARGET Qt::QuickTest)
    add_subdirectory(qml)
endif()
