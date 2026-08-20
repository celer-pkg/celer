# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

set(assets_animations
    universalRiggedHead_rotation_0.qad
    universalRiggedLCalf_rotation_0.qad
    universalRiggedLCollarbone_rotation_0.qad
    universalRiggedLForearm_rotation_0.qad
    universalRiggedLThigh_rotation_0.qad
    universalRiggedLUpperarm_rotation_0.qad
    universalRiggedNeck_rotation_0.qad
    universalRiggedRCalf_rotation_0.qad
    universalRiggedRCollarbone_rotation_0.qad
    universalRiggedRForearm_rotation_0.qad
    universalRiggedRibcage_rotation_0.qad
    universalRiggedRThigh_rotation_0.qad
    universalRiggedRUpperarm_rotation_0.qad
    universalRiggedSpine1_rotation_0.qad
    universalRiggedSpine2_rotation_0.qad
    universalRiggedSpine3_rotation_0.qad
)

qt_target_qml_sources(${TARGET_NAME}
    PREFIX "/qt/qml/ToyCustomizer/qml/animations/"
    RESOURCES ${assets_animations}
)

set(clean_animations "${assets_animations}")
list(TRANSFORM clean_animations PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${clean_animations}")
