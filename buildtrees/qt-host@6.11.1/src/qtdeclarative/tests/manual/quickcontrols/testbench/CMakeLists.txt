# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(testbench LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core CorePrivate Gui Qml Quick QuickControls2 QuickControls2Private)

qt_standard_project_setup(REQUIRES 6.8)

qt_internal_add_manual_test(testbench
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::QuickControls2
        Qt::QuickControls2Private
)

target_compile_definitions(testbench PRIVATE
    APP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

qt_add_qml_module(testbench
    URI Testbench
    SOURCES
        assetfixer.cpp
        assetfixer.h
        clipboard.cpp
        clipboard.h
        directoryvalidator.cpp
        directoryvalidator.h
        utils.cpp
        utils.h
    QML_FILES
        "+Imagine/ContentPane.qml"
        "+Imagine/MainWindow.qml"
        "+Imagine/TestbenchDialog.qml"
        "+Imagine/TestbenchMenu.qml"
        "+Imagine/TestbenchToolBar.qml"
        "ColorEditor.qml"
        "ContentPane.qml"
        "ControlContainer.qml"
        "ExampleContainer.qml"
        "Main.qml"
        "MainWindow.qml"
        "SettingsDialog.qml"
        "TestbenchDialog.qml"
        "TestbenchMenu.qml"
        "TestbenchToolBar.qml"
        "controls/BusyIndicator.qml"
        "controls/Button.qml"
        "controls/CheckBox.qml"
        "controls/CheckDelegate.qml"
        "controls/ComboBox.qml"
        "controls/DelayButton.qml"
        "controls/Dial.qml"
        "controls/Dialog.qml"
        "controls/DoubleSpinBox.qml"
        "controls/Frame.qml"
        "controls/GroupBox.qml"
        "controls/ItemDelegate.qml"
        "controls/Label.qml"
        "controls/Menu.qml"
        "controls/MenuBar.qml"
        "controls/Page.qml"
        "controls/PageIndicator.qml"
        "controls/Pane.qml"
        "controls/ProgressBar.qml"
        "controls/RadioButton.qml"
        "controls/RadioDelegate.qml"
        "controls/RangeSlider.qml"
        "controls/RoundButton.qml"
        "controls/ScrollBar.qml"
        "controls/ScrollIndicator.qml"
        "controls/SearchField.qml"
        "controls/Slider.qml"
        "controls/SpinBox.qml"
        "controls/SplitView.qml"
        "controls/SwipeDelegate.qml"
        "controls/Switch.qml"
        "controls/SwitchDelegate.qml"
        "controls/TabBar.qml"
        "controls/TextArea.qml"
        "controls/TextField.qml"
        "controls/ToolBar.qml"
        "controls/ToolTip.qml"
        "controls/Tumbler.qml"
    RESOURCES
        "qtquickcontrols2.conf"
        "fonts/fontello.ttf"
)
