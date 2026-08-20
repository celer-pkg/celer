# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(scattertest
    GUI
    SOURCES
        main.cpp
        scatterchart.cpp scatterchart.h
    NO_PCH_SOURCES
        scatterchart.cpp # undef QT_NO_FOREACH
    )
target_link_libraries(scattertest PUBLIC
    Qt::Gui
    Qt::Widgets
    Qt::DataVisualization
    )
