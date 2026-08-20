// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Window {
    id: window
    width: 200
    height: 400
    visible: true
    property int counter: 0;
    MouseArea {
        anchors.fill: parent
        onClicked:
        {
            window.counter = (window.counter + 1) % 3;
            pageLoader.source = "plugins"+window.counter+".qml"
        }
    }
    Loader {
        id: pageLoader
        source: "plugins0.qml"
    }
}
