// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Fusion
import QtMultimedia
import MediaControls
import Config

Item {
    id: root

    implicitWidth: 380
    implicitHeight: childrenRect.height

    required property int selectedTrack
    required property list<mediaMetaData> metaData
    property string headerText: ""

    function readTracks(metadataList : list<mediaMetaData>) {
        const LanguageKey = 6
        elements.clear()
        if (!metadataList || !metadataList.length)
            return

        elements.append({
            language: "No Track",
            trackNumber: -1
        })

        metadataList.forEach(function (metadata, index) {
            const language = metadata.stringValue(LanguageKey)
            const label = language ? language : "Track " + (index + 1)
            elements.append({
                language: label,
                trackNumber: index
            })
        });
    }

    ListModel { id: elements }

    ButtonGroup { id: group }

    Column {
        spacing: 16

        Label {
            id: header
            text: elements.count ? qsTr(root.headerText) : qsTr("No " + root.headerText + " present")
            font.pixelSize: 18
            font.bold: true
            color: Config.secondaryColor
        }

        Column {
            id: trackList
            spacing: 18
            Repeater {
                model: elements
                delegate: CustomRadioButton {

                    checked: trackNumber === root.selectedTrack
                    text: language

                    required property int trackNumber
                    required property string language

                    ButtonGroup.group: group

                    onClicked: root.selectedTrack = trackNumber
                }
            }
        }
    }

    onMetaDataChanged: readTracks(root.metaData)
}
