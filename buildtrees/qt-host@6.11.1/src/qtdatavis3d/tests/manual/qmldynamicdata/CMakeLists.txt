# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(qmldynamicdata
    GUI
    SOURCES
        main.cpp
    )
target_link_libraries(qmldynamicdata PUBLIC
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::DataVisualization
    )

set(qmldynamicdata_resource_files
    "qml/qmldynamicdata/main.qml"
    )

qt_internal_add_resource(qmldynamicdata "qmldynamicdata"
    PREFIX
    "/"
    FILES
    ${qmldynamicdata_resource_files}
    )
