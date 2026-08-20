// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
pragma ComponentBehavior

import QtQuick
import QtQuick.Controls

Item {
    id: mainView

    required property Loader loader

    readonly property real listItemWidth: 280
    readonly property real listItemHeight: 40

    anchors.fill: parent

    ListModel {
        id: testsModel
        ListElement {
            name: "Bar Themes"
            file: "BarThemes.qml"
        }
        ListElement {
            name: "Custom Bars"
            file: "CustomBars.qml"
        }
        ListElement {
            name: "Axis & Grid Setup"
            file: "AxisGridSetup.qml"
        }
        ListElement {
            name: "Bar Selection"
            file: "BarSelection.qml"
        }
        ListElement {
            name: "Bar Labels"
            file: "BarLabels.qml"
        }
        ListElement {
            name: "Add/Remove Series"
            file: "AddRemoveSeries.qml"
        }
        ListElement {
            name: "Add/Remove Pie"
            file: "AddRemovePie.qml"
        }
        ListElement {
            name: "C++ Bar Series"
            file: "CppBarSeries.qml"
        }
        ListElement {
            name: "C++ Line Series"
            file: "CppLineSeries.qml"
        }
        ListElement {
            name: "Line Properties"
            file: "LineProperties.qml"
        }
        ListElement {
            name: "Line Selection"
            file: "LineSelection.qml"
        }
        ListElement {
            name: "Scatter Properties"
            file: "ScatterProperties.qml"
        }
        ListElement {
            name: "Multi Axis"
            file: "MultiAxis.qml"
        }
    }
    ListModel {
        id: testsModel2
        ListElement {
            name: "Callout"
            file: "Callout.qml"
        }
        ListElement {
            name: "Pie Graph"
            file: "PieChart.qml"
        }
        ListElement {
            name: "Spline"
            file: "SplineSeries.qml"
        }
        ListElement {
            name: "Area"
            file: "AreaSeries.qml"
        }
        ListElement {
            name: "Dynamic Series"
            file: "DynamicSeries.qml"
        }
        ListElement {
            name: "Donut"
            file: "Donut.qml"
        }
        ListElement {
            name: "BarChangingSetCount"
            file: "BarChangingSetCount.qml"
        }
        ListElement {
            name: "DateTime Axis"
            file: "DateTimeAxis.qml"
        }
        ListElement {
            name: "BarModelMapping"
            file: "BarModelMapping.qml"
        }
        ListElement {
            name: "QML Usage Series"
            file: "QmlUsageSeries.qml"
        }
        ListElement {
            name: "PieModelMapping"
            file: "PieModelMapping.qml"
        }
        ListElement {
            name: "Custom Input"
            file: "CustomInput.qml"
        }
        ListElement {
            name: "Custom Series"
            file: "Custom.qml"
        }
    }

    Component {
        id: listComponent
        Button {
            id: button
            required property string name
            required property string file

            width: mainView.listItemWidth
            height: mainView.listItemHeight
            background: Rectangle {
                id: buttonBackground
                border.width: 0.5
                border.color: "#d0808080"
                color: "#d0404040"
                opacity: button.hovered ? 1.0 : 0.5
            }
            contentItem: Text {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: "#f0f0f0"
                font.pointSize: settings.fontSizeSmall
                text: button.name
            }

            onClicked: {
                mainView.loader.source = button.file
            }
        }
    }

    Text {
        id: topLabel
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 20
        text: qsTr("QtGraphs - Testbed")
        color: "#f0f0f0"
        font.pointSize: settings.fontSizeLarge
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: topLabel.bottom
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        spacing: 20
        clip: true
        ListView {
            id: examplesListView
            width: mainView.listItemWidth
            height: parent.height
            model: testsModel
            delegate: listComponent
        }
        ListView {
            id: examplesListView2
            width: mainView.listItemWidth
            height: parent.height
            model: testsModel2
            delegate: listComponent
        }
    }
}
