# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_examples_build_begin(EXTERNAL_BUILD)

if (TARGET Qt6::Widgets)
    add_subdirectory(svg)
endif()

qt_examples_build_end()
