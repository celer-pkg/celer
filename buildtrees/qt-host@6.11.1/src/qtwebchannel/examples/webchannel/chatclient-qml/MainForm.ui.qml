// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2016 basysKom GmbH, author Bernd Lamecker <bernd.lamecker@basyskom.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    property alias chat: chat
    property alias userlist: userlist
    property alias message: message

    GridLayout {
        anchors.fill: parent
        rows: 2
        columns: 2

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Label {
                id: chat
            }
        }

        Label {
            id: userlist
            width: 150
            Layout.fillHeight: true
        }

        TextField {
            id: message
            height: 50
            Layout.fillWidth: true
            Layout.columnSpan: 2
            focus: true
        }
    }
}
