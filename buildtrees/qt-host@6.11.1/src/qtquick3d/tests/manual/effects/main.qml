// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D.Effects
import QtQuick3D.Helpers
import Qt.labs.platform

Window {
    visible: true
    width: 800
    height: 600
    title: qsTr("Quick3D Effects Test")
    color: "#f0f0f0"

    View3D {
        id: view3D
        property real animationValue: 0.0
        property real fastAnimationValue: animationValue * 10
        anchors.left: settings.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        NumberAnimation on animationValue {
            id: modelAnimation
            running: true
            paused: !animationButton.checked
            loops: Animation.Infinite
            from: 0.0
            to: 360.0
            duration: 40000
        }

        PerspectiveCamera {
            z: 500
            clipNear: 200
            clipFar: 1000
        }

        DirectionalLight {
            eulerRotation.x: -30
            brightness: 2
        }

        environment: SceneEnvironment {
            id: sceneEnvironment
            clearColor: "#09102b"
            backgroundMode: motionBox.checked ? SceneEnvironment.Transparent : SceneEnvironment.Color

            antialiasingMode: modeButton1.checked ? SceneEnvironment.NoAA : modeButton2.checked
                                                    ? SceneEnvironment.SSAA : modeButton3.checked
                                                      ? SceneEnvironment.MSAA : SceneEnvironment.ProgressiveAA

            antialiasingQuality: qualityButton1.checked ? SceneEnvironment.Medium : qualityButton2.checked
                                                          ? SceneEnvironment.High : SceneEnvironment.VeryHigh
            temporalAAEnabled: temporalModeButton.checked
            temporalAAStrength: temporalStrengthSlider.value

            effects: [  ]
        }

        Texture {
            id: corkTexture
            source: "cork.jpg"
            scaleU: 0.5
            scaleV: 0.5
        }

        Texture {
            id: textTexture
            source: "texture.png"
        }

        //! [scene]
        Node {
            id: scene

            Model {
                source: "#Cube"
                x: -100
                eulerRotation.y: 45
                eulerRotation.x: 30 + view3D.animationValue
                scale: Qt.vector3d(2, 2, 2)
                materials: DefaultMaterial {
                    diffuseMap: corkTexture
                }
            }

            Model {
                source: "#Cube"
                x: 350 * Math.sin(view3D.fastAnimationValue/180 * Math.PI)
                y: 350 * Math.cos(view3D.fastAnimationValue/180 * Math.PI)
                z: -300
                eulerRotation.y: 5
                eulerRotation.x: 5
                scale: Qt.vector3d(1.2, 1.2, 1.2)
                materials: DefaultMaterial {
                    diffuseMap: textTexture
                }
            }

            Model {
                source: "#Sphere"
                x: 80
                y: -40
                z: 200
                scale: Qt.vector3d(1.4, 1.4, 1.4)
                materials: PrincipledMaterial {
                    baseColor: "#41cd52"
                    metalness: 0.0
                    roughness: 0.1
                    opacity: 1.0
                }
            }
        }
        //! [scene]
    }
    Button {
        id: animationButton
        anchors.top: settings.top
        anchors.horizontalCenter: settings.horizontalCenter
        text: "Animate!"
        checkable: true
        z: 1
    }
    ScrollView {
        id: settings
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: settingsRect.width

        Flickable {
          clip: false
          anchors.fill: parent
          contentWidth: settingsRect.width
          contentHeight: settingsRect.height
          Rectangle {
            id: settingsRect
            implicitHeight: settingsArea.height
            width: settingsArea.width + 20

            ColumnLayout {
                id: settingsArea

                spacing: 2

                Item {
                    id: animationButtonSpace
                    Layout.preferredHeight: animationButton.height
                }

                EffectBox {
                    id: customBox
                    text: "Custom combination"
                    effect: CustomEffect {
                        amount: customAmount.sliderValue
                    }
                }
                SettingsGroup {
                    visible: customBox.checked
                    EffectSlider {
                        id: customAmount
                        fromValue: 0.0
                        toValue: 0.05
                        sliderValue: 0.01
                        precision: 4
                    }
                }

                EffectBox {
                    id: gradientBox
                    text: "AdditiveColorGradient"
                    effect: AdditiveColorGradient {
                        topColor: gradientTop.colorVector
                        bottomColor: gradientBottom.colorVector
                    }
                }
                SettingsGroup {
                    visible: gradientBox.checked
                    frame: false
                    EffectColor {
                        id: gradientTop
                        colorVector: Qt.vector3d(0.7, 1.0, 0.7)
                        description: "top"
                    }
                    EffectColor {
                        id: gradientBottom
                        colorVector: Qt.vector3d(0.0, 0.0, 0.0)
                        description: "bottom"
                    }
                }

                EffectBox {
                    id: brushBox
                    text: "BrushStrokes"
                    effect: BrushStrokes {
                        brushAngle: brushStrokesAngle.sliderValue
                        brushLength: brushStrokesLength.sliderValue
                        brushSize: brushStrokesSize.sliderValue
                    }
                }
                SettingsGroup {
                    visible: brushBox.checked
                    EffectSlider {
                        id: brushStrokesAngle
                        fromValue: 0.0
                        toValue: 360.0
                        precision: 0
                        sliderValue: 45
                        description: "brush angle"
                    }
                    EffectSlider {
                        id: brushStrokesLength
                        fromValue: 0.0
                        toValue: 3.0
                        sliderValue: 1
                        description: "stroke length"
                    }
                    EffectSlider {
                        id: brushStrokesSize
                        fromValue: 10.0
                        toValue: 200.0
                        sliderValue: 100
                        precision: 0
                        description: "stroke size"
                    }
                }

                //! [effect]
                EffectBox {
                    id: chromaticBox
                    text: "ChromaticAberration"
                    effect: ChromaticAberration {
                        aberrationAmount: chromaticAmount.sliderValue
                        focusDepth: chromaticDepth.sliderValue
                    }
                }
                SettingsGroup {
                    visible: chromaticBox.checked
                    EffectSlider {
                        id: chromaticAmount
                        fromValue: -200.0
                        toValue: 200.0
                        sliderValue: 50
                        precision: 0
                        description: "aberration amount"
                    }
                    EffectSlider {
                        id: chromaticDepth
                        fromValue: 0.0
                        toValue: 1000.0
                        sliderValue: 600
                        precision: 0
                        description: "focus depth"
                    }
                }
                //! [effect]

                EffectBox {
                    id: colorMasterBox
                    text: "ColorMaster"
                    effect: ColorMaster {
                        redStrength: colorMasterRed.sliderValue
                        greenStrength: colorMasterGreen.sliderValue
                        blueStrength: colorMasterBlue.sliderValue
                        saturation: colorMasterSaturation.sliderValue
                    }
                }
                SettingsGroup {
                    visible: colorMasterBox.checked
                    EffectSlider {
                        id: colorMasterRed
                        fromValue: 0.0
                        toValue: 2.0
                        sliderValue: 1
                        precision: 2
                        description: "red strength"
                    }
                    EffectSlider {
                        id: colorMasterGreen
                        fromValue: 0.0
                        toValue: 2.0
                        sliderValue: 1.5
                        precision: 2
                        description: "green strength"
                    }
                    EffectSlider {
                        id: colorMasterBlue
                        fromValue: 0.0
                        toValue: 2.0
                        sliderValue: 1
                        precision: 2
                        description: "blue strength"
                    }
                    EffectSlider {
                        id: colorMasterSaturation
                        fromValue: -1.0
                        toValue: 1.0
                        sliderValue: 0
                        precision: 2
                        description: "saturation"
                    }
                }

                EffectBox {
                    id: dofBox
                    text: "DepthOfFieldHQBlur"
                    effect: DepthOfFieldHQBlur {
                        focusDistance: dofFocusDistance.sliderValue
                        focusRange: dofFocusRange.sliderValue
                        blurAmount: dofBlurAmount.sliderValue
                    }
                }
                SettingsGroup {
                    visible: dofBox.checked
                    EffectSlider {
                        id: dofFocusDistance
                        fromValue: 0.0
                        toValue: 1000.0
                        sliderValue: 400
                        precision: 0
                        description: "focus distance"
                    }
                    EffectSlider {
                        id: dofFocusRange
                        fromValue: 0.0
                        toValue: 400.0
                        sliderValue: 100
                        precision: 0
                        description: "focus range"
                    }
                    EffectSlider {
                        id: dofBlurAmount
                        fromValue: 0.0
                        toValue: 10.0
                        sliderValue: 4
                        precision: 1
                        description: "blur amount"
                    }
                }

                EffectBox {
                    id: desaturateBox
                    text: "Desaturate"
                    effect: Desaturate {
                        amount: desaturateAmount.sliderValue
                    }
                }
                SettingsGroup {
                    visible: desaturateBox.checked
                    EffectSlider {
                        id: desaturateAmount
                        fromValue: 0.0
                        toValue: 1.0
                        sliderValue: 0.7
                    }
                }

                EffectBox {
                    id: rippleBox
                    text: "DistortionRipple"
                    effect: DistortionRipple {
                        radius: rippleRadius.sliderValue
                        distortionWidth: rippleWidth.sliderValue
                        distortionHeight: rippleHeight.sliderValue
                        distortionPhase: ripplePhase.sliderValue
                        center: Qt.vector2d(0.5, 0.5)
                    }
                }
                SettingsGroup {
                    visible: rippleBox.checked
                    EffectSlider {
                        id: rippleRadius
                        fromValue: 0.0
                        toValue: 100
                        sliderValue: 45
                        description: "radius"
                        precision: 1
                    }
                    EffectSlider {
                        id: rippleWidth
                        fromValue: 2.0
                        toValue: 100
                        sliderValue: 90
                        description: "width"
                        precision: 1
                    }
                    EffectSlider {
                        id: rippleHeight
                        fromValue: 0.0
                        toValue: 100
                        sliderValue: 40
                        description: "height"
                        precision: 1
                    }
                    EffectSlider {
                        id: ripplePhase
                        fromValue: 0.0
                        toValue: 360
                        sliderValue: 0
                        description: "phase"
                        precision: 1
                    }
                }

                EffectBox {
                    id: sphereBox
                    text: "DistortionSphere"
                    effect: DistortionSphere {
                        radius: sphereRadius.sliderValue
                        distortionHeight: sphereHeight.sliderValue
                        center: Qt.vector2d(0.5, 0.5)
                    }
                }
                SettingsGroup {
                    visible: sphereBox.checked
                    EffectSlider {
                        id: sphereRadius
                        fromValue: 0.0
                        toValue: 1.0
                        sliderValue: 0.25
                        description: "radius"
                    }
                    EffectSlider {
                        id: sphereHeight
                        fromValue: -1.0
                        toValue: 1.0
                        sliderValue: 0.5
                        description: "height"
                    }
                }

                EffectBox {
                    id: spiralBox
                    text: "DistortionSpiral"
                    effect: DistortionSpiral {
                        radius: spiralRadius.sliderValue
                        distortionStrength: spiralStrength.sliderValue
                        center: Qt.vector2d(0.5, 0.5)
                    }
                }
                SettingsGroup {
                    visible: spiralBox.checked
                    EffectSlider {
                        id: spiralRadius
                        fromValue: 0.0
                        toValue: 1.0
                        sliderValue: 0.25
                        description: "radius"
                    }
                    EffectSlider {
                        id: spiralStrength
                        fromValue: -10.0
                        toValue: 10.0
                        sliderValue: 1
                        precision: 1
                        description: "strength"
                    }
                }

                EffectBox {
                    id: edgeBox
                    text: "EdgeDetect"
                    effect: EdgeDetect {
                        edgeStrength: edgeS.sliderValue
                    }
                }
                SettingsGroup {
                    visible: edgeBox.checked
                    EffectSlider {
                        id: edgeS
                        fromValue: 0.0
                        toValue: 1.0
                        sliderValue: 0.5
                        precision: 2
                    }
                }

                EffectBox {
                    id: embossBox
                    text: "Emboss"
                    effect: Emboss {
                        amount: embossAmount.sliderValue
                    }
                }
                SettingsGroup {
                    visible: embossBox.checked
                    EffectSlider {
                        id: embossAmount
                        fromValue: 0.0
                        toValue: 0.01
                        sliderValue: 0.003
                        precision: 4
                    }
                }

                EffectBox {
                    id: flipBox
                    text: "Flip"
                    effect: Flip {
                        flipHorizontally: flipH.checked
                        flipVertically: flipV.checked
                    }
                }
                SettingsGroup {
                    visible: flipBox.checked
                    CheckBox {
                        id: flipH
                        checked: true
                        text: "horizontal"
                    }
                    CheckBox {
                        id: flipV
                        checked: true
                        text: "vertical"
                    }
                }

                EffectBox {
                    id: hdrBox
                    text: "HDRBloomTonemap"
                    effect: HDRBloomTonemap {
                        bloomThreshold: hdrBloomThreshold.sliderValue
                        blurFalloff: hdrBlurFalloff.sliderValue
                        exposure: hdrExposure.sliderValue
                        gamma: hdrGamma.expValue
                    }
                }
                SettingsGroup {
                    visible: hdrBox.checked
                    EffectSlider {
                        id: hdrBloomThreshold
                        fromValue: 0.0
                        toValue: 1.0
                        precision: 2
                        sliderValue: 1
                        description: "bloomThreshold"
                    }
                    EffectSlider {
                        id: hdrBlurFalloff
                        fromValue: 0.0
                        toValue: 10.0
                        precision: 1
                        sliderValue: 0
                        description: "blurFalloff"
                    }
                    EffectSlider {
                        id: hdrExposure
                        fromValue: -9
                        toValue: 9
                        precision: 1
                        sliderValue: 0
                        description: "exposure"
                    }
                    EffectSlider {
                        id: hdrGamma
                        exponential: true
                        fromValue: Math.log2(0.1)
                        toValue: Math.log2(4.0)
                        precision: 2
                        sliderValue: 0 // i.e. 1
                        description: "gamma"
                    }
                }

                EffectBox {
                    id: motionBox
                    text: "MotionBlur"
                    effect: MotionBlur {
                        fadeAmount: motionAmount.sliderValue
                        blurQuality: motionQuality.sliderValue
                    }
                }
                SettingsGroup {
                    visible: motionBox.checked
                    EffectSlider {
                        id: motionAmount
                        fromValue: 0.0
                        toValue: 1.0
                        precision: 2
                        sliderValue: 0.25
                        description: "fadeAmount"
                    }
                    EffectSlider {
                        id: motionQuality
                        fromValue: 0.1
                        toValue: 1.0
                        precision: 2
                        sliderValue: 0.25
                        description: "blurQuality"
                    }
                }

                EffectBox {
                    id: scatterBox
                    text: "Scatter"
                    effect: Scatter {
                        amount: scatterAmount.sliderValue
                        direction: scatterDirection.currentIndex
                        randomize: scatterRandomize.checked
                    }
                }
                SettingsGroup {
                    visible: scatterBox.checked
                    EffectSlider {
                        id: scatterAmount
                        fromValue: 0
                        toValue: 127
                        precision: 0
                        sliderValue: 10
                        description: "amount"
                    }
                    ComboBox {
                        id: scatterDirection
                        currentIndex: 0
                        displayText: "Dir: " + currentText
                        //0 = both, 1 = horizontal, 2 = vertical
                        model: ["both", "horizontal", "vertical"]
                    }
                    CheckBox {
                        id: scatterRandomize
                        checked: true
                        text: "randomize"
                    }
                }

                EffectBox {
                    id: sCurveBox
                    text: "SCurveTonemap"
                    effect: SCurveTonemap {
                        shoulderSlope: sCurveShoulderSlope.sliderValue
                        shoulderEmphasis: sCurveShoulderEmphasis.sliderValue
                        toeSlope: sCurveToeSlope.sliderValue
                        toeEmphasis: sCurveToeEmphasis.sliderValue
                        contrastBoost: sCurveContrast.sliderValue
                        saturationLevel: sCurveSaturation.sliderValue
                        gammaValue: sCurveGamma.expValue
                        useExposure: sCurveUseExposure.checked
                        whitePoint: sCurveWhitePoint.expValue
                        exposureValue: sCurveExposure.expValue
                    }
                }
                SettingsGroup {
                    visible: sCurveBox.checked
                    EffectSlider {
                        id: sCurveShoulderSlope
                        fromValue: 0.0
                        toValue: 3.0
                        precision: 2
                        sliderValue: 1.0
                        description: "shoulderSlope"
                    }
                    EffectSlider {
                        id: sCurveShoulderEmphasis
                        fromValue: -1.0
                        toValue: 1.0
                        precision: 2
                        sliderValue: 0.0
                        description: "shoulderEmphasis"
                    }
                    EffectSlider {
                        id: sCurveToeSlope
                        fromValue: 0.0
                        toValue: 3.0
                        precision: 2
                        sliderValue: 1.0
                        description: "toeSlope"
                    }
                    EffectSlider {
                        id: sCurveToeEmphasis
                        fromValue: -1.0
                        toValue: 1.0
                        precision: 2
                        sliderValue: 0.0
                        description: "toeEmphasis"
                    }
                    EffectSlider {
                        id: sCurveContrast
                        fromValue: -1.0
                        toValue: 2.0
                        precision: 2
                        sliderValue: 0.0
                        description: "contrastBoost"
                    }
                    EffectSlider {
                        id: sCurveSaturation
                        fromValue: 0.0
                        toValue: 1.0
                        precision: 2
                        sliderValue: 1.0
                        description: "saturationLevel"
                    }
                    EffectSlider {
                        id: sCurveGamma
                        exponential: true
                        fromValue: Math.log2(0.1)
                        toValue: Math.log2(8.0)
                        precision: 2
                        sliderValue: Math.log2(2.2)
                        description: "gammaValue"
                    }
                    EffectSlider {
                        id: sCurveWhitePoint
                        visible: !sCurveUseExposure.checked
                        exponential: true
                        fromValue: Math.log2(0.01)
                        toValue: Math.log2(128)
                        precision: 2
                        sliderValue: 0 // i.e. 1
                        description: "whitePoint"
                    }
                    EffectSlider {
                        id: sCurveExposure
                        visible: sCurveUseExposure.checked
                        exponential: true
                        fromValue: Math.log2(0.01)
                        toValue: Math.log2(16.0)
                        precision: 2
                        sliderValue: 0 // i.e. 1
                        description: "exposureValue"
                    }
                    CheckBox {
                        id: sCurveUseExposure
                        checked: false
                        text: "use exposure instead of whitepoint"
                    }
                }

                EffectBox {
                    id: tiltBox
                    text: "TiltShift"
                    effect: TiltShift {
                         focusPosition: tiltPosition.sliderValue
                         focusWidth: tiltWidth.sliderValue
                         blurAmount: tiltBlur.sliderValue
                         isVertical: tiltVertical.checked
                         isInverted: tiltInverted.checked
                    }
                }
                SettingsGroup {
                    visible: tiltBox.checked
                    EffectSlider {
                        id: tiltBlur
                        fromValue: 0.0
                        toValue: 10.0
                        sliderValue: 4
                        precision: 1
                        description: "blur amount"
                    }
                    EffectSlider {
                        id: tiltPosition
                        fromValue: 0.0
                        toValue: 1.0
                        sliderValue: 0.5
                        precision: 2
                        description: "focus position"
                    }
                    EffectSlider {
                        id: tiltWidth
                        fromValue: 0.0
                        toValue: 1.0
                        sliderValue: 0.2
                        precision: 2
                        description: "focus width"
                    }
                    CheckBox {
                        id: tiltInverted
                        text: "inverted"
                        checked: false
                    }
                    CheckBox {
                        id: tiltVertical
                        text: "vertical"
                        checked: false
                    }
                }

                EffectBox {
                    id: vignetteBox
                    text: "Vignette"
                    effect: Vignette {
                        vignetteColor: vignetteCol.colorVector
                        vignetteRadius: vignetteR.sliderValue
                        vignetteStrength: vignetteS.sliderValue
                    }
                }
                SettingsGroup {
                    visible: vignetteBox.checked
                    EffectColor {
                        id: vignetteCol
                        colorVector:  Qt.vector3d(0.5, 0.5, 0.5)
                        description: "color"
                    }
                    EffectSlider {
                        id: vignetteR
                        fromValue: 0.0
                        toValue: 5.0
                        sliderValue: 0.35
                        precision: 2
                        description: "radius"
                    }
                    EffectSlider {
                        id: vignetteS
                        fromValue: 0.0
                        toValue: 15.0
                        sliderValue: 15
                        precision: 1
                        description: "strength"
                    }
                }

                EffectBox {
                    id: blurBox
                    text: "Simple blur"
                    effect : Blur {
                        amount: blurEffectAmount.sliderValue
                    }
                }
                EffectSlider {
                    visible: blurBox.checked
                    id: blurEffectAmount
                    fromValue: 0.0
                    toValue: 0.01
                    sliderValue: 0.003
                    precision: 4
                }

                EffectBox {
                    id: gaussBox
                    text: "Gaussian blur"
                    effect: GaussianBlur {
                        amount: gaussAmount.sliderValue
                    }
                }
                EffectSlider {
                    visible: gaussBox.checked
                    id: gaussAmount
                    fromValue: 0.0
                    toValue: 10.0
                    sliderValue: 2
                    precision: 1
                }

                EffectBox {
                    id: fxaaCheckBox
                    text: "FXAA effect"
                    effect : Fxaa {}
                }

                EffectBox {
                    id: customFileBox
                    text: "Load from file"
                    effect: FileEffect {
                        id: fileEffect
                        vertexShaderFile: "default.vert"
                        fragmentShaderFile: "default.frag"
                    }
                    FileDialog {
                        id: vertexShaderFileDialog
                        nameFilters: ["Vertex shader files (*.vert)", "All files (*)"]
                        acceptLabel: "Use this vertex shader"
                        onAccepted: fileEffect.vertexShaderFile = file
                    }
                    FileDialog {
                        id: fragmentShaderFileDialog
                        nameFilters: ["Fragment shader files (*.frag)", "All files (*)"]
                        acceptLabel: "Use this fragment shader"
                        onAccepted: fileEffect.fragmentShaderFile = file
                    }
                }
                SettingsGroup {
                    visible: customFileBox.checked
                    Label {
                        text: "Vertex shader: " + fileEffect.vertexShaderFile
                    }
                    Button {
                        text: "Change vertex shader"
                        onClicked: vertexShaderFileDialog.open()
                    }
                    Label {
                        text: "Fragment shader: " + fileEffect.fragmentShaderFile
                    }
                    Button {
                        text: "Change fragment shader"
                        onClicked: fragmentShaderFileDialog.open()
                    }
                }


                ColumnLayout {
                    id: antialiasingSettings
//                    visible: aaCheckBox.checked

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#909090"
                    }
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        font.bold: true
                        text: "Antialiasing mode"
                    }
                    RadioButton {
                        id: modeButton1
                        checked: true
                        text: qsTr("NoAA")
                    }
                    RadioButton {
                        id: modeButton2
                        text: qsTr("SSAA")
                    }
                    RadioButton {
                        id: modeButton3
                        text: qsTr("MSAA")
                    }
                    RadioButton {
                        id: modeButton4
                        text: qsTr("ProgressiveAA")
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#909090"
                    }
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        font.bold: true
                        text: "Antialiasing quality"
                    }
                    ButtonGroup {
                        buttons: antialiasingQualityColumn.children
                    }
                    ColumnLayout {
                        id: antialiasingQualityColumn
                        RadioButton {
                            id: qualityButton1
                            text: qsTr("Normal")
                        }
                        RadioButton {
                            id: qualityButton2
                            checked: true
                            text: qsTr("High")
                        }
                        RadioButton {
                            id: qualityButton3
                            text: qsTr("VeryHigh")
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#909090"
                    }
                    CheckBox {
                        id: temporalModeButton
                        text: "temporalAAEnabled"
                    }
                    Item { width: 1; height: 10 }
                    Slider {
                        id: temporalStrengthSlider
                        from: 0.0
                        to: 2.0
                        value: 0.3
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.verticalCenter
                            anchors.bottomMargin: 16
                            text: "temporalAAStrength: " + temporalStrengthSlider.value.toFixed(2);
                            z: 10
                        }
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#909090"
                }


                function recalcEffects()
                {
                    sceneEnvironment.effects = []
                    for (var i = 0; i < settingsArea.children.length; i++) {
                        let obj = settingsArea.children[i]
                        if (obj.effect) {
//                            console.log("item "+i);
//                            console.log("     obj " + obj);
//                            console.log("   check " + obj.checked)
//                            console.log("  effect " + obj.effect)
                            if (obj.checked)
                                sceneEnvironment.effects.push(obj.effect)
                        }
                    }
                }

            } // ColumnLayout settingsArea
          } // Rectangle contentsRect
        } // Flickable
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    } // ScrollView

    Item {
        width: debugViewToggleText.implicitWidth
        height: debugViewToggleText.implicitHeight
        anchors.right: parent.right
        Text {
            id: debugViewToggleText
            text: "Click here " + (dbg.visible ? "to hide DebugView" : "for DebugView")
            font.pointSize: 8
            color: "white"
            anchors.right: parent.right
            anchors.top: parent.top
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dbg.visible = !dbg.visible
            DebugView {
                y: debugViewToggleText.height * 2
                anchors.right: parent.right
                source: view3D
                id: dbg
                visible: false
            }
        }
    }
}
