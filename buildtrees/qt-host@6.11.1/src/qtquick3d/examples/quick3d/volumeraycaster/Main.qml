// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Universal

import VolumetricExample
import "SpacingMap.mjs" as SpacingMap

ApplicationWindow {
    id: window
    width: 1200
    height: 1080
    visible: true

    Universal.theme: Universal.Dark

    FileDialog {
        id: fileDialog
        onAccepted: {
            window.loadFile(selectedFile)
        }
    }

    function clamp(number, min, max) {
        return Math.max(min, Math.min(number, max))
    }

    function loadFile(selectedFile) {
        var width = parseInt(dataWidth.text)
        var height = parseInt(dataHeight.text)
        var depth = parseInt(dataDepth.text)
        var dataSize = dataTypeComboBox.currentText

        // Parses file names of the form:
        // boston_teapot_256x256x178_uint8.raw
        const re = new RegExp(".?([0-9]+)x([0-9]+)x([0-9]+)_([a-zA-Z0-9]+)\.raw")
        let matches = re.exec(String(selectedFile))
        if (matches.length === 5) {
            width = parseInt(matches[1])
            height = parseInt(matches[2])
            depth = parseInt(matches[3])
            dataSize = matches[4]
        }

        let dimensions = Qt.vector3d(width, height, depth).normalized()
        var spacing = SpacingMap.get(String(selectedFile)).times(dimensions)
        let maxSide = Math.max(Math.max(spacing.x, spacing.y), spacing.z)
        spacing = spacing.times(1 / maxSide)

        volumeTextureData.loadAsync(selectedFile, width, height,
                                    depth, dataSize)
        spinner.running = true
    }

    function getColormapSource(currentIndex) {
        switch (currentIndex) {
        case 0:
            return "images/colormap-coolwarm.png"
        case 1:
            return "images/colormap-plasma.png"
        case 2:
            return "images/colormap-viridis.png"
        case 3:
            return "images/colormap-rainbow.png"
        case 4:
            return "images/colormap-gnuplot.png"
        default:
            break
        }
        return ""
    }

    // position and width are normalized [0..1]
    function sliceSliderMin(posX, widthX, posY, widthY, posZ, widthZ) {
        let x = clamp(posX - 0.5 * widthX, 0, 1 - widthX)
        let y = clamp(posY - 0.5 * widthY, 0, 1 - widthY)
        let z = clamp(posZ - 0.5 * widthZ, 0, 1 - widthZ)
        return Qt.vector3d(x, y, z)
    }

    // position and width are normalized [0..1]
    function sliceSliderMax(posX, widthX, posY, widthY, posZ, widthZ) {
        let x = clamp(posX + 0.5 * widthX, widthX, 1)
        let y = clamp(posY + 0.5 * widthY, widthY, 1)
        let z = clamp(posZ + 0.5 * widthZ, widthZ, 1)
        return Qt.vector3d(x, y, z)
    }

    function sliceBoxPosition(x, y, z, xWidth, yWidth, zWidth) {
        let min = sliceSliderMin(x, xWidth, y, yWidth, z, zWidth)
        let max = sliceSliderMax(x, xWidth, y, yWidth, z, zWidth)
        let xMid = (min.x + max.x) * 0.5 - 0.5
        let yMid = (min.y + max.y) * 0.5 - 0.5
        let zMid = (min.z + max.z) * 0.5 - 0.5
        return Qt.vector3d(xMid, yMid, zMid).times(100)
    }

    Connections {
        target: volumeTextureData
        function onLoadSucceeded(source, width, height, depth, dataType) {
            var spacing = SpacingMap.get(String(source)).times(
                        Qt.vector3d(width, height, depth).normalized())
            let maxSide = Math.max(Math.max(spacing.x, spacing.y), spacing.z)
            spacing = spacing.times(1 / maxSide)

            switch (dataType) {
            case 'uint8':
                dataTypeComboBox.currentIndex = 0
                break
            case 'uint16':
                dataTypeComboBox.currentIndex = 1
                break
            case 'int16':
                dataTypeComboBox.currentIndex = 2
                break
            case 'float32':
                dataTypeComboBox.currentIndex = 3
                break
            case 'float64':
                dataTypeComboBox.currentIndex = 4
                break
            }

            dataWidth.text = width
            dataHeight.text = height
            dataDepth.text = depth
            scaleWidth.text = parseFloat(spacing.x.toFixed(4))
            scaleHeight.text = parseFloat(spacing.y.toFixed(4))
            scaleDepth.text = parseFloat(spacing.z.toFixed(4))
            stepLengthText.text = parseFloat((1 / cubeModel.maxSide).toFixed(6))
            volumeTextureData.source = source
            spinner.running = false
        }
        function onLoadFailed(source, width, height, depth, dataType) {
            spinner.running = false
        }
    }

    View3D {
        id: view

        x: settingsPane.x + settingsPane.width
        width: parent.width - x
        height: parent.height

        camera: cameraNode

        PerspectiveCamera {
            id: cameraNode
            z: 300
        }

        //! [cube]
        Model {
            id: cubeModel
            source: "#Cube"
            visible: true
            materials: CustomMaterial {
                shadingMode: CustomMaterial.Unshaded
                vertexShader: "alpha_blending.vert"
                fragmentShader: "alpha_blending.frag"

                //! [volume-texture]
                property TextureInput volume: TextureInput {
                    texture: Texture {
                        textureData: VolumeTextureData {
                            id: volumeTextureData
                            source: "file:///default_colormap"
                            dataType: dataTypeComboBox.currentText ? dataTypeComboBox.currentText : "uint8"
                            width: parseInt(dataWidth.text)
                            height: parseInt(dataHeight.text)
                            depth: parseInt(dataDepth.text)
                        }
                        minFilter: Texture.Nearest
                        mipFilter: Texture.None
                        magFilter: Texture.Nearest
                        tilingModeHorizontal: Texture.ClampToEdge
                        tilingModeVertical: Texture.ClampToEdge
                        //tilingModeDepth: Texture.ClampToEdge // Qt 6.7
                    }
                }
                //! [volume-texture]

                property TextureInput colormap: TextureInput {
                    enabled: true
                    texture: Texture {
                        id: colormapTexture
                        tilingModeHorizontal: Texture.ClampToEdge
                        source: window.getColormapSource(colormapCombo.currentIndex)
                    }
                }
                property real stepLength: Math.max(0.0001, parseFloat(
                                                       stepLengthText.text,
                                                       1 / cubeModel.maxSide))
                property real minSide: 1 / cubeModel.minSide
                property real stepAlpha: stepAlphaSlider.value
                property bool multipliedAlpha: multipliedAlphaBox.checked

                property real tMin: tSlider.first.value
                property real tMax: tSlider.second.value
                property vector3d sliceMin: window.sliceSliderMin(
                                                xSliceSlider.value,
                                                xSliceWidthSlider.value,
                                                ySliceSlider.value,
                                                ySliceWidthSlider.value,
                                                zSliceSlider.value,
                                                zSliceWidthSlider.value)
                property vector3d sliceMax: window.sliceSliderMax(
                                                xSliceSlider.value,
                                                xSliceWidthSlider.value,
                                                ySliceSlider.value,
                                                ySliceWidthSlider.value,
                                                zSliceSlider.value,
                                                zSliceWidthSlider.value)

                sourceBlend: CustomMaterial.SrcAlpha
                destinationBlend: CustomMaterial.OneMinusSrcAlpha
            }
            property real maxSide: Math.max(parseInt(dataWidth.text),
                                            parseInt(dataHeight.text),
                                            parseInt(dataDepth.text))
            property real minSide: Math.min(parseInt(dataWidth.text),
                                            parseInt(dataHeight.text),
                                            parseInt(dataDepth.text))
            scale: Qt.vector3d(parseFloat(scaleWidth.text),
                               parseFloat(scaleHeight.text),
                               parseFloat(scaleDepth.text))

            //! [bounding-boxes]
            Model {
                visible: drawBoundingBox.checked
                geometry: LineBoxGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "#323232"
                    lighting: PrincipledMaterial.NoLighting
                }
                receivesShadows: false
                castsShadows: false
            }

            Model {
                visible: drawBoundingBox.checked
                geometry: LineBoxGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "#323232"
                    lighting: PrincipledMaterial.NoLighting
                }
                receivesShadows: false
                castsShadows: false
                position: window.sliceBoxPosition(xSliceSlider.value,
                                           ySliceSlider.value,
                                           zSliceSlider.value,
                                           xSliceWidthSlider.value,
                                           ySliceWidthSlider.value,
                                           zSliceWidthSlider.value)
                scale: Qt.vector3d(xSliceWidthSlider.value,
                                   ySliceWidthSlider.value,
                                   zSliceWidthSlider.value)
            }
            //! [bounding-boxes]
        }
        //! [cube]

        //! [arcball]
        ArcballController {
            id: arcballController
            controlledObject: cubeModel

            function jumpToAxis(axis) {
                cameraRotation.from = arcballController.controlledObject.rotation
                cameraRotation.to = originGizmo.quaternionForAxis(
                            axis, arcballController.controlledObject.rotation)
                cameraRotation.duration = 200
                cameraRotation.start()
            }

            function jumpToRotation(qRotation) {
                cameraRotation.from = arcballController.controlledObject.rotation
                cameraRotation.to = qRotation
                cameraRotation.duration = 100
                cameraRotation.start()
            }

            QuaternionAnimation {
                id: cameraRotation
                target: arcballController.controlledObject
                property: "rotation"
                type: QuaternionAnimation.Slerp
                running: false
                loops: 1
            }
        }

        DragHandler {
            id: dragHandler
            target: null
            acceptedModifiers: Qt.NoModifier
            onCentroidChanged: {
                arcballController.mouseMoved(toNDC(centroid.position.x,
                                                   centroid.position.y))
            }

            onActiveChanged: {
                if (active) {
                    view.forceActiveFocus()
                    arcballController.mousePressed(toNDC(centroid.position.x,
                                                         centroid.position.y))
                } else
                    arcballController.mouseReleased(toNDC(centroid.position.x,
                                                          centroid.position.y))
            }

            function toNDC(x, y) {
                return Qt.vector2d((2.0 * x / view.width) - 1.0,
                                   1.0 - (2.0 * y / view.height))
            }
        }

        WheelHandler {
            id: wheelHandler
            orientation: Qt.Vertical
            target: null
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: event => {
                         let delta = -event.angleDelta.y * 0.01
                         cameraNode.z += cameraNode.z * 0.1 * delta
                     }
        }
        //! [arcball]

        FrameAnimation {
            running: autoRotateCheckbox.checked
            onTriggered: {
                arcballController.mousePressed(Qt.vector2d(0, 0))
                arcballController.mouseMoved(Qt.vector2d(0.01, 0))
                arcballController.mouseReleased(Qt.vector2d(0.01, 0))
            }
        }

        Keys.onPressed: event => {
                            if (event.key === Qt.Key_Space) {
                                let rotation = originGizmo.quaternionAlign(
                                    arcballController.controlledObject.rotation)
                                arcballController.jumpToRotation(rotation)
                            } else if (event.key === Qt.Key_S) {
                                settingsPane.toggleHide()
                            } else if (event.key === Qt.Key_Left
                                       || event.key === Qt.Key_A) {
                                let rotation = originGizmo.quaternionRotateLeft(
                                    arcballController.controlledObject.rotation)
                                arcballController.jumpToRotation(rotation)
                            } else if (event.key === Qt.Key_Right
                                       || event.key === Qt.Key_D) {
                                let rotation = originGizmo.quaternionRotateRight(
                                    arcballController.controlledObject.rotation)
                                arcballController.jumpToRotation(rotation)
                            }
                        }
    }

    //! [origingizmo]
    OriginGizmo {
        id: originGizmo
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        width: 120
        height: 120
        targetNode: cubeModel

        onAxisClicked: axis => {
                           arcballController.jumpToAxis(axis)
                       }
    }
    //! [origingizmo]

    RoundButton {
        id: iconOpen
        text: "\u2630" // Unicode Character 'TRIGRAM FOR HEAVEN', no qsTr()
        x: settingsPane.x + settingsPane.width + 10
        y: 10
        onClicked: settingsPane.toggleHide()
    }

    Spinner {
        id: spinner
        running: false
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
    }

    //! [settings]
    ScrollView {
        id: settingsPane
        height: parent.height
        property bool hidden: false

        function toggleHide() {
            if (settingsPane.hidden) {
                settingsPaneAnimation.from = settingsPane.x
                settingsPaneAnimation.to = 0
            } else {
                settingsPaneAnimation.from = settingsPane.x
                settingsPaneAnimation.to = -settingsPane.width
            }
            settingsPane.hidden = !settingsPane.hidden
            settingsPaneAnimation.running = true
        }

        NumberAnimation on x {
            id: settingsPaneAnimation
            running: false
            from: view.width
            to: view.width
            duration: 100
        }

        Column {
            topPadding: 10
            bottomPadding: 10
            leftPadding: 20
            rightPadding: 20

            spacing: 10

            Label {
                text: qsTr("Visible value-range:")
            }

            RangeSlider {
                id: tSlider
                from: 0
                to: 1
                first.value: 0
                second.value: 1
            }

            Image {
                width: tSlider.width
                height: 20
                source: window.getColormapSource(colormapCombo.currentIndex)
            }

            Label {
                text: qsTr("Colormap:")
            }

            ComboBox {
                id: colormapCombo
                model: [qsTr("Cool Warm"), qsTr("Plasma"), qsTr("Viridis"), qsTr("Rainbow"), qsTr("Gnuplot")]
            }

            Label {
                text: qsTr("Step alpha:")
            }

            Slider {
                id: stepAlphaSlider
                from: 0
                value: 0.2
                to: 1
            }

            Grid {
                horizontalItemAlignment: Grid.AlignHCenter
                verticalItemAlignment: Grid.AlignVCenter
                spacing: 5
                Label {
                    text: qsTr("Step length:")
                }

                TextField {
                    id: stepLengthText
                    text: "0.00391" // ~1/256
                    width: 100
                }
            }

            CheckBox {
                id: multipliedAlphaBox
                text: qsTr("Multiplied alpha")
                checked: true
            }

            CheckBox {
                id: drawBoundingBox
                text: qsTr("Draw Bounding Box")
                checked: true
            }

            CheckBox {
                id: autoRotateCheckbox
                text: qsTr("Auto-rotate model")
                checked: false
            }

            // X plane
            Label {
                text: qsTr("X plane slice (position, width):")
            }

            Slider {
                id: xSliceSlider
                from: 0
                to: 1
                value: 0.5
            }

            Slider {
                id: xSliceWidthSlider
                from: 0
                value: 1
                to: 1
            }

            // Y plane
            Label {
                text: qsTr("Y plane slice (position, width):")
            }

            Slider {
                id: ySliceSlider
                from: 0
                to: 1
                value: 0.5
            }

            Slider {
                id: ySliceWidthSlider
                from: 0
                value: 1
                to: 1
            }

            // Z plane
            Label {
                text: qsTr("Z plane slice (position, width):")
            }

            Slider {
                id: zSliceSlider
                from: 0
                to: 1
                value: 0.5
            }

            Slider {
                id: zSliceWidthSlider
                from: 0
                value: 1
                to: 1
            }

            // Dimensions
            Label {
                text: qsTr("Dimensions (width, height, depth):")
            }

            Row {
                spacing: 5
                TextField {
                    id: dataWidth
                    text: "256"
                    validator: IntValidator {
                        bottom: 1
                        top: 2048
                    }
                }
                TextField {
                    id: dataHeight
                    text: "256"
                    validator: IntValidator {
                        bottom: 1
                        top: 2048
                    }
                }
                TextField {
                    id: dataDepth
                    text: "256"
                    validator: IntValidator {
                        bottom: 1
                        top: 2048
                    }
                }
            }

            Label {
                text: qsTr("Scale (x, y, z):")
            }

            Row {
                spacing: 5
                TextField {
                    id: scaleWidth
                    text: "1"
                    validator: DoubleValidator {
                        bottom: 0.001
                        top: 1000
                        decimals: 4
                    }
                }
                TextField {
                    id: scaleHeight
                    text: "1"
                    validator: DoubleValidator {
                        bottom: 0.001
                        top: 1000
                        decimals: 4
                    }
                }
                TextField {
                    id: scaleDepth
                    text: "1"
                    validator: DoubleValidator {
                        bottom: 0.001
                        top: 1000
                        decimals: 4
                    }
                }
            }

            Label {
                text: qsTr("Data type:")
            }

            ComboBox {
                id: dataTypeComboBox
                model: ["uint8", "uint16", "int16", "float32", "float64"]
            }

            Label {
                text: qsTr("Load Built-in Volume:")
            }

            Row {
                spacing: 5

                Button {
                    text: qsTr("Helix")
                    onClicked: {
                        volumeTextureData.loadAsync("file:///default_helix",
                                                    256, 256, 256, "uint8")
                        spinner.running = true
                    }
                }

                Button {
                    text: qsTr("Box")
                    onClicked: {
                        volumeTextureData.loadAsync("file:///default_box", 256,
                                                    256, 256, "uint8")
                        spinner.running = true
                    }
                }

                Button {
                    text: qsTr("Colormap")
                    onClicked: {
                        volumeTextureData.loadAsync("file:///default_colormap",
                                                    256, 256, 256, "uint8")
                        spinner.running = true
                    }
                }
            }

            Button {
                text: qsTr("Load Volume...")
                onClicked: fileDialog.open()
            }
        }
    }
    //! [settings]
}
