#Copyright (C) 2023 The Qt Company Ltd.
#SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_add_library(CustomStyle STATIC)
qt6_add_qml_module(CustomStyle
    URI "CustomStyle"
    DEPENDENCIES
        TARGET ToDoList
    QML_FILES
        Button.qml
        CheckBox.qml
        ComboBox.qml
        Dialog.qml
        Page.qml
        Pane.qml
        ItemDelegate.qml
        RadioDelegate.qml
        SwitchDelegate.qml
        TextArea.qml
        TextField.qml
        ToolBar.qml
        ToolButton.qml
        ToolTip.qml
    RESOURCES
        images/Expand_Icon.svg
)
