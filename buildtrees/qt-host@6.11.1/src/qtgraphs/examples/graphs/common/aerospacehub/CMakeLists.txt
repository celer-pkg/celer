# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(aerospacehub VERSION 0.1 LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Graphs Quick3D)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(aerospacehub
    main.cpp
)

target_include_directories(aerospacehub PRIVATE
    "simulator/"
)

set_source_files_properties(qml/GlobalSettings.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(aerospacehub
    URI AerospaceHubExample
    QML_FILES
        Main.qml
        qml/Area2DGraph.qml
        qml/Bar2DGraph.qml
        qml/Bar3DGraph.qml
        qml/BigGraphItem.qml
        qml/DashboardPane.qml
        qml/GlobalSettings.qml
        qml/GraphHeaderItem.qml
        qml/ImageItem.qml
        qml/NotificationItem.qml
        qml/Pie2DGraph.qml
        qml/Scatter3DGraph.qml
        qml/SettingsCheckBox.qml
        qml/SettingsComboBox.qml
        qml/SettingsItem.qml
        qml/SettingsSlider.qml
        qml/SmallGraphItem.qml
        qml/StatusItem.qml
        qml/StatusPane.qml
        qml/Surface3DGraph.qml
        qml/ToolBarItem.qml
        qml/ToolTip2D.qml
    RESOURCES
        fonts/Inter-VariableFont_opsz,wght.ttf
        images/BuiltWithQtLogo.png
        images/arrow-head-down-s.png
        images/arrow-head-up-s.png
        images/back-icon.png
        images/clock_24px.png
        images/close_large_16px.png
        images/component-failure-icon_48px.png
        images/component-failure-icon_96px.png
        images/dashboard-bg.png
        images/dashboard-icon.png
        images/energy-production-breakdown-icon_48px.png
        images/energy-production-breakdown-icon_96px.png
        images/energy-production-icon_48px.png
        images/energy-production-icon_96px.png
        images/fleet-level-icon_48px.png
        images/fleet-level-icon_96px.png
        images/info_circle_24px.png
        images/layers_24px.png
        images/lightning_24px.png
        images/meter_24px.png
        images/meter_48px.png
        images/meter_96px.png
        images/pan-icon.png
        images/power-curve-icon_48px.png
        images/power-curve-icon_96px.png
        images/rotate-icon.png
        images/settings-bg.png
        images/settings-icon.png
        images/split-icon.png
        images/turbine-logo.png
        images/wind_24px.png
        images/zoom-in-icon.png
        images/zoom_in_16px.png
)

add_subdirectory(simulator)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(aerospacehub PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.aerospacehub
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(aerospacehub
    PRIVATE
        Qt6::Quick
        Qt6::Graphs
        Qt6::Quick3D
)

include(GNUInstallDirs)
install(TARGETS aerospacehub
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
