# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(qml/MyModule)
add_subdirectory(qml2/MyModule2)
add_subdirectory(qml3/MyModule3)

qt_add_qml_module(QmllsBuildIni
    URI QmllsBuildIni
    QML_FILES
        Main.qml
    DEPENDENCIES
        TARGET MyModuleInQml
        TARGET MyModule3InQml3
    IMPORTS
        TARGET MyModule2InQml2
)
