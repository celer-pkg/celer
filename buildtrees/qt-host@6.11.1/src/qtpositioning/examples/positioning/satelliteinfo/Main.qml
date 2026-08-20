// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtCore
import QtQuick
import QtQuick.Window
import SatelliteInformation

Window {
    id: root

    required property SatelliteModel satellitesModel
    required property SortFilterModel sortFilterModel

    width: 360
    height: 640
    visible: true
    title: qsTr("Satellite Info")


    LocationPermission {
        id: permission
        accuracy: LocationPermission.Precise
        availability: LocationPermission.WhenInUse
    }

    PermissionsScreen {
        anchors.fill: parent
        visible: permission.status !== Qt.PermissionStatus.Granted
        requestDenied: permission.status === Qt.PermissionStatus.Denied
        onRequestPermission: permission.request()
    }

    Component {
        id: applicationComponent
        ApplicationScreen {
            satellitesModel: root.satellitesModel
            sortFilterModel: root.sortFilterModel
        }
    }

    Loader {
        anchors.fill: parent
        active: permission.status === Qt.PermissionStatus.Granted
        sourceComponent: applicationComponent
    }
}
