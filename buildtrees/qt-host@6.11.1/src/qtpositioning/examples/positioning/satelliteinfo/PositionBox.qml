// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property alias latitudeString: latValue.text
    property alias longitudeString: lonValue.text

    implicitHeight: rootLayout.implicitHeight + 2 * rootLayout.anchors.margins
    color: Theme.darkBackgroundColor

    GridLayout {
        id: rootLayout
        anchors {
            fill: parent
            margins: Theme.defaultSpacing * 2
        }
        columns: 3
        Image {
            id: posImg
            source: "icons/place.svg"
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            Layout.rowSpan: 2
        }
        Text {
            text: qsTr("latitude")
            color: Theme.textMainColor
            font.pixelSize: Theme.mediumFontSize
            font.weight: Theme.fontDefaultWeight
        }
        Text {
            id: latValue
            text: qsTr("N/A")
            color: Theme.textMainColor
            font.pixelSize: Theme.mediumFontSize
            font.weight: Theme.fontLightWeight
            Layout.fillWidth: true
        }
        Text {
            text: qsTr("longitude")
            color: Theme.textMainColor
            font.pixelSize: Theme.mediumFontSize
            font.weight: Theme.fontDefaultWeight
        }
        Text {
            id: lonValue
            text: qsTr("N/A")
            color: Theme.textMainColor
            font.pixelSize: Theme.mediumFontSize
            font.weight: Theme.fontLightWeight
            Layout.fillWidth: true
        }
    }
}
