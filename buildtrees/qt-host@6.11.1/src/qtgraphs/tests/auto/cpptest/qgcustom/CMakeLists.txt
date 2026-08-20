# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qgcustom
    SOURCES
        tst_custom.cpp
    LIBRARIES
        Qt::Gui
        Qt::Graphs
)

qt_internal_add_resource(tst_qgcustom "qgcustom"
    PREFIX
        "/"
    FILES
        "customitem.mesh"
)
