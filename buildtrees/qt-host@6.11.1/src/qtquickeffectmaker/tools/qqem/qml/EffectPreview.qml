// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import "../nodes/common"

Item {
    id: rootItem

    property real animatedTime: previewFrameTimer.elapsedTime + playbackTimeComponent.scrubTime
    property int animatedFrame: previewFrameTimer.currentFrame + playbackTimeComponent.scrubFrame
    property bool timeRunning: previewAnimationRunning
    property rect paddingRect: effectManager.effectPadding
    property alias source: source

    // Scale the content automatically to closest scale step
    // With a small minimum margin
    function autoScaleToContent() {
        const minMargin = 10;
        var areaWidth = rootItem.width;
        var areaHeight = rootItem.height - effectPreviewToolbar.height;
        var maxScaleW = areaWidth / (componentParent.width + minMargin);
        var maxScaleH = areaHeight / (componentParent.height + minMargin);
        var maxScale = Math.min(maxScaleW, maxScaleH);
        var optimalScale = Math.floor(maxScale / effectPreviewToolbar.contentScaleStep) * effectPreviewToolbar.contentScaleStep;
        effectPreviewToolbar.updateScaleSlider(optimalScale);
    }

    function renderToImage(filename) {
        componentParent.grabToImage(function(result) {
            result.saveToFile(filename);
        });
    }

    function selectLastPreviewSource() {
        effectPreviewToolbar.selectLastPreviewSource();
    }

    clip: true

    Image {
        anchors.fill: parent
        source: effectPreviewToolbar.currentBackgroundImage
        fillMode: Image.PreserveAspectCrop
    }

    Item {
        id: source
        x: -paddingRect.x
        y: -paddingRect.y
        width: sourceImage.width + paddingRect.x + paddingRect.width
        height: sourceImage.height + paddingRect.y + paddingRect.height
        layer.enabled: true
        layer.mipmap: true
        layer.smooth: effectPreviewToolbar.useSmoothContent
        visible: false
        Image {
            id: sourceImage
            x: paddingRect.x
            y: paddingRect.y
            source: effectPreviewToolbar.currentSourceImage
            fillMode: Image.PreserveAspectFit
            smooth: effectPreviewToolbar.useSmoothContent
            Behavior on source {
                SequentialAnimation {
                    NumberAnimation {
                        target: sourceImage
                        property: "opacity"
                        to: 0.0
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAction {
                        target: sourceImage
                        property: "source"
                    }
                    ScriptAction {
                        script: {
                            // When image is changed, automatically scale it
                            // And do this directly, without smoothness
                            effectPreviewToolbar.smoothScaling = false;
                            autoScaleToContent();
                            effectPreviewToolbar.smoothScaling = true;
                        }
                    }
                    NumberAnimation {
                        target: sourceImage
                        property: "opacity"
                        to: 1.0
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
    }

    // Mouse handling for iMouse variable
    property real _effectMouseX: 0
    property real _effectMouseY: 0
    property real _effectMouseZ: 0
    property real _effectMouseW: 0
    MouseArea {
        anchors.fill: componentParent
        scale: componentParent.scale
        onPressed: (mouse)=> {
            _effectMouseX = mouse.x
            _effectMouseY = mouse.y
            _effectMouseZ = mouse.x
            _effectMouseW = mouse.y
            clickTimer.restart();
        }
        onPositionChanged: (mouse)=> {
            _effectMouseX = mouse.x
            _effectMouseY = mouse.y
        }
        onReleased: (mouse)=> {
            _effectMouseZ = -(_effectMouseZ)
        }
        onWheel: (wheel)=> {
            if (wheel.angleDelta.y > 0)
                effectPreviewToolbar.contentZoomIn();
            else
                effectPreviewToolbar.contentZoomOut();
        }

        Timer {
            id: clickTimer
            interval: 20
            onTriggered: {
                _effectMouseW = -(_effectMouseW)
            }
        }
    }

    BlurHelper {
        id: blurHelper
        anchors.fill: parent
        property int blurMax: g_propertyData.blur_helper_max_level ? g_propertyData.blur_helper_max_level : 64
        property real blurMultiplier: g_propertyData.blurMultiplier ? g_propertyData.blurMultiplier : 0
    }

    Item {
        id: componentParent
        width: source.width
        height: source.height
        anchors.centerIn: parent
        anchors.verticalCenterOffset: effectPreviewToolbar.height / 2
        scale: effectPreviewToolbar.contentScale
        // Cache the layer. This way heavy shaders rendering doesn't
        // slow down code editing & rest of the UI.
        layer.enabled: true
        layer.smooth: effectPreviewToolbar.useSmoothContent
    }

    BorderImage {
        source: "images/item_border.png"
        anchors.fill: componentParent
        anchors.margins: -2
        border.left: 4
        border.top: 4
        border.right: 4
        border.bottom: 4
        scale: componentParent.scale
        visible: effectPreviewToolbar.showContentBorders
    }

    EffectPreviewToolbar {
        id: effectPreviewToolbar
        anchors.top: parent.top
        width: parent.width
    }

    function createNewComponent() {
        var oldComponent = componentParent.children[0];
        if (oldComponent)
            oldComponent.destroy();

        try {
            const newObject = Qt.createQmlObject(
                effectManager.qmlComponentString,
                componentParent,
                ""
            );
            effectManager.resetEffectError(0);
        } catch(error) {
            let errorString = "QML: ERROR: ";
            let errorLine = -1;
            if (error.qmlErrors.length > 0) {
                // Show the first QML error
                let e = error.qmlErrors[0];
                errorString += e.lineNumber + ": " + e.message;
                errorLine = e.lineNumber;
            }
            effectManager.setEffectError(errorString, 0, errorLine);
        }
    }

    Connections {
        target: effectManager
        function onShadersBaked() {
            mainWindow.releaseResources();
            updateTimer.restart();
        }
    }
    Connections {
        target: effectManager.uniformModel
        function onUniformsChanged() {
            mainWindow.releaseResources();
            updateTimer.restart();
        }
    }
    Connections {
        target: effectManager
        function onQmlComponentStringChanged() {
            mainWindow.releaseResources();
            updateTimer.restart();
        }
    }

    Timer {
        id: updateTimer
        interval: effectManager.effectUpdateDelay();
        onTriggered: {
            console.debug("Updating ShaderEffect!");
            effectManager.updateQmlComponent();
            createNewComponent();
        }
    }
}
