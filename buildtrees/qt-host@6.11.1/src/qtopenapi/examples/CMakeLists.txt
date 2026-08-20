# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_examples_build_begin(EXTERNAL_BUILD)

if(TARGET Qt::OpenApi)
    add_subdirectory(openapi)
endif()

qt_examples_build_end()
