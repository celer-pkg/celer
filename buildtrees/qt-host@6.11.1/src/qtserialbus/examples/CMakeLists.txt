# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_examples_build_begin(EXTERNAL_BUILD)

if(NOT TARGET Qt::Widgets)
    return()
endif()
if(NOT TARGET Qt::SerialBus)
    return()
endif()
add_subdirectory(serialbus)

qt_examples_build_end()
