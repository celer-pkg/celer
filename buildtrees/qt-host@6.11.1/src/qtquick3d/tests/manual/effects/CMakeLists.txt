# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_effects
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
)

qt_add_qml_module(manual_test_effects
    URI Example
    VERSION 1.0
    QML_FILES
        CustomEffect.qml
        EffectBox.qml
        EffectColor.qml
        EffectSlider.qml
        FileEffect.qml
        SettingsGroup.qml
        main.qml
    RESOURCES
        cork.jpg
        texture.png
        default.vert
        default.frag
    NO_RESOURCE_TARGET_PATH
)
