// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Shapes
import QtQuick.VectorImage

Item {
    id: root

    property color color: "#00000000"
    property alias source: vectorImage.source

    implicitWidth: vectorImage.implicitWidth
    implicitHeight: vectorImage.implicitHeight

    onColorChanged: vectorImage.colorize(vectorImage)

    VectorImage {
        id: vectorImage

        function colorize(item)
        {
            if (root.color.a > 0) {
                if (item instanceof Shape) {
                    for (var j = 0; j < item.data.length; ++j) {
                        if (item.data[j].hasOwnProperty("fillColor"))
                            item.data[j].fillColor = root.color
                    }
                }
                for (var i = 0; i < item.children.length; ++i)
                    colorize(item.children[i])
            }
        }

        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        preferredRendererType: VectorImage.CurveRenderer

        Component.onCompleted: colorize(vectorImage)

        onSourceChanged: colorize(vectorImage)
    }
}
