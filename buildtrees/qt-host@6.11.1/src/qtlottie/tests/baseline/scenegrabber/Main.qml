// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import Qt.labs.lottieqt
import QtQuick.VectorImage

Rectangle {
    id: toplevel
    color: "darkgray"
    property real pad: 10 / Screen.devicePixelRatio
    property real maxDim: 400 / Screen.devicePixelRatio - (3 * pad / 2)
    property real sceneScale: Math.min(0.5 / Screen.devicePixelRatio, maxDim / qtlottie.width)
    property real sceneWidth: qtlottie.width * sceneScale
    property real sceneHeight: qtlottie.height * sceneScale
    width: 2 * sceneWidth + 3 * pad
    height: sceneHeight + 2 * pad


    Image {
        id: la_background
        source: "qrc:///checkered.png"
        fillMode: Image.Tile
        x: toplevel.pad
        y: toplevel.pad
        width: toplevel.sceneWidth
        height: toplevel.sceneHeight

        LottieAnimation {
            id: qtlottie
            scale: toplevel.sceneScale
            transformOrigin: Item.TopLeft
            objectName: "qtlottie_animation_item"
            quality: LottieAnimation.HighQuality
            autoPlay: false
            property int freezeFrame: -1
            onStatusChanged: {
                if (status === LottieAnimation.Ready) {
                    if (freezeFrame < 0)
                        freezeFrame = Math.floor(startFrame + ((endFrame - startFrame) / 2));
                    gotoAndStop(freezeFrame);
                }
            }
            clip: true
        }
    }

    Image {
        id: vi_background
        source: "qrc:///checkered.png"
        fillMode: Image.Tile
        x: la_background.width + 2 * pad
        y: la_background.y
        width: la_background.width
        height: la_background.height

        VectorImage {
            scale: qtlottie.scale
            transformOrigin: Item.TopLeft
            assumeTrustedSource: true
            animations.paused: true
            source: qtlottie.source
            preferredRendererType: VectorImage.CurveRenderer
            clip: true
        }
    }
}
