// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
//![0]
import QtQuick

ImageViewer {
    id: viewer

    currentImage: "https://code.qt.io/cgit/qt/qtbase.git/plain/src/widgets/dialogs/images/qtlogo-64.png"

    Text { text: viewer.currentImage }
}
//![0]
