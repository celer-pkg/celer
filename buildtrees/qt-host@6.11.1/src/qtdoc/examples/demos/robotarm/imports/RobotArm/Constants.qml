// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
import QtQuick

QtObject {
    readonly property int width: 1280
    readonly property int height: 720

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
}
