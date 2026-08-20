# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(barstest
    GUI
    SOURCES
        chart.cpp chart.h
        sliderwrapper.cpp sliderwrapper.h
        buttonwrapper.cpp buttonwrapper.h
        custominputhandler.cpp custominputhandler.h
        main.cpp
    NO_PCH_SOURCES
        chart.cpp # undef QT_NO_FOREACH
    )
target_link_libraries(barstest PUBLIC
    Qt::Gui
    Qt::Widgets
    Qt::DataVisualization
    )

set(barstest_resource_files
    "shuttle.obj"
    "shuttle.png"
    )

qt_internal_add_resource(barstest "barstest"
    PREFIX
    "/"
    FILES
    ${barstest_resource_files}
    )
