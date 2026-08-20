// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtWebEngine

Window {
    id: main
    width: 800
    height: 600
    visible: true

    Rectangle {
        id: background
        anchors.fill: parent
        color: "lightsteelblue"
    }

    OAuth2 {
        id: oauth2
        //! [webengine-qml-authorization]
        onAuthorizeWithBrowser:
            (url) => {
                console.log("Starting authorization with WebView")
                authorizationWebView.url = url
                authorizationWebView.visible = true
            }
        onAuthorizationCompleted:
            (success) => {
                console.log("Authorized: " + success);
                authorizationWebView.visible = false
            }
        //! [webengine-qml-authorization]
    }

    Column {
        anchors.centerIn: parent
        Button {
            text: "Authorize"
            onClicked: oauth2.authorize()
        }
    }

    //! [webengine-qml-view]
    WebEngineView {
        id: authorizationWebView
        anchors.fill: parent
        visible: false
    }
    //! [webengine-qml-view]
}
