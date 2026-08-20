# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(qsensor)
add_subdirectory(cmake)
if(TARGET Qt::Quick)
    add_subdirectory(qml)
endif()
