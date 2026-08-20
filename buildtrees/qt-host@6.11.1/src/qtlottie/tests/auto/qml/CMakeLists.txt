# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_lottie_qml
    GUI
    QMLTEST
    SOURCES
        tst_qml.cpp
    LIBRARIES
        Qt::Quick
    TESTDATA
        data/tst_main.qml
)

qt_internal_add_resource(tst_lottie_qml "qml"
    PREFIX
        "/"
    FILES
        data/rec_pos_col_opa.json
)
