// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

CustomDialog {
    id: root

    title: qsTr("Delete Selected Nodes")
    width: 320
    height: 200
    modal: true
    focus: true
    standardButtons: Dialog.Yes | Dialog.No
    closePolicy: Popup.NoAutoClose

    Text {
        width: parent.width
        text: "Are you sure you want to delete the selected nodes?"
        font.pixelSize: 14
        color: mainView.foregroundColor2
        wrapMode: Text.WordWrap
    }

    onAccepted: {
        mainView.nodeViewItem.deleteSelectedNodes();
    }
}
