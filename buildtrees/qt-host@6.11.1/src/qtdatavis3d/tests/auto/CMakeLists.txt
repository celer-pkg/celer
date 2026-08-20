# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT ANDROID)
    add_subdirectory(cpptest)
endif()
if(TARGET Qt::Quick AND NOT boot2qt)
    add_subdirectory(qmltest)
endif()
