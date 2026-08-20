# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_examples_build_begin(EXTERNAL_BUILD)

if(QT_FEATURE_graphs_3d)
    add_subdirectory(graphs/3d)
endif()

if(QT_FEATURE_graphs_2d)
    add_subdirectory(graphs/2d)
endif()

if(QT_FEATURE_graphs_2d AND QT_FEATURE_graphs_3d)
    add_subdirectory(graphs/common)
endif()

qt_examples_build_end()
