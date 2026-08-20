# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmltestbed LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmltestbed
    GUI
    SOURCES
        main.cpp
)

target_link_libraries(tst_qmltestbed PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::Graphs
)

set(qmltestbed_resource_files
    "qml/qmltestbed/main.qml"
    "qml/qmltestbed/StartupView.qml"
    "qml/qmltestbed/SettingsView.qml"
    "qml/qmltestbed/CustomCheckBox.qml"
    "qml/qmltestbed/CustomLabel.qml"
    "qml/qmltestbed/CustomSelectionBox.qml"
    "qml/qmltestbed/CustomSlider.qml"
    "qml/qmltestbed/BarThemes.qml"
    "qml/qmltestbed/BarSelection.qml"
    "qml/qmltestbed/BarLabels.qml"
    "qml/qmltestbed/BarChangingSetCount.qml"
    "qml/qmltestbed/BarModelMapping.qml"
    "qml/qmltestbed/LineProperties.qml"
    "qml/qmltestbed/AxisGridSetup.qml"
    "qml/qmltestbed/Callout.qml"
    "qml/qmltestbed/ScatterProperties.qml"
    "qml/qmltestbed/CppBarSeries.qml"
    "qml/qmltestbed/CppLineSeries.qml"
    "qml/qmltestbed/PieChart.qml"
    "qml/qmltestbed/SplineSeries.qml"
    "qml/qmltestbed/CustomBars.qml"
    "qml/qmltestbed/BarsEffect/export/BarsEffect.qml"
    "qml/qmltestbed/AreaSeries.qml"
    "qml/qmltestbed/DynamicSeries.qml"
    "qml/qmltestbed/Donut.qml"
    "qml/qmltestbed/DateTimeAxis.qml"
    "qml/qmltestbed/LineSelection.qml"
    "qml/qmltestbed/AddRemoveSeries.qml"
    "qml/qmltestbed/AddRemovePie.qml"
    "qml/qmltestbed/QmlUsageSeries.qml"
    "qml/qmltestbed/PieModelMapping.qml"
    "qml/qmltestbed/CustomInput.qml"
    "qml/qmltestbed/MultiAxis.qml"
    "qml/qmltestbed/Custom.qml"
    "qml/qmltestbed/images/arrow_icon.png"
    "qml/qmltestbed/images/icon_settings.png"
    "qml/qmltestbed/images/happy_box.png"
    "qml/qmltestbed/images/Sevillana-Regular.ttf"
    "qml/qmltestbed/images/bar_fg_l.png"
    "qml/qmltestbed/images/bar_fg_r.png"
    "qml/qmltestbed/images/bar_fg_c.png"
    "qml/qmltestbed/images/bar_mask_l.png"
    "qml/qmltestbed/images/bar_mask_r.png"
    "qml/qmltestbed/images/bar_mask_c.png"
    "qml/qmltestbed/BarsEffect/export/barseffect.frag.qsb"
    "qml/qmltestbed/BarsEffect/export/barseffect.vert.qsb"
    "qml/qmltestbed/BarsEffect/export/bar_mask.png"
)

qt_internal_add_resource(tst_qmltestbed "qmltestbed"
    PREFIX
        "/"
    FILES
        ${qmltestbed_resource_files}
)

qt_add_qml_module(tst_qmltestbed
    URI TestbedExample
    VERSION 1.0
    SOURCES
        custombarseries.h custombarseries.cpp
        customlineseries.h customlineseries.cpp
        candlestick.h candlestick.cpp
        barsetgenerator.h barsetgenerator.cpp
        barmodelmappermodel.h barmodelmappermodel.cpp
        piemodelmappermodel.h piemodelmappermodel.cpp
)
