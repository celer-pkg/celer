# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

file(GLOB_RECURSE designer_image_files
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "images/*.png"
)
qt_path_join(images_destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtGraphs/designer/images")
qt_copy_or_install(
    FILES
        ${designer_image_files}
    DESTINATION "${images_destination}"
)

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtGraphs/designer")
qt_copy_or_install(
    FILES
        Bars3DSpecifics.qml
        GraphsCameraSection.qml
        GraphsSection.qml
        Scatter3DSpecifics.qml
        Surface3DSpecifics.qml
        qtgraphs.metainfo
    DESTINATION "${destination}"
)

add_subdirectory(default)
