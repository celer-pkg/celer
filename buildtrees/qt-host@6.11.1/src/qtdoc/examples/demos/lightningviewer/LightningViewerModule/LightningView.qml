// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtCore
import QtQuick
import QtQuick.Dialogs

Item {
    visible: true

    LocationPermission {
        id: permission
        accuracy: LocationPermission.Precise
        availability: LocationPermission.WhenInUse
        Component.onCompleted: request();
    }

    MapView {
        id: mapView
        anchors.fill: parent
        zoomLevel: 5
        mapType: actionsLayer.mapType
        lightningLayerVisible: actionsLayer.lightningLayerVisible
        distanceLayerVisible: locationAllowed && actionsLayer.distanceLayerVisible
        locationAllowed: permission.status === Qt.Granted
    }

    ActionsLayer {
        id: actionsLayer
        anchors.fill: parent
        anchors.margins: 15
        lightningLayerVisible: true
        distanceLayerVisible: false
        onRecenterRequested: {
            switch (permission.status) {
            case Qt.Granted:
                mapView.recenter();
                break;
            case Qt.Denied:
                permissionMessageDialog.open();
                break;
            default:
                permission.request();
                break;
            }
        }
        onDistanceLayerVisibleChanged: {
            if (permission.status !== Qt.Granted) {
                distanceLayerVisible = false;
                permissionMessageDialog.open();
            }
        }
    }

    Binding {
        target: LightningController
        property: "distanceTimeLayerEnabled"
        value: actionsLayer.distanceLayerVisible
        when: permission.status === Qt.Granted
    }

    MessageDialog {
        id: permissionMessageDialog
        buttons: MessageDialog.Ok
        text: qsTr("The application does not have the permission to access the Location.")
        informativeText: qsTr("Please grant the permission and restart the application.")
        onButtonClicked: function(button, role) {
            if (button === MessageDialog.Ok)
                accept();
        }
    }
}
