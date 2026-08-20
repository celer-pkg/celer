# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtGraphs/designer/default")
qt_copy_or_install(
    FILES
        Bars3D.qml
        Scatter3D.qml
        Surface3D.qml
    DESTINATION "${destination}"
)
