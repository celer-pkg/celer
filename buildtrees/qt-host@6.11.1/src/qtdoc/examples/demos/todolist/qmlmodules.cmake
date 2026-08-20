#Copyright (C) 2023 The Qt Company Ltd.
#SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

add_subdirectory(imports)
add_subdirectory(content)

qt6_add_qml_module(ToDoListApp
    URI "Main"
    VERSION 1.0
    DEPENDENCIES
        TARGET ToDoList
        TARGET todolist_content
    QML_FILES Main.qml
)

target_link_libraries(ToDoListApp PRIVATE
    todolist_contentplugin
    CustomControlsplugin
    CustomStyleplugin
    ToDoListplugin
)
