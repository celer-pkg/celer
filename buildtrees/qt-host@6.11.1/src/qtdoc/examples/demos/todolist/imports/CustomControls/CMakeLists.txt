#Copyright (C) 2023 The Qt Company Ltd.
#SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_add_library(CustomControls STATIC)
qt6_add_qml_module(CustomControls
    URI "CustomControls"
    DEPENDENCIES
        TARGET ToDoList
    QML_FILES
        AmPmButton.qml
        CustomButton.qml
        CustomLabel.qml
        CustomTextArea.qml
        CustomTextField.qml
        SwipeDelegateContent.qml
        TasksListDelegate.qml
        TimeInput.qml
        TimeTextField.qml
    RESOURCES
        images/Drag_Icon.svg
        images/Drag_Icon_Dark.svg
        images/Red_Trash.svg
        images/Star_Icon.svg
        images/White_Star.svg
        images/White_Trash.svg
)
