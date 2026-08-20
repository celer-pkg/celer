# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

set_source_files_properties(Config.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

qt_add_library(LightningViewerStyle STATIC)

qt_add_qml_module(LightningViewerStyle
    URI LightningViewerStyle
    QML_FILES
        Button.qml
        Config.qml
        RoundButton.qml
        StyleImage.qml
    RESOURCES
        dark/images/roundbutton-background-checked-disabled@2x.png
        dark/images/button-background-checked-hovered.png
        dark/images/roundbutton-background-checked-disabled.png
        dark/images/flatbutton-background.png
        dark/images/flatbutton-background-disabled.png
        dark/images/flatbutton-background-checked-pressed.png
        dark/images/button-background.png
        dark/images/flatbutton-background-hovered.png
        dark/images/button-background-checked-hovered@2x.png
        dark/images/roundbutton-background-checked-hovered.png
        dark/images/button-background-checked.png
        dark/images/flatbutton-background-checked-hovered@2x.png
        dark/images/roundbutton-background-disabled.png
        dark/images/flatbutton-background-disabled@2x.png
        dark/images/roundbutton-background@2x.png
        dark/images/flatbutton-background-checked-pressed@2x.png
        dark/images/button-background@2x.png
        dark/images/button-background-hovered@2x.png
        dark/images/roundbutton-background-checked-pressed.png
        dark/images/roundbutton-background-hovered.png
        dark/images/roundbutton-background-checked.png
        dark/images/flatbutton-background-checked@2x.png
        dark/images/roundbutton-background.png
        dark/images/button-background-pressed@2x.png
        dark/images/roundbutton-background-disabled@2x.png
        dark/images/flatbutton-background-checked.png
        dark/images/button-background-disabled.png
        dark/images/roundbutton-background-pressed@2x.png
        dark/images/flatbutton-background-checked-disabled.png
        dark/images/roundbutton-background-checked@2x.png
        dark/images/roundbutton-background-checked-hovered@2x.png
        dark/images/button-background-hovered.png
        dark/images/flatbutton-background-checked-disabled@2x.png
        dark/images/button-background-checked-disabled@2x.png
        dark/images/button-background-checked-disabled.png
        dark/images/button-background-checked-pressed@2x.png
        dark/images/flatbutton-background@2x.png
        dark/images/button-background-checked@2x.png
        dark/images/button-background-pressed.png
        dark/images/button-background-checked-pressed.png
        dark/images/button-background-disabled@2x.png
        dark/images/flatbutton-background-pressed.png
        dark/images/flatbutton-background-checked-hovered.png
        dark/images/flatbutton-background-pressed@2x.png
        dark/images/roundbutton-background-pressed.png
        dark/images/roundbutton-background-checked-pressed@2x.png
        dark/images/flatbutton-background-hovered@2x.png
        dark/images/roundbutton-background-hovered@2x.png
        light/images/roundbutton-background-checked-disabled@2x.png
        light/images/button-background-checked-hovered.png
        light/images/roundbutton-background-checked-disabled.png
        light/images/flatbutton-background.png
        light/images/flatbutton-background-disabled.png
        light/images/flatbutton-background-checked-pressed.png
        light/images/button-background.png
        light/images/flatbutton-background-hovered.png
        light/images/button-background-checked-hovered@2x.png
        light/images/roundbutton-background-checked-hovered.png
        light/images/button-background-checked.png
        light/images/flatbutton-background-checked-hovered@2x.png
        light/images/roundbutton-background-disabled.png
        light/images/flatbutton-background-disabled@2x.png
        light/images/roundbutton-background@2x.png
        light/images/flatbutton-background-checked-pressed@2x.png
        light/images/button-background@2x.png
        light/images/button-background-hovered@2x.png
        light/images/roundbutton-background-checked-pressed.png
        light/images/roundbutton-background-hovered.png
        light/images/roundbutton-background-checked.png
        light/images/flatbutton-background-checked@2x.png
        light/images/roundbutton-background.png
        light/images/button-background-pressed@2x.png
        light/images/roundbutton-background-disabled@2x.png
        light/images/flatbutton-background-checked.png
        light/images/button-background-disabled.png
        light/images/roundbutton-background-pressed@2x.png
        light/images/flatbutton-background-checked-disabled.png
        light/images/roundbutton-background-checked@2x.png
        light/images/roundbutton-background-checked-hovered@2x.png
        light/images/button-background-hovered.png
        light/images/flatbutton-background-checked-disabled@2x.png
        light/images/button-background-checked-disabled@2x.png
        light/images/button-background-checked-disabled.png
        light/images/button-background-checked-pressed@2x.png
        light/images/flatbutton-background@2x.png
        light/images/button-background-checked@2x.png
        light/images/button-background-pressed.png
        light/images/button-background-checked-pressed.png
        light/images/button-background-disabled@2x.png
        light/images/flatbutton-background-pressed.png
        light/images/flatbutton-background-checked-hovered.png
        light/images/flatbutton-background-pressed@2x.png
        light/images/roundbutton-background-pressed.png
        light/images/roundbutton-background-checked-pressed@2x.png
        light/images/flatbutton-background-hovered@2x.png
        light/images/roundbutton-background-hovered@2x.png
)

target_link_libraries(LightningViewerStyle PRIVATE
    Qt6::Quick
)
