// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal
import QtQuick.Layouts

RowLayout {
    id: graphheader

    property alias titletext: title.text
    property alias titlesize: title.font.pixelSize
    property alias desctext: description.text
    property alias descsize: description.font.pixelSize
    property alias imagesource: image.source
    property bool small: false

    Image {
        id: image
        sourceSize.width: graphheader.small ? 48 : 96
        sourceSize.height: graphheader.small ? 48 : 96
        Layout.preferredWidth: graphheader.small ? GlobalSettings.imagesize48px
                                         : GlobalSettings.imagesize96px
        Layout.preferredHeight: Layout.preferredWidth
    }

    ColumnLayout {
        spacing: graphheader.spacing / 4

        Label {
            id: title
            font.bold: true
            color: "white"
        }

        Label {
            id: description
            color: "white"
        }
    }
}
