// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import NdefEditorModule

SwipeDelegate {
    required property int index
    required property int recordType
    required property string recordText

    id: delegate

    contentItem: ColumnLayout {
        Label {
            text: qsTr("Record %L1 - %2").arg(delegate.index + 1).arg(
                    delegate.describeRecordType(delegate.recordType))
            font.bold: true
        }

        Label {
            text: delegate.recordText
            elide: Text.ElideRight
            maximumLineCount: 1
            Layout.fillWidth: true
        }
    }

    swipe.left: Button {
        text: qsTr("Delete")
        padding: 12
        height: delegate.height
        anchors.left: delegate.left
        SwipeDelegate.onClicked: delegate.deleteClicked()
    }

    function describeRecordType(type) {
        switch (type) {
        case NdefMessageModel.TextRecord: return qsTr("Text")
        case NdefMessageModel.UriRecord: return qsTr("URI")
        default: return qsTr("Other")
        }
    }

    signal deleteClicked()
}
