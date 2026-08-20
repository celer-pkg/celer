# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_qml_module(appImportPathOrderStudy
    URI ImportPathOrderStudy
    QML_FILES
        Main.qml

    DEPENDENCIES
        ##!! Swapping the order if the 2 lines below will change which module is seen by the tools
        TARGET MyLib_Real
        TARGET MyLib_Mock
)
