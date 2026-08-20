// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Local

Flickable {
    width: content.width
    height: content.height
    contentWidth: content.width
    contentHeight: content.height

    Rectangle {
        id: content
        width: mainRow.implicitWidth
        height: mainRow.implicitHeight

        BluetoothDevice {
            id: device
            function evaluateError(error)
            {
                switch (error) {
                case 0: return "Last Error: NoError"
                case 1: return "Last Error: Pairing Error"
                case 100: return "Last Error: Unknown Error"
                case 1000: return "Last Error: <None>"
                }
            }

            function evaluateSocketState(s) {
                switch (s) {
                case 0: return "Socket: Unconnected";
                case 1: return "Socket: HostLookup";
                case 2: return "Socket: Connecting";
                case 3: return "Socket: Connected";
                case 4: return "Socket: Bound";
                case 5: return "Socket: Listening";
                case 6: return "Socket: Closing";
                }
                return "Socket: <None>";
            }


            onErrorOccurred: (error) => errorText.text = evaluateError(error)
            onHostModeStateChanged: hostModeText.text = device.hostMode;
            onSocketStateUpdate: (foobar) => socketStateText.text = evaluateSocketState(foobar);
        }

        Row {
            id: mainRow
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 4

            spacing: 8
            Column {
                spacing: 8
                Text{
                    width: connectBtn.width
                    horizontalAlignment: Text.AlignLeft
                    font.pointSize: 12
                    wrapMode: Text.WordWrap
                    text: "Device Management"
                }
                Button {

                    buttonText: "PowerOn"
                    onClicked: device.powerOn()
                }
                Button {
                    buttonText: "PowerOff"
                    onClicked: device.setHostMode(0)
                }
                Button {
                    id: connectBtn
                    buttonText: "Connectable"
                    onClicked: device.setHostMode(1)
                }
                Button {
                    buttonText: "Discover"
                    onClicked: device.setHostMode(2)
                }
                Button {
                    buttonText: "Pair"
                    onClicked: device.requestPairingUpdate(true)
                }
                Button {
                    buttonText: "Unpair"
                    onClicked: device.requestPairingUpdate(false)
                }
                Button {
                    buttonText: "Cycle SecFlag"
                    onClicked: device.cycleSecurityFlags()
                }
                Text {
                    id: errorText
                    text: "Last Error: <None>"
                }
                Text {
                    id: hostModeText
                    text: device.hostMode
                }
                Text {
                    id: socketStateText
                    text: device.evaluateSocketState(0)
                }
                Text {
                    id: secFlagLabel;
                    text: "SecFlags: " + device.secFlags
                }
            }
            Column {
                spacing: 8
                Text{
                    width: startFullSDiscBtn.width
                    horizontalAlignment: Text.AlignLeft
                    font.pointSize: 12
                    wrapMode: Text.WordWrap
                    text: "Device & Service Discovery"
                }
                Button {
                    buttonText: "StartDDisc"
                    onClicked: device.startDiscovery()
                }
                Button {
                    buttonText: "StopDDisc"
                    onClicked: device.stopDiscovery()
                }
                Button {
                    buttonText: "StartMinSDisc"
                    onClicked: device.startServiceDiscovery(true)
                }
                Button {
                    id: startFullSDiscBtn
                    buttonText: "StartFullSDisc"
                    onClicked: device.startServiceDiscovery(false)
                }
                Button {
                    id: startRemoteSDiscBtn
                    buttonText: "StartRemSDisc"
                    onClicked: device.startTargettedServiceDiscovery()
                }
                Button {
                    buttonText: "StopSDisc"
                    onClicked: device.stopServiceDiscovery();
                }
                Button {
                    buttonText: "DumpSDisc"
                    onClicked: device.dumpServiceDiscovery();
                }

            }

            Column {
                spacing: 8
                Text{
                    width: connectSearchBtn.width
                    horizontalAlignment: Text.AlignLeft
                    font.pointSize: 12
                    wrapMode: Text.WordWrap
                    text: "Client & Server Socket"
                }
                Button {
                    buttonText: "SocketDump"
                    onClicked: device.dumpSocketInformation()
                }
                Button {
                    buttonText: "CConnect"
                    onClicked: device.connectToService()
                }
                Button {
                    id: connectSearchBtn
                    buttonText: "ConnectSearch"
                    onClicked: device.connectToServiceViaSearch()
                }
                Button {
                    buttonText: "CDisconnect"
                    onClicked: device.disconnectToService()
                }

                Button {
                    buttonText: "CClose"
                    onClicked: device.closeSocket()
                }

                Button {
                    buttonText: "CAbort"
                    onClicked: device.abortSocket()
                }
                Button {
                    //Write to all open sockets ABCABC\n
                    buttonText: "CSWrite"
                    onClicked: device.writeData()
                }
                Button {
                    buttonText: "ServerDump"
                    onClicked: device.dumpServerInformation()
                }
                Button {
                    //Listen on server via port
                    buttonText: "SListenPort"
                    onClicked: device.serverListenPort()
                }
                Button {
                    //Listen on server via uuid
                    buttonText: "SListenUuid"
                    onClicked: device.serverListenUuid()
                }
                Button {
                    //Close Bluetooth server
                    buttonText: "SClose"
                    onClicked: device.serverClose()
                }
            }
            Column {
                spacing: 8
                Text{
                    width: centralBtn.width
                    horizontalAlignment: Text.AlignLeft
                    color: device.centralExists ? "blue" : "black"
                    font.bold: device.centralExists
                    font.pointSize: 12
                    wrapMode: Text.WordWrap
                    text: "Low Energy Central Controller"
                }
                Rectangle {
                    color: "lightsteelblue"
                    width: parent.width
                    height: centralInfo.height
                    clip: true
                    Column {
                        id: centralInfo
                        Text { text: "CState:" + device.centralState }
                        Text { text: "CError:" + device.centralError }
                        Text { text: "SState:" + device.centralServiceState }
                        Text { text: "SError:" + device.centralServiceError }
                        Text { text: "Subscribed: " + device.centralSubscribed }
                        Text { text: "RSSI: " + device.centralRSSI }
                    }
                }
                // The ordinal numbers below indicate the typical sequence
                Button {
                    id: centralBtn
                    buttonText: "1 DeviceDiscovery"
                    onClicked: device.startLeDeviceDiscovery()
                }
                Button {
                    buttonText: "2 CreateCentral"
                    onClicked: device.centralCreate()
                }
                Button {
                    buttonText: "3 ConnectCentral"
                    onClicked: device.centralConnect()
                }
                Button {
                    buttonText: "4 ServiceDiscovery"
                    onClicked: device.centralStartServiceDiscovery()
                }
                Button {
                    buttonText: "5 ServiceDetails"
                    onClicked: device.centralDiscoverServiceDetails();
                }
                Button {
                    buttonText: "CharacteristicRead"
                    onClicked: device.centralCharacteristicRead()
                }
                Button {
                    buttonText: "CharacteristicWrite"
                    onClicked: device.centralCharacteristicWrite()
                }
                Button {
                    buttonText: "DescriptorRead"
                    onClicked: device.centralDescriptorRead()
                }
                Button {
                    buttonText: "DescriptorWrite"
                    onClicked: device.centralDescriptorWrite()
                }
                Button {
                    buttonText: "Sub/Unsubscribe"
                    onClicked: device.centralSubscribeUnsubscribe();
                }
                Button {
                    buttonText: "DeleteController"
                    onClicked: device.centralDelete()
                }
                Button {
                    buttonText: "Disconnect"
                    onClicked: device.centralDisconnect()
                }
                Button {
                    buttonText: "ReadRSSI"
                    onClicked: device.centralReadRSSI()
                }
            }
            Column {
                spacing: 8
                Text{
                    width: centralBtn.width
                    horizontalAlignment: Text.AlignLeft
                    color: device.peripheralExists ? "blue" : "black"
                    font.bold: device.peripheralExists
                    font.pointSize: 12
                    wrapMode: Text.WordWrap
                    text: "Low Energy Peripheral Controller"
                }
                Rectangle {
                    color: "lightsteelblue"
                    width: parent.width
                    height: peripheralInfo.height
                    clip: true
                    Column {
                        id: peripheralInfo
                        Text { text: "CState: " + device.peripheralState }
                        Text { text: "CError: " + device.peripheralError }
                        Text { text: "SState: " + device.peripheralServiceState }
                        Text { text: "SError: " + device.peripheralServiceError }
                    }
                }
                Button {
                    buttonText: "1 CreatePeripheral"
                    onClicked: device.peripheralCreate()
                }
                Button {
                    buttonText: "2 AddServices"
                    onClicked: device.peripheralAddServices()
                }
                Button {
                    buttonText: "3 StartAdvertise"
                    onClicked: device.peripheralStartAdvertising()
                }
                Button {
                    buttonText: "StopAdvertise"
                    onClicked: device.peripheralStopAdvertising()
                }
                Button {
                    buttonText: "CharacteristicRead"
                    onClicked: device.peripheralCharacteristicRead()
                }
                Button {
                    buttonText: "CharacteristicWrite"
                    onClicked: device.peripheralCharacteristicWrite()
                }
                Button {
                    buttonText: "DescriptorRead"
                    onClicked: device.peripheralDescriptorRead()
                }
                Button {
                    buttonText: "DescriptorWrite"
                    onClicked: device.peripheralDescriptorWrite()
                }
                Button {
                    buttonText: "DeleteController"
                    onClicked: device.peripheralDelete()
                }
                Button {
                    buttonText: "Disconnect"
                    onClicked: device.peripheralDisconnect()
                }
            }
            Column {
                spacing: 8
                Text{
                    width: resetBtn.width
                    horizontalAlignment: Text.AlignLeft
                    font.pointSize: 12
                    wrapMode: Text.WordWrap
                    text: "Misc Controls"
                }
                Button {
                    buttonText: "Dump"
                    onClicked: device.dumpInformation()
                }
                Button {
                    id: resetBtn
                    buttonText: "Reset"
                    onClicked: device.reset()
                }
                Button {
                    id: errorButton
                    buttonText: "Errors"
                    onClicked: device.dumpErrors()
                }
            }
        }
    }
}
