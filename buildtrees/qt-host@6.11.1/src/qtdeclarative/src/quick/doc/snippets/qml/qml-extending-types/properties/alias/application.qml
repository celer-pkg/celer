// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
//![0]
// application.qml
import QtQuick

ImageViewer {
    id: viewer

    currentImage.source: "https://code.qt.io/cgit/qt/qtbase.git/plain/src/widgets/dialogs/images/qtlogo-64.png"
    currentImage.width: width
    currentImage.height: height
    currentImage.fillMode: Image.Tile

    Text { text: currentImage.source }
}
//![0]
