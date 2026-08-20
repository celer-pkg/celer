// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma ComponentBehavior: Bound

import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Qt.labs.assetdownloader
import CarRendering

Window {
    id: rootWindow

    width: Constants.width
    height: Constants.height

    visibility: Qt.platform.os === 'android' ? Window.FullScreen : Window.AutomaticVisibility

    visible: true
    title: "Car Configurator"

    property url downloadBase

    function downloadProgress(progressValue: int, progressMaximum: int, text: string) {
        progressBar.from = 0
        progressBar.to = progressMaximum
        progressBar.value = progressValue
        progressText.text = text
    }

    function downloadComplete() {
        messageText.text = qsTr("Initializing...")
        progressText.text = ""
        progressBar.visible = false
        rootWindow.downloadBase = downloader.localDownloadDir
        loader.sourceComponent = mainComponent
    }

    function downloadFailed() {
        messageText.text = qsTr("Cannot download assets")
        progressText.text = ""
        progressBar.visible = false
    }

    AssetDownloader {
        id: downloader

        downloadBase: "https://download.qt.io/learning/examples/"
        jsonFileName : "car-configurator-assets-v3.json"
        zipFileName : "car-configurator-assets-v3.zip"
        offlineAssetsFilePath: "assets/assets_download.json"
        preferredLocalDownloadDir: StandardPaths.writableLocation(StandardPaths.AppDataLocation)
    }

    DirectoryFontLoader {
        fontDirectory: rootWindow.downloadBase + "/content/fonts"
    }

    Connections {
        target: downloader

        function onProgressChanged(progressValue: int,
                                   progressMaximum: int,
                                   text: string) {
            rootWindow.downloadProgress(progressValue, progressMaximum, text)
        }

        function onFinished(success: bool) {
            if (success)
                rootWindow.downloadComplete()
            else
                rootWindow.downloadFailed()
        }
    }

    Rectangle {
        id: mainScreen

        color: "black"
        anchors.fill: parent

        property bool ready: false

        Loader {
            id: loader

            anchors.fill: parent
            Component.onCompleted: downloader.start()
        }

        Component {
            id: mainComponent

            ScreenPrimary {
                downloadBase: rootWindow.downloadBase
                onAssetPreLoadCompleteChanged: {
                    mainScreen.ready = assetPreLoadComplete
                }
            }
        }
    }

    Rectangle {
        id: splashScreen

        color: "black"
        anchors.fill: parent
        visible: !mainScreen.ready

        Behavior on opacity { PropertyAnimation { duration: Constants.animationDuration } }

        Image {
            id: builtWithQtLogo

            source: "images/builtWithQt.png"
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * .3
        }

        Text {
            id: messageText

            color: "white"
            text: qsTr("Downloading Assets...")
            font.pixelSize: 30
            font.styleName: "Bold"

            anchors.top: builtWithQtLogo.bottom
            anchors.topMargin: 30
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: progressText

            color: "white"
            font.pixelSize: 16

            anchors.top: messageText.bottom
            anchors.topMargin: 30
            anchors.horizontalCenter: parent.horizontalCenter
        }

        ProgressBar {
            id: progressBar

            width: parent.width * 0.3
            height: 10

            anchors.top: progressText.bottom
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
