#Copyright (C) 2023 The Qt Company Ltd.
#SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_add_library(todolist_content STATIC)
qt6_add_qml_module(todolist_content
    URI "content"
    DEPENDENCIES
        TARGET CustomControls
    QML_FILES
        App.qml
        CalendarView.qml
        CalendarViewForm.ui.qml
        Clock.qml
        ClockForm.ui.qml
        ClockView.qml
        ClockViewForm.ui.qml
        FontSizeSettings.qml
        FontSizeSettingsForm.ui.qml
        HomePage.qml
        HomePageForm.ui.qml
        MaxTasksSettings.qml
        MaxTasksSettingsForm.ui.qml
        NavBar.qml
        NavBarForm.ui.qml
        NewTask.qml
        NewTaskForm.ui.qml
        OtherSettings.qml
        OtherSettingsForm.ui.qml
        QuickTaskField.qml
        QuickTaskFieldForm.ui.qml
        SettingsView.qml
        SettingsViewForm.ui.qml
        StyleSettings.qml
        StyleSettingsForm.ui.qml
        TasksList.qml
        TasksListForm.ui.qml
        TasksListsView.qml
        TasksListsViewForm.ui.qml
    RESOURCES
        images/Add_Icon.svg
        images/Before_Icon.svg
        images/Calendar_Icon.svg
        images/Check_Icon.svg
        images/Clock.svg
        images/Clock_Dark.svg
        images/Clock_Hand.svg
        images/Clock_Icon.svg
        images/Enter_Icon.svg
        images/Font_Size_Icon.svg
        images/HideTasks_Icon.svg
        images/LeftArrow_Icon.svg
        images/LeftArrow_Icon_Dark.svg
        images/Minutes_Clock.svg
        images/Minutes_Clock_Dark.svg
        images/Next_Icon.svg
        images/Qt_Icon.svg
        images/Random_Task_Icon.svg
        images/Remove_Done_Icon.svg
        images/Right_Arrow_Icon.svg
        images/Right_Arrow_Icon_Dark.svg
        images/Settings_Icon.svg
        images/ShowTasks_Icon.svg
        images/Style_Icon.svg
        images/Tasks_Icon.svg
        images/Theme_Icon.svg
)
