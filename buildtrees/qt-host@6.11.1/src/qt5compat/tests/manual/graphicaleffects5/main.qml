// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Window {
    width: 640
    height: 600
    visible: true
    title: qsTr("Graphical Effects")

    Image {
        id: bug
        source: "images/bug.jpg"
        sourceSize: Qt.size(parent.width / grid.columns, parent.width / grid.columns)
        smooth: true
        visible: false
    }

    Image {
        id: butterfly
        source: "images/butterfly.png"
        sourceSize: Qt.size(parent.width / grid.columns, parent.width / grid.columns)
        smooth: true
        visible: false
    }

    Grid {
        id: grid
        anchors.fill: parent
        columns: 6

        BrightnessContrastEffect {
            width: parent.width / parent.columns
            height: width
        }

        ColorOverlayEffect {
            width: parent.width / parent.columns
            height: width
        }

        ColorizeEffect {
            width: parent.width / parent.columns
            height: width
        }

        DesaturateEffect {
            width: parent.width / parent.columns
            height: width
        }

        GammaAdjustEffect {
            width: parent.width / parent.columns
            height: width
        }

        HueSaturationEffect {
            width: parent.width / parent.columns
            height: width
        }

        LevelAdjustEffect {
            width: parent.width / parent.columns
            height: width
        }

        ConicalGradientEffect {
            width: parent.width / parent.columns
            height: width
        }

        LinearGradientEffect {
            width: parent.width / parent.columns
            height: width
        }

        RadialGradientEffect {
            width: parent.width / parent.columns
            height: width
        }

        DisplaceEffect {
            width: parent.width / parent.columns
            height: width
        }

        DropShadowEffect {
            width: parent.width / parent.columns
            height: width
        }

        InnerShadowEffect {
            width: parent.width / parent.columns
            height: width
        }

        InnerShadowFastEffect {
            width: parent.width / parent.columns
            height: width
        }

        FastBlurEffect {
            width: parent.width / parent.columns
            height: width
        }

        GaussianBlurEffect {
            width: parent.width / parent.columns
            height: width
        }

        MaskedBlurEffect {
            width: parent.width / parent.columns
            height: width
        }

        RadialBlurEffect {
            width: parent.width / parent.columns
            height: width
        }

        RecursiveBlurEffect {
            width: parent.width / parent.columns
            height: width
        }

        ZoomBlurEffect {
            width: parent.width / parent.columns
            height: width
        }

        DirectionalBlurEffect {
            width: parent.width / parent.columns
            height: width
        }

        GlowEffect {
            width: parent.width / parent.columns
            height: width
        }

        RectangularGlowEffect {
            width: parent.width / parent.columns
            height: width
        }

        OpacityMaskEffect {
            width: parent.width / parent.columns
            height: width
        }

        ThresholdMaskEffect {
            width: parent.width / parent.columns
            height: width
        }

        BlendEffect {
            width: parent.width / parent.columns
            height: width
        }
    }
}
