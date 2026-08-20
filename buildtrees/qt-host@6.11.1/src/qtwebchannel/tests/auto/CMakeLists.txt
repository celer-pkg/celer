# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(cmake)
add_subdirectory(webchannel)
if(TARGET Qt::Quick)
    add_subdirectory(qml)
endif()
