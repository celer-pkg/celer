// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: actionsLayer

    property alias mapType: switchMap.mapType
    property bool distanceLayerVisible: false
    property bool lightningLayerVisible: false

    signal recenterRequested();

    GridLayout {
        anchors.fill: parent
        anchors.leftMargin: 15
        anchors.rightMargin: 15
        columns: 3
        rows: 3
        visible: LightningViewConfig.isLandscape

        LayoutItemProxy { target: switchMap }
        Item {
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        Item {
            Layout.columnSpan: 3
            Layout.fillHeight: true
        }

        MapLayersItem {
            id: mapLayersItem
            mapActionButton: mapLayersButtonComponent
            onVisibleChanged: {
                if (!visible && opened)
                    close();
            }
        }
        Item { Layout.fillWidth: true }
        LayoutItemProxy { target: recenterButton }
    }

    GridLayout {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 15
        height: parent.height
        width: 60
        rowSpacing: 15
        columns: 1
        rows: 5
        visible: LightningViewConfig.isPortrait

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        Loader {
            Layout.preferredHeight: 60
            Layout.preferredWidth: 60
            Layout.alignment: Qt.AlignHCenter
            sourceComponent: mapLayersButtonComponent
        }

        LayoutItemProxy { target: recenterButton }

        LayoutItemProxy { target: switchMap; Layout.topMargin: -10 }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    Component {
        id: mapLayersButtonComponent
        Button {
            icon.source: "icons/map-layers.svg"
            icon.color: !enabled ? "#808080" : checked ? "#80EBB6" : "#2CDE85"
            icon.width: 24
            icon.height: 24
            checked: mapLayersButtonConfig.checked
            onClicked: mapLayersButtonConfig.clicked()
        }
    }
    QtObject {
        id: mapLayersButtonConfig
        property bool checked: false
        signal clicked()
    }

    Button {
        id: recenterButton
        Layout.alignment: Qt.AlignHCenter
        icon.source: "icons/recenter.svg"
        icon.color: !enabled ? "#808080" : checked ? "#80EBB6" : "#2CDE85"
        icon.width: 24
        icon.height: 24
        onClicked: actionsLayer.recenterRequested();
    }

    SwitchMap { id: switchMap }

    MapLayersDrawer {
        id: drawer
        parent: Overlay.overlay
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width
        height: 200
        dragMargin: 30
        onVisibleChanged: {
            if (!visible && opened)
                close();
        }
        visible: LightningViewConfig.isPortrait
    }

    states: [
        State {
            when: LightningViewConfig.isLandscape
            PropertyChanges {
                switchMap.orientation: Qt.Horizontal
                mapLayersButtonConfig.checked: mapLayersItem.opened
                mapLayersButtonConfig.onClicked: mapLayersItem.toggle()
            }
        },
        State {
            when: LightningViewConfig.isPortrait
            PropertyChanges {
                switchMap.orientation: Qt.Vertical
                mapLayersButtonConfig.checked: drawer.opened
                mapLayersButtonConfig.onClicked: drawer.toggle()
            }
        }
    ]

    Binding {
        mapLayersItem.lightningLayerVisible: actionsLayer.lightningLayerVisible
        mapLayersItem.distanceLayerVisible: actionsLayer.distanceLayerVisible
        drawer.lightningLayerVisible: actionsLayer.lightningLayerVisible
        drawer.distanceLayerVisible: actionsLayer.distanceLayerVisible
    }
    Connections {
        target: mapLayersItem
        function onLightningLayerVisibleChanged() {
            actionsLayer.lightningLayerVisible = mapLayersItem.lightningLayerVisible
        }
        function onDistanceLayerVisibleChanged() {
            actionsLayer.distanceLayerVisible = mapLayersItem.distanceLayerVisible
        }
    }
    Connections {
        target: drawer
        function onLightningLayerVisibleChanged() {
            actionsLayer.lightningLayerVisible = drawer.lightningLayerVisible
        }
        function onDistanceLayerVisibleChanged() {
            actionsLayer.distanceLayerVisible = drawer.distanceLayerVisible
        }
    }
}
