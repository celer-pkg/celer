# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtQuick3D/designer/source")
qt_copy_or_install(
    FILES
        custom_material_default_shader.frag
        effect_default_shader.frag
        effect_template.qml
        view3D_template.qml
    DESTINATION "${destination}"
)
