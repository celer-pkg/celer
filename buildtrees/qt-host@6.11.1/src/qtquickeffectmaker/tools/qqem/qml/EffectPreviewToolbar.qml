// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Item {
    id: rootItem

    property real contentScale: 1.0
    property bool smoothScaling: true
    property bool useSmoothContent: true
    property bool showContentBorders: false
    property alias currentSourceImage: sourceImageSelector.currentImage
    property alias currentBackgroundImage: backgroundImageSelector.currentImage

    readonly property real contentMinScale: 0.10
    readonly property real contentMaxScale: 4.0
    readonly property real contentScaleStep: 0.10

    function updateScaleSlider(newScale) {
        scaleSlider.value = newScale;
    }

    function getStepSize(zoomingIn) {
        // Differentiate zooming directions so zoomin 1 step in, then 1
        // step out will end up into starting zoom level.
        var z = zoomingIn ? contentScale + 0.01 : contentScale - 0.01;
        if (z >= 2.0)
            return contentScaleStep * 5.0;
        else if (z >= 1.0)
            return contentScaleStep * 2.0;
        return contentScaleStep;
    }

    function contentZoomIn() {
        var step = getStepSize(true);
        var newScale = scaleSlider.value + step;
        newScale = Math.min(newScale, contentMaxScale);
        scaleSlider.value = newScale;
    }

    function contentZoomOut() {
        var step = getStepSize(false);
        var newScale = scaleSlider.value - step;
        newScale = Math.max(newScale, contentMinScale);
        scaleSlider.value = newScale;
    }

    function selectLastPreviewSource() {
        sourceImageSelector.currentIndex = sourceImageSelector.imagesModel.rowCount - 1;
    }

    height: 50

    Behavior on contentScale {
        enabled: rootItem.smoothScaling
        NumberAnimation {
            duration: 400
            easing.type: Easing.InOutQuad
        }
    }

    Rectangle {
        anchors.fill: parent
        color: mainView.backgroundColor1
        opacity: 0.8
    }
    Row {
        anchors.verticalCenter: parent.verticalCenter
        height: 50
        spacing: 2
        x: 5
        CustomImageSelector {
            id: sourceImageSelector
            anchors.verticalCenter: parent.verticalCenter
            description: "Preview source"
            imagesModel: effectManager.settings.sourceImagesModel
            showAddButton: true
            onAddPressed: {
                applicationSettingsDialog.addPreviewSourceImage();
            }
        }
        CustomImageSelector {
            id: backgroundImageSelector
            anchors.verticalCenter: parent.verticalCenter
            description: "Preview background"
            imagesModel: effectManager.settings.backgroundImagesModel
        }
        Item {
            width: 20
            height: 1
        }
        CustomIconButton {
            id: zoomOutButton
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height * 0.6
            width: height
            icon: "images/icon_zoom_out.png"
            description: "Zoom out"
            onClicked: {
                contentZoomOut();
            }
        }
        Slider {
            id: scaleSlider
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 5
            width: 120
            from: contentMinScale
            to: contentMaxScale
            stepSize: contentScaleStep
            value: 1.0
            onValueChanged: {
                contentScale = value;
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: -16
                text: (100 * contentScale).toFixed(0) + " %"
                font.pixelSize: 14
                color: mainView.foregroundColor2
                z: 2
            }
        }
        CustomIconButton {
            id: zoomInButton
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height * 0.6
            width: height
            icon: "images/icon_zoom_in.png"
            description: "Zoom in"
            onClicked: {
                contentZoomIn();
            }
        }
        CustomIconButton {
            id: zoomAutoButton
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height * 0.6
            width: height
            icon: "images/icon_zoom_auto.png"
            description: "Zoom to fit"
            onClicked: {
                autoScaleToContent();
            }
        }

        Item {
            width: 20
            height: 1
        }

        CustomIconButton {
            id: softenButton
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height * 0.6
            width: height
            icon: "images/icon_soften.png"
            toggledIcon: "images/icon_soften_on.png"
            toggleButton: true
            toggled: useSmoothContent
            description: "Bilinear scaling"
            onClicked: {
                useSmoothContent = !useSmoothContent;
            }
        }
        CustomIconButton {
            id: showBordersButton
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height * 0.6
            width: height
            icon: "images/icon_borders.png"
            toggledIcon: "images/icon_borders_on.png"
            toggleButton: true
            toggled: showContentBorders
            description: "Show item borders"
            onClicked: {
                showContentBorders = !showContentBorders;
            }
        }
    }
}
