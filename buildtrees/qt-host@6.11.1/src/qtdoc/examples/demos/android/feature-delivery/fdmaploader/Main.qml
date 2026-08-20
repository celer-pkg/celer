// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import io.qt.fdmaploader

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("MapLoader")

    background: Item {
        id: windowBackground
        anchors.fill: parent
        Image {
            id: image
            Component.onCompleted: {
                if (loadedImages.count > 0) {
                    image.source = loadedImages.get(0).name;
                }
            }
        }
    }

    MapLoader {
        id: mapLoader
    }

    MouseArea {
        id: panArea
        anchors.fill: parent
        drag.target: image
        drag.minimumX: parent.width - image.width
        drag.maximumX: 0
        drag.minimumY: parent.height - image.height
        drag.maximumY: 0
    }

    GridLayout {
        id: buttonLayout

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            left: parent.left
            right: parent.right
        }

        columns: 3

        BasicButton {
            id: loadMapButton
            buttonText: "Load Map"
            onClicked: {
                mapLoader.loadModuleFromStore("fdwintermapmodule")
                enabled = false
            }
        }

        BasicButton {
            id:  changeMapButton
            buttonText: "Change Map"
            onClicked: selectionPopup.open();
        }

        BasicButton {
            id: showMapInfoButton
            buttonText: "Show Info"
            onClicked: mapLoader.showMapInfo();
        }

        Item {}

        BasicButton {
            id: removeMapButton
            buttonText: "Remove Map"
            onClicked: {
                if (loadedImages.count > 1) {
                    mapLoader.removeModule("fdwintermapmodule");
                    loadMapButton.enabled = true;
                    changeMapButton.enabled = false;
                    removeMapButton.enabled = false;
                    if (loadedImages.count > 0) {
                        image.source = loadedImages.get(0).name;
                    }
                }
            }
        }

        Item {}
    }

    function showToast (message) {
        toastText.text = message
        toastPopup.visible = true
        toastTimer.start()
    }

    function showPopup (message) {
        mapInfoPopupText.text = message
        mapInfoPopup.open();
    }

    Timer {
        id: toastTimer
        interval: 3000
        repeat: false
        onTriggered: toastPopup.visible = false
    }

    Rectangle {
        id: toastPopup
        width: parent.width
        height: 50
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        color: "black"
        radius: 10
        visible: false
        Text {
            id: toastText
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 20
        }
    }

    Popup {
        id: mapInfoPopup
        width: (parent.width * 2 / 3)
        height: mapInfoPopupText.height + mapInfoPopupOkButton.height + 20
        anchors.centerIn: parent
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color: "lightblue"
            radius: 2
            border.width: 2
            border.color: "darkgray"
        }
        Text {
            id: mapInfoPopupText
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            font.pixelSize: 20
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            text: "Map info not loaded"
        }

        BasicButton {
            id: mapInfoPopupOkButton
            anchors.top: mapInfoPopupText.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            buttonText: "Close"
            onClicked: mapInfoPopup.close();
        }
    }

    Popup {
        id: downloadPopup
        width: (parent.width * 2 / 3)
        height: mapInfoPopupText.height + mapInfoPopupOkButton.height + 20
        anchors.centerIn: parent
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color: "lightblue"
            radius: 2
            border.width: 2
            border.color: "darkgray"
        }
        ProgressBar {
            id: downloadProgressBar
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
        }
        BasicButton {
            id: downloadCancelButton
            anchors.top: downloadProgressBar.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 10
            buttonText: "Cancel"
            onClicked: {
                downloadPopup.close();
                mapLoader.cancelDownload();
            }
        }
    }

    Popup {
        id: selectionPopup
        width: parent.width - 20
        height: parent.height - 20
        anchors.centerIn: parent
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color: "lightblue"
            radius: 2
            border.width: 2
            border.color: "darkgray"
        }
        Text {
            id: popupHeader
            anchors.horizontalCenter: parent.horizontalCenter
            color: "black"
            font.pixelSize: 20
            text: "Select image"
        }
        Column {
            id: contentColumn
            width: parent.width
            height: parent.height - popupHeader.height - okButton.height
            anchors.top: popupHeader.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2

            ListView {
                height: parent.height
                width: parent.width
                model: loadedImages
                delegate: RadioButton {
                    text: modelData
                    onClicked: {
                        image.source = modelData
                    }
                }
            }
        }
        BasicButton {
            id: okButton
            anchors.top: contentColumn.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            buttonText: "Close"
            onClicked: selectionPopup.close();
        }
    }

    ListModel {
        id: loadedImages
        Component.onCompleted: {
            var imageNames = mapLoader.getImageNames();
            updateImageList();
            if (loadedImages.count > 0) {
                image.source = loadedImages.get(0).name;
            }
            if (loadedImages.count <= 1) {
                changeMapButton.enabled = false;
                removeMapButton.enabled = false;
            }
        }
    }

    Connections {
        target: mapLoader
        function onShowToast(message) {
            showToast(message);
        }
        function onShowMapInfoPopup(message) {
            showPopup(message);
        }
        function onModuleLoaded() {
            updateImageList();
            changeMapButton.enabled = true;
            removeMapButton.enabled = true;
        }
        function onErrorOccured(message, errorResult) {
            console.log("Error: " + message);
            showPopup(message);
            if (mapLoader.ExitApp == errorResult) {
                Qt.callLater(Qt.quit);
            }
        }
        function onShowDownloadPopup() {
            downloadPopup.open()
        }
        function onHideDownloadPopup() {
            downloadPopup.close();
        }
        function onUpdateDownloadProgress(bytes, total) {
            downloadProgressBar.value = bytes;
            downloadProgressBar.to = total;
        }
        function onInstallCanceled() {
            showToast("Install cancelled");
            loadMapButton.enabled = true;
            changeMapButton.enabled = false;
            removeMapButton.enabled = false;
        }
    }

    function updateImageList() {
        loadedImages.clear();
        var imageNames = mapLoader.getImageNames();
        for (var i = 0; i < imageNames.length; i++)
        {
            var name = "qrc:/images/" + imageNames[i];
            loadedImages.append({"name":name});
        }
    }
}
