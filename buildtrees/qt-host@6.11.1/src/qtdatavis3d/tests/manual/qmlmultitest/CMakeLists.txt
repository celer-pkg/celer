# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(qmlmultitest
    GUI
    SOURCES
        main.cpp
    )
target_link_libraries(qmlmultitest PUBLIC
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::DataVisualization
    )

set(qmlmultitest_resource_files
    "qml/qmlmultitest/Data.qml"
    "qml/qmlmultitest/main.qml"
    )

qt_internal_add_resource(qmlmultitest "qmlmultitest"
    PREFIX
    "/"
    FILES
    ${qmlmultitest_resource_files}
    )
