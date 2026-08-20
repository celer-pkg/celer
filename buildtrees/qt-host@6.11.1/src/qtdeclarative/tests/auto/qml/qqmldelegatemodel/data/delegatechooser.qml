// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Window {
    width: 640
    height: 480
    title: qsTr("Hello World")

    ListView {
        width: 200; height: 400

        model: ListModel {
            ListElement { c: "red" }
            ListElement { c: "green" }
            ListElement { c: "blue" }
            ListElement { c: "purple" }
            ListElement { c: "orange" }
            ListElement { c: "red" }
            ListElement { c: "brown"}
        }

        delegate: DelegateChooser {
            role: "c"
            DelegateChoice { roleValue: "red"; Rectangle { color: modelData; width: 200; height: 50 } }
            DelegateChoice { roleValue: "green"; Rectangle { color: modelData; width: 200; height: 50 } }
            DelegateChoice { roleValue: "blue"; Rectangle { color: modelData; width: 200; height: 50 } }
            DelegateChoice { roleValue: "purple"; Rectangle { color: modelData; width: 200; height: 50 } }
            DelegateChoice { roleValue: "orange"; Rectangle { color: modelData; width: 200; height: 50 } }
            DelegateChoice { roleValue: "brown"; Rectangle { color: modelData; width: 200; height: 50 } }
        }
    }
}
