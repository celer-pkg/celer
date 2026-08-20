// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton

import QtQuick
import QtQuick.Studio.Application

QtObject {
    id: root

    readonly property int width: 1920
    readonly property int height: 1080

    property string relativeFontDirectory: "fonts"

    /* Edit this comment to add your custom font */
    readonly property font font: Qt.font({
                                             family: Application.font.family,
                                             pixelSize: Application.font.pixelSize
                                         })
    readonly property font largeFont: Qt.font({
                                                  family: Application.font.family,
                                                  pixelSize: Application.font.pixelSize * 1.6
                                              })

    readonly property color backgroundColor: "#c2c2c2"


    property StudioApplication application: StudioApplication {
        fontPath: Qt.resolvedUrl("../../content/" + root.relativeFontDirectory)
    }
}
