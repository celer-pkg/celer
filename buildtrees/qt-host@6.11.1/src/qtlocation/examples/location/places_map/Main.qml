// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtCore
import QtQuick

Item {
    id: root
    anchors.fill: parent

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
        PlacesMap {}
    }

    Loader {
        anchors.fill: parent
        active: permission.status === Qt.PermissionStatus.Granted
        sourceComponent: applicationComponent
    }
}
