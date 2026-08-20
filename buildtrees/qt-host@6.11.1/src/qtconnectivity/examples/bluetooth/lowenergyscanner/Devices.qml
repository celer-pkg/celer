// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtCore
import QtQuick

Rectangle {
    id: devicesPage

    property bool deviceState: Device.state
    signal showServices

    width: 300
    height: 600

    function toggleDiscovery() {
        if (!Device.state) {
            Device.startDeviceDiscovery()
            // if startDeviceDiscovery() failed Device.state is not set
            if (Device.state) {
                info.dialogText = "Searching..."
                info.visible = true
            }
        } else {
            Device.stopDeviceDiscovery()
        }
    }

    onDeviceStateChanged: {
        if (!Device.state)
            info.visible = false
    }

    Header {
        id: header
        anchors.top: parent.top
        headerText: {
            if (Device.state)
                return "Discovering"

            if (Device.devicesList.length > 0)
                return "Select a device"

            return "Start Discovery"
        }
    }

    Dialog {
        id: info
        anchors.centerIn: parent
        visible: false
    }

    ListView {
        id: theListView
        width: parent.width
        clip: true

        anchors.top: header.bottom
        anchors.bottom: connectToggle.top
        model: Device.devicesList

        delegate: Rectangle {
            required property var modelData
            id: box
            height: 100
            width: theListView.width
            color: "lightsteelblue"
            border.width: 2
            border.color: "black"
            radius: 5

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    Device.scanServices(box.modelData.deviceAddress)
                    showServices()
                }
            }

            Label {
                id: deviceName
                textContent: box.modelData.deviceName
                anchors.top: parent.top
                anchors.topMargin: 5
            }

            Label {
                id: deviceAddress
                textContent: box.modelData.deviceAddress
                font.pointSize: deviceName.font.pointSize * 0.7
                anchors.bottom: box.bottom
                anchors.bottomMargin: 5
            }
        }
    }

    Menu {
        id: connectToggle

        menuWidth: parent.width
        anchors.bottom: menu.top
        menuText: {
            visible = Device.devicesList.length > 0
            if (Device.useRandomAddress)
                return "Address type: Random"
            else
                return "Address type: Public"
        }

        onButtonClick: Device.useRandomAddress = !Device.useRandomAddress
    }

    //! [permission-object]
    BluetoothPermission {
        id: permission
        communicationModes: BluetoothPermission.Access
        onStatusChanged: {
            if (permission.status === Qt.PermissionStatus.Denied)
                Device.update = "Bluetooth permission required"
            else if (permission.status === Qt.PermissionStatus.Granted)
                devicesPage.toggleDiscovery()
        }
    }
    //! [permission-object]

    Menu {
        id: menu
        anchors.bottom: parent.bottom
        menuWidth: parent.width
        menuHeight: (parent.height / 6)
        menuText: Device.update
        //! [permission-request]
        onButtonClick: {
            if (permission.status === Qt.PermissionStatus.Undetermined)
                permission.request()
            else if (permission.status === Qt.PermissionStatus.Granted)
                devicesPage.toggleDiscovery()
        }
        //! [permission-request]
    }
}
