# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtQuick3D/Effects/designer/images")
qt_copy_or_install(
    FILES
        effect.png
        effect16.png
        effect@2x.png
    DESTINATION "${destination}"
)
