// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Page {
    id: toolPage

    required property PerspectiveCamera camera
    required property var sceneEnvironment
    required property Texture lutTexture

    component HorizontalSpacer : Item {
        Layout.fillWidth: true
    }

    component ResetButton : Button {
        signal reset()

        text: "Reset to Defaults"
        Layout.alignment: Qt.AlignRight
        onClicked: {
            reset()
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: 10
        contentHeight: editableSections.implicitHeight

        ColumnLayout {
            id: editableSections
            width: parent.width

            // AA
            SectionLayout {
                id: antialiasingSection
                title: "Antialiasing"

                Label {
                    text: "AA Mode"
                    Layout.fillWidth: true
                }

                RowLayout {
                    HorizontalSpacer { }
                    ComboBox {
                        id: antialiasingModeComboBox
                        textRole: "text"
                        valueRole: "value"
                        implicitContentWidthPolicy: ComboBox.WidestText
                        onActivated: toolPage.sceneEnvironment.antialiasingMode = currentValue
                        Layout.fillWidth: true
                        model: [
                            { value: SceneEnvironment.NoAA, text: "No Antialiasing"},
                            { value: SceneEnvironment.SSAA, text: "Supersample AA"},
                            { value: SceneEnvironment.MSAA, text: "Multisample AA"},
                            { value: SceneEnvironment.ProgressiveAA, text: "Progressive AA"}
                        ]

                        Connections {
                            target: toolPage.sceneEnvironment
                            function onAntialiasingModeChanged() {
                                antialiasingModeComboBox.currentIndex = antialiasingModeComboBox.indexOfValue(toolPage.sceneEnvironment.antialiasingMode)
                            }
                        }
                    }
                }

                Label {
                    text: "AA Quality"
                    Layout.fillWidth: true
                    visible: toolPage.sceneEnvironment.antialiasingMode !== SceneEnvironment.NoAA
                }

                RowLayout {
                    visible: toolPage.sceneEnvironment.antialiasingMode !== SceneEnvironment.NoAA
                    HorizontalSpacer { }
                    ComboBox {
                        id: antialiasingQualityComboBox
                        Layout.fillWidth: true
                        textRole: "text"
                        valueRole: "value"
                        implicitContentWidthPolicy: ComboBox.WidestText
                        onActivated: toolPage.sceneEnvironment.antialiasingQuality = currentValue
                        model: [
                            { value: SceneEnvironment.Medium, text: "Medium"},
                            { value: SceneEnvironment.High, text: "High"},
                            { value: SceneEnvironment.VeryHigh, text: "VeryHigh"}
                        ]

                        Connections {
                            target: toolPage.sceneEnvironment
                            function onAntialiasingModeChanged() {
                                antialiasingQualityComboBox.currentIndex = antialiasingQualityComboBox.indexOfValue(toolPage.sceneEnvironment.antialiasingQuality)
                            }
                        }
                    }
                }

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enable FXAA"
                    checked: toolPage.sceneEnvironment.fxaaEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.fxaaEnabled = checked
                    }
                }

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enable Temporal AA"
                    checked: toolPage.sceneEnvironment.temporalAAEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.temporalAAEnabled = checked
                    }
                }

                Label {
                    visible: toolPage.sceneEnvironment.temporalAAEnabled
                    text: "Temporal AA Strength (" + toolPage.sceneEnvironment.temporalAAStrength.toFixed(2) + ")"
                    Layout.fillWidth: true
                }

                Slider {
                    visible: toolPage.sceneEnvironment.temporalAAEnabled
                    Layout.fillWidth: true
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.temporalAAStrength
                    onValueChanged:
                        toolPage.sceneEnvironment.temporalAAStrength = value
                }


                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enable Specular AA"
                    checked: toolPage.sceneEnvironment.specularAAEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.specularAAEnabled = checked
                    }
                }
                HorizontalSpacer { }
                ResetButton {
                    onReset: {
                        toolPage.sceneEnvironment.antialiasingMode = SceneEnvironment.NoAA
                        toolPage.sceneEnvironment.antialiasingQuality = SceneEnvironment.High
                        toolPage.sceneEnvironment.fxaaEnabled = false
                        toolPage.sceneEnvironment.temporalAAEnabled = false
                        toolPage.sceneEnvironment.temporalAAStrength = 0.3
                        toolPage.sceneEnvironment.specularAAEnabled = false
                    }
                }
            }

            // TONEMAPPING
            SectionLayout {
                id: tonemappingSection
                title: "Tonemapping"

                Label {
                    text: "Tonemapping Mode"
                    Layout.fillWidth: true
                }

                ComboBox {
                    id: tonemappingModeComboBox
                    Layout.fillWidth: true
                    textRole: "text"
                    valueRole: "value"
                    implicitContentWidthPolicy: ComboBox.WidestText
                    onActivated: toolPage.sceneEnvironment.tonemapMode = currentValue
                    Component.onCompleted: tonemappingModeComboBox.currentIndex = tonemappingModeComboBox.indexOfValue(toolPage.sceneEnvironment.tonemapMode)
                    model: [
                        { value: SceneEnvironment.TonemapModeNone, text: "None"},
                        { value: SceneEnvironment.TonemapModeAces, text: "ACES"},
                        { value: SceneEnvironment.TonemapModeFilmic, text: "Filmic"},
                        { value: SceneEnvironment.TonemapModeHejlDawson, text: "HejlDawson"},
                        { value: SceneEnvironment.TonemapModeLinear, text: "Linear"}
                    ]

                    Connections {
                        target: toolPage.sceneEnvironment
                        function onTonemapModeChanged() {
                            tonemappingModeComboBox.currentIndex = tonemappingModeComboBox.indexOfValue(toolPage.sceneEnvironment.tonemapMode)
                        }
                    }
                }

                Label {
                    text: "Exposure (" + (toolPage.sceneEnvironment.exposure).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 10.0
                    value: toolPage.sceneEnvironment.exposure
                    onValueChanged:
                        toolPage.sceneEnvironment.exposure = value
                }

                Label {
                    text: "Probe Exposure (" + (toolPage.sceneEnvironment.probeExposure).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 10.0
                    value: toolPage.sceneEnvironment.probeExposure
                    onValueChanged:
                        toolPage.sceneEnvironment.probeExposure = value
                }

                Label {
                    enabled: !(toolPage.sceneEnvironment.tonemapMode === SceneEnvironment.TonemapModeLinear || toolPage.sceneEnvironment.tonemapMode === SceneEnvironment.TonemapModeNone)
                    text: "White Point (" + (toolPage.sceneEnvironment.whitePoint).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: !(toolPage.sceneEnvironment.tonemapMode === SceneEnvironment.TonemapModeLinear || toolPage.sceneEnvironment.tonemapMode === SceneEnvironment.TonemapModeNone)
                    Layout.fillWidth: true
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.whitePoint
                    onValueChanged:
                        toolPage.sceneEnvironment.whitePoint = value
                }

                Label {
                    text: "Sharpness Amount (" + (toolPage.sceneEnvironment.sharpnessAmount).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.sharpnessAmount
                    onValueChanged:
                        toolPage.sceneEnvironment.sharpnessAmount = value
                }


                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enable Dithering"
                    checked: toolPage.sceneEnvironment.ditheringEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.ditheringEnabled = checked
                    }
                }

                HorizontalSpacer {
                }
                ResetButton {
                    onReset: {
                        toolPage.sceneEnvironment.tonemapMode = SceneEnvironment.TonemapModeLinear
                        toolPage.sceneEnvironment.exposure = 1.0
                        toolPage.sceneEnvironment.whitePoint = 1.0
                        toolPage.sceneEnvironment.sharpnessAmount = 0.0
                        toolPage.sceneEnvironment.ditheringEnabled = false
                    }
                }
            }

            // AO
            SectionLayout {
                id: aoSection
                title: "Ambient Occlusion"
                isExpanded: false

                readonly property bool aoEnabled: toolPage.sceneEnvironment.aoEnabled

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enable Screen Space Ambient Occlusion"
                    checked: toolPage.sceneEnvironment.aoEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.aoEnabled = checked
                    }
                }

                Label {
                    enabled: aoSection.aoEnabled
                    text: "Strength (" + (toolPage.sceneEnvironment.aoStrength).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: aoSection.aoEnabled
                    Layout.fillWidth: true
                    from: aoSection.aoEnabled ? 0.01 : 0.0
                    to: 100.0
                    value: toolPage.sceneEnvironment.aoStrength
                    onValueChanged: {
                        toolPage.sceneEnvironment.aoStrength = value
                    }
                }


                Label {
                    enabled: aoSection.aoEnabled
                    text: "Softness (" + (toolPage.sceneEnvironment.aoSoftness).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: aoSection.aoEnabled
                    Layout.fillWidth: true
                    from: 0.0
                    to: 50.0
                    value: toolPage.sceneEnvironment.aoSoftness
                    onValueChanged:
                        toolPage.sceneEnvironment.aoSoftness = value
                }


                Label {
                    enabled: aoSection.aoEnabled
                    text: "Distance (" + (toolPage.sceneEnvironment.aoDistance).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: aoSection.aoEnabled
                    Layout.fillWidth: true
                    from: 0.01
                    to: 5.0
                    value: toolPage.sceneEnvironment.aoDistance
                    onValueChanged:
                        toolPage.sceneEnvironment.aoDistance = value
                }

                Label {
                    enabled: aoSection.aoEnabled
                    text: "Sample Rate"
                    Layout.fillWidth: true
                }

                ComboBox {
                    id: aoSampleRateComboBox
                    enabled: aoSection.aoEnabled
                    textRole: "text"
                    valueRole: "value"
                    implicitContentWidthPolicy: ComboBox.WidestText
                    onActivated: toolPage.sceneEnvironment.aoSampleRate = currentValue
                    Layout.fillWidth: true

                    model: [
                        { value: 2, text: "2"},
                        { value: 3, text: "3"},
                        { value: 4, text: "4"}
                    ]

                    Connections {
                        target: toolPage.sceneEnvironment
                        function onAoSampleRateChanged() {
                            aoSampleRateComboBox.currentIndex = aoSampleRateComboBox.indexOfValue(toolPage.sceneEnvironment.aoSampleRate)
                        }
                    }
                }

                Label {
                    enabled: aoSection.aoEnabled
                    text: "Bias (" + (toolPage.sceneEnvironment.aoBias).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: aoSection.aoEnabled
                    Layout.fillWidth: true
                    from: -1.0
                    to: 1.0
                    stepSize: 0.01
                    value: toolPage.sceneEnvironment.aoBias
                    onValueChanged:
                        toolPage.sceneEnvironment.aoBias = value
                }

                CheckBox {
                    enabled: aoSection.aoEnabled
                    Layout.columnSpan: 2
                    text: "Enable AO Dither"
                    checked: toolPage.sceneEnvironment.aoDither
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.aoDither = checked
                    }
                }

                HorizontalSpacer {
                }
                ResetButton {
                    onReset: {
                        if (toolPage.sceneEnvironment.aoEnabled)
                            toolPage.sceneEnvironment.aoStrength = 100.0
                        else
                            toolPage.sceneEnvironment.aoStrength = 0.0
                        toolPage.sceneEnvironment.aoSoftness = 50.0
                        toolPage.sceneEnvironment.aoDistance = 5.0
                        toolPage.sceneEnvironment.aoSampleRate = 2
                        toolPage.sceneEnvironment.aoBias = 0.0
                        toolPage.sceneEnvironment.aoDither = false
                    }
                }
            }

            // SSGI
            SectionLayout {
                id: ssgiSection
                title: "SSGI"
                isExpanded: false

                readonly property bool ssgiEnabled: toolPage.sceneEnvironment.ssgiEnabled

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enabled"
                    checked: toolPage.sceneEnvironment.ssgiEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.ssgiEnabled = checked
                    }
                }

                Label {
                    enabled: ssgiSection.ssgiEnabled
                    text: "Buffer size factor (" + (toolPage.sceneEnvironment.ssgiBufferSizeFactor).toFixed(2) +")"
                    Layout.fillWidth: true
                }

                Slider {
                    enabled: ssgiSection.ssgiEnabled
                    Layout.fillWidth: true
                    from: 0.1
                    to: 1.0
                    value: toolPage.sceneEnvironment.ssgiBufferSizeFactor
                    onValueChanged: toolPage.sceneEnvironment.ssgiBufferSizeFactor = value
                }

                CheckBox {
                    enabled: ssgiSection.ssgiEnabled
                    Layout.columnSpan: 2
                    text: "Indirect light"
                    checked: toolPage.sceneEnvironment.ssgiIndirectLightEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.ssgiIndirectLightEnabled = checked
                    }
                }

                Label {
                    enabled: ssgiSection.ssgiEnabled && toolPage.sceneEnvironment.ssgiIndirectLightEnabled
                    text: "Indirect light boost (" + (toolPage.sceneEnvironment.ssgiIndirectLightBoost).toFixed(2) + ")"
                    Layout.fillWidth: true
                }

                Slider {
                    enabled: ssgiSection.ssgiEnabled && toolPage.sceneEnvironment.ssgiIndirectLightEnabled
                    Layout.fillWidth: true
                    from: 1.0
                    to: 100.0
                    value: toolPage.sceneEnvironment.ssgiIndirectLightBoost
                    onValueChanged: toolPage.sceneEnvironment.ssgiIndirectLightBoost = value
                }

                CheckBox {
                    enabled: ssgiSection.ssgiEnabled && toolPage.sceneEnvironment.ssgiIndirectLightEnabled
                    Layout.columnSpan: 2
                    text: "Simulated bounce"
                    checked: toolPage.sceneEnvironment.ssgiSimulatedBounceEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.ssgiSimulatedBounceEnabled = checked
                    }
                }

                Label {
                    enabled: ssgiSection.ssgiEnabled && toolPage.sceneEnvironment.ssgiIndirectLightEnabled && toolPage.sceneEnvironment.ssgiSimulatedBounceEnabled
                    text: "Simulated bounce factor (" + (toolPage.sceneEnvironment.ssgiSimulatedBounceFactor).toFixed(2) +")"
                    Layout.fillWidth: true
                }

                Slider {
                    enabled: ssgiSection.ssgiEnabled && toolPage.sceneEnvironment.ssgiIndirectLightEnabled && toolPage.sceneEnvironment.ssgiSimulatedBounceEnabled
                    Layout.fillWidth: true
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.ssgiSimulatedBounceFactor
                    onValueChanged: toolPage.sceneEnvironment.ssgiSimulatedBounceFactor = value
                }

                Label {
                    enabled: ssgiSection.ssgiEnabled
                    text: "Sample count (" + toolPage.sceneEnvironment.ssgiSampleCount +")"
                    Layout.fillWidth: true
                }

                Slider {
                    enabled: ssgiSection.ssgiEnabled
                    Layout.fillWidth: true
                    from: 1
                    to: 16
                    stepSize: 1
                    value: toolPage.sceneEnvironment.ssgiSampleCount
                    onValueChanged: toolPage.sceneEnvironment.ssgiSampleCount = value
                }

                Label {
                    enabled: ssgiSection.ssgiEnabled
                    text: "Sample radius (" + (toolPage.sceneEnvironment.ssgiSampleRadius).toFixed(2) +")"
                    Layout.fillWidth: true
                }

                Slider {
                    enabled: ssgiSection.ssgiEnabled
                    Layout.fillWidth: true
                    from: 0.001
                    to: 4.0
                    value: toolPage.sceneEnvironment.ssgiSampleRadius
                    onValueChanged: toolPage.sceneEnvironment.ssgiSampleRadius = value
                }

                Label {
                    enabled: ssgiSection.ssgiEnabled
                    text: "Slice count (" + toolPage.sceneEnvironment.ssgiSliceCount +")"
                    Layout.fillWidth: true
                }

                Slider {
                    enabled: ssgiSection.ssgiEnabled
                    Layout.fillWidth: true
                    from: 1
                    to: 8
                    stepSize: 1
                    value: toolPage.sceneEnvironment.ssgiSliceCount
                    onValueChanged: toolPage.sceneEnvironment.ssgiSliceCount = value
                }

                Label {
                    enabled: ssgiSection.ssgiEnabled
                    text: "Hit thickness (" + (toolPage.sceneEnvironment.ssgiHitThickness).toFixed(2) +")"
                    Layout.fillWidth: true
                }

                Slider {
                    enabled: ssgiSection.ssgiEnabled
                    Layout.fillWidth: true
                    from: 0.001
                    to: 4.0
                    value: toolPage.sceneEnvironment.ssgiHitThickness
                    onValueChanged: toolPage.sceneEnvironment.ssgiHitThickness = value
                }

                HorizontalSpacer {
                }
                ResetButton {
                    onReset: {
                        toolPage.sceneEnvironment.ssgiIndirectLightEnabled = true;
                        toolPage.sceneEnvironment.ssgiIndirectLightBoost = 4.0;
                        toolPage.sceneEnvironment.ssgiBufferSizeFactor = 0.5;
                        toolPage.sceneEnvironment.ssgiSimulatedBounceEnabled = false;
                        toolPage.sceneEnvironment.ssgiSimulatedBounceFactor = 0.5;
                        toolPage.sceneEnvironment.ssgiSampleCount = 4;
                        toolPage.sceneEnvironment.ssgiSampleRadius = 0.1;
                        toolPage.sceneEnvironment.ssgiSliceCount = 4;
                        toolPage.sceneEnvironment.ssgiHitThickness = 0.5;
                    }
                }
            }

            // DOF
            SectionLayout {
                id: dofSection
                title: "Depth of Field"
                isExpanded: false

                readonly property bool dofEnabled: toolPage.sceneEnvironment.depthOfFieldEnabled

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enabled"
                    checked: toolPage.sceneEnvironment.depthOfFieldEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.depthOfFieldEnabled = checked
                    }
                }

                Label {
                    enabled: dofSection.dofEnabled
                    Layout.columnSpan: 2
                    text: "Focus Distance (" + (toolPage.sceneEnvironment.depthOfFieldFocusDistance).toFixed(2) + ")"
                    Layout.fillWidth: true
                }

                Label {
                    enabled: dofSection.dofEnabled
                    Layout.columnSpan: 2
                    text: "Focus Range (" + (toolPage.sceneEnvironment.depthOfFieldFocusRange).toFixed(2) +")"
                    Layout.fillWidth: true
                }

                Label {
                    enabled: dofSection.dofEnabled
                    Layout.columnSpan: 2
                    text: "Focus Near (" + dofFocusSlider.first.value.toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Label {
                    enabled: dofSection.dofEnabled
                    Layout.columnSpan: 2
                    text: "Focus Far (" + dofFocusSlider.second.value.toFixed(2) + ")"
                    Layout.fillWidth: true
                }

                RangeSlider {
                    id: dofFocusSlider
                    enabled: dofSection.dofEnabled
                    Layout.columnSpan: 2
                    from: toolPage.camera.clipNear
                    to: toolPage.camera.clipFar
                    Layout.fillWidth: true

                    function updateSlider() {
                        const dofNear = toolPage.sceneEnvironment.depthOfFieldFocusDistance - toolPage.sceneEnvironment.depthOfFieldFocusRange * 0.5
                        const dofFar = toolPage.sceneEnvironment.depthOfFieldFocusDistance + toolPage.sceneEnvironment.depthOfFieldFocusRange * 0.5
                        second.value = dofFar
                        first.value = dofNear
                    }

                    Component.onCompleted: updateSlider()
                    first.onMoved: {
                        toolPage.sceneEnvironment.depthOfFieldFocusRange = second.value - first.value
                        toolPage.sceneEnvironment.depthOfFieldFocusDistance = first.value + toolPage.sceneEnvironment.depthOfFieldFocusRange * 0.5
                    }
                    second.onMoved: {
                        toolPage.sceneEnvironment.depthOfFieldFocusRange = second.value - first.value
                        toolPage.sceneEnvironment.depthOfFieldFocusDistance = first.value + toolPage.sceneEnvironment.depthOfFieldFocusRange * 0.5
                    }
                }


                Label {
                    enabled: dofSection.dofEnabled
                    Layout.fillWidth: true
                    text: "Blur Amount (" + (toolPage.sceneEnvironment.depthOfFieldBlurAmount).toFixed(2) + ")"
                }
                Slider {
                    enabled: dofSection.dofEnabled
                    Layout.fillWidth: true
                    from: 0.0
                    to: 25.0
                    value: toolPage.sceneEnvironment.depthOfFieldBlurAmount
                    onValueChanged:
                        toolPage.sceneEnvironment.depthOfFieldBlurAmount = value
                }
                HorizontalSpacer {
                }
                ResetButton {
                    onReset: {
                        toolPage.sceneEnvironment.depthOfFieldFocusRange = 100.0
                        toolPage.sceneEnvironment.depthOfFieldFocusDistance = 600.0
                        toolPage.sceneEnvironment.depthOfFieldBlurAmount = 4.0
                        dofFocusSlider.updateSlider()
                    }
                }

            }

            // FOG
            SectionLayout {
                id: fogSection
                title: "Fog"
                isExpanded: false

                property bool fogEnabled: toolPage.sceneEnvironment.fog.enabled

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enabled"
                    checked: toolPage.sceneEnvironment.fog.enabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.fog.enabled = checked
                    }
                }

                Label {
                    enabled: fogSection.fogEnabled
                    Layout.fillWidth: true
                    text: "Density (" + toolPage.sceneEnvironment.fog.density.toFixed(2) + ")"
                }
                Slider {
                    id: valDensity
                    enabled: fogSection.fogEnabled
                    Layout.fillWidth: true
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.fog.density
                    onValueChanged: toolPage.sceneEnvironment.fog.density = value
                }


                Label {
                    enabled: fogSection.fogEnabled
                    Layout.fillWidth: true
                    text: "Color"
                }
                ColorPicker {
                    enabled: fogSection.fogEnabled
                    Layout.fillWidth: true
                    color: toolPage.sceneEnvironment.fog.color
                    onColorChanged: toolPage.sceneEnvironment.fog.color = color
                }


                // DEPTH FOG
                CheckBox {
                    id: depthFogCheckbox
                    Layout.columnSpan: 2
                    enabled: fogSection.fogEnabled
                    Layout.fillWidth: true
                    text: "Depth fog enabled"
                    checked: toolPage.sceneEnvironment.fog.depthEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.fog.depthEnabled = checked
                    }
                }

                Label {
                    visible: depthFogCheckbox.checked
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    text: "Near (" + toolPage.sceneEnvironment.fog.depthNear.toFixed(2) + ")"
                }
                Label {
                    visible: depthFogCheckbox.checked
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    text: "Far (" + toolPage.sceneEnvironment.fog.depthFar.toFixed(2) + ")"
                }

                RangeSlider {
                    id: valDepth
                    visible: depthFogCheckbox.checked
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    from: -1000.0
                    to: 1000.0
                    first.value: toolPage.sceneEnvironment.fog.depthNear
                    second.value: toolPage.sceneEnvironment.fog.depthFar
                    first.onValueChanged: toolPage.sceneEnvironment.fog.depthNear = first.value
                    second.onValueChanged: toolPage.sceneEnvironment.fog.depthFar = second.value
                }


                Label {
                    visible: depthFogCheckbox.checked
                    Layout.fillWidth: true
                    text: "Curve (" + toolPage.sceneEnvironment.fog.depthCurve.toFixed(2) + ")"
                }
                Slider {
                    id: valDepthCurve
                    visible: depthFogCheckbox.checked
                    Layout.fillWidth: true
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.fog.depthCurve
                    onValueChanged: toolPage.sceneEnvironment.fog.depthCurve = value
                }


                // HEIGHT FOG
                CheckBox {
                    id: heightFogCheckbox
                    Layout.columnSpan: 2
                    enabled: fogSection.fogEnabled
                    text: "Height fog enabled"
                    checked: toolPage.sceneEnvironment.fog.heightEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.fog.heightEnabled = checked
                    }
                }

                Label {
                    visible: heightFogCheckbox.checked
                    Layout.fillWidth: true
                    text: "Least Intense Y (" + toolPage.sceneEnvironment.fog.leastIntenseY.toFixed(2) + ")"
                }
                Slider {
                    id: valHeightMin
                    visible: heightFogCheckbox.checked
                    Layout.fillWidth: true
                    from: -1000.0
                    to: 1000.0
                    value: toolPage.sceneEnvironment.fog.leastIntenseY
                    onValueChanged: toolPage.sceneEnvironment.fog.leastIntenseY = value
                }


                Label {
                    visible: heightFogCheckbox.checked
                    Layout.fillWidth: true
                    text: "Most Intense Y (" + toolPage.sceneEnvironment.fog.mostIntenseY.toFixed(2) + ")"
                }
                Slider {
                    id: valHeightMax
                    visible: heightFogCheckbox.checked
                    Layout.fillWidth: true
                    from: -1000.0
                    to: 1000.0
                    value: toolPage.sceneEnvironment.fog.mostIntenseY
                    onValueChanged: toolPage.sceneEnvironment.fog.mostIntenseY = value
                }


                Label {
                    visible: heightFogCheckbox.checked
                    Layout.fillWidth: true
                    text: "Curve (" + toolPage.sceneEnvironment.fog.heightCurve.toFixed(2) + ")"
                }
                Slider {
                    id: valHeightCurve
                    visible: heightFogCheckbox.checked
                    Layout.fillWidth: true
                    from: 0.0
                    to: 100.0
                    value: toolPage.sceneEnvironment.fog.heightCurve
                    onValueChanged: toolPage.sceneEnvironment.fog.heightCurve = value
                }


                // TRANSMISSION
                CheckBox {
                    id: fogTransmitCheckbox
                    Layout.columnSpan: 2
                    visible: heightFogCheckbox.checked || depthFogCheckbox.checked
                    text: "Light transmission enabled"
                    checked: toolPage.sceneEnvironment.fog.transmitEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.fog.transmitEnabled = checked
                    }
                }

                Label {
                    visible: fogTransmitCheckbox.checked
                    Layout.fillWidth: true
                    text: "Curve (" + toolPage.sceneEnvironment.fog.transmitCurve.toFixed(2) + ")"
                }
                Slider {
                    id: valTransmitCurve
                    visible: fogTransmitCheckbox.checked
                    Layout.fillWidth: true
                    from: 0.0
                    to: 100.0
                    value: toolPage.sceneEnvironment.fog.transmitCurve
                    onValueChanged: toolPage.sceneEnvironment.fog.transmitCurve = value
                }

                HorizontalSpacer {
                }
                ResetButton {
                    onReset: {
                        toolPage.sceneEnvironment.fog.depthEnabled = false
                        toolPage.sceneEnvironment.fog.heightEnabled = false
                        toolPage.sceneEnvironment.fog.transmitEnabled = false
                        toolPage.sceneEnvironment.fog.density = 1.0;
                        toolPage.sceneEnvironment.fog.color = Qt.rgba(0.5, 0.6, 0.7, 1.0);
                        toolPage.sceneEnvironment.fog.depthNear = 10.0;
                        toolPage.sceneEnvironment.fog.depthFar = 1000.0;
                        toolPage.sceneEnvironment.fog.depthCurve = 1.0;
                        toolPage.sceneEnvironment.fog.leastIntenseY = 10.0;
                        toolPage.sceneEnvironment.fog.mostIntenseY = 0.0;
                        toolPage.sceneEnvironment.fog.heightCurve = 1.0;
                        toolPage.sceneEnvironment.fog.transmitCurve = 1.0;
                    }
                }
            }

            // GLOW
            SectionLayout {
                id: glowSection
                title: "Glow"
                isExpanded: false

                property bool glowEnabled: toolPage.sceneEnvironment.glowEnabled

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enable Glow"
                    checked: toolPage.sceneEnvironment.glowEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.glowEnabled = checked
                    }
                }
                CheckBox {
                    Layout.columnSpan: 2
                    text: "High Quality Mode"
                    enabled: glowSection.glowEnabled
                    checked: toolPage.sceneEnvironment.glowQualityHigh
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.glowQualityHigh = checked
                    }
                }
                CheckBox {
                    Layout.columnSpan: 2
                    enabled: glowSection.glowEnabled
                    text: "Use Bicubic Upscale"
                    checked: toolPage.sceneEnvironment.glowUseBicubicUpscale
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.glowUseBicubicUpscale = checked
                    }
                }

                Label {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    text: "Strength (" + (toolPage.sceneEnvironment.glowStrength).toFixed(2) + ")"
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    from: 0.0
                    to: 2.0
                    value: toolPage.sceneEnvironment.glowStrength
                    onValueChanged:
                        toolPage.sceneEnvironment.glowStrength = value
                }

                Label {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    text: "Intensity (" + (toolPage.sceneEnvironment.glowIntensity).toFixed(2) + ")"
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    from: 0.0
                    to: 2.0
                    value: toolPage.sceneEnvironment.glowIntensity
                    onValueChanged:
                        toolPage.sceneEnvironment.glowIntensity = value
                }


                Label {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    text: "Bloom (" + (toolPage.sceneEnvironment.glowBloom).toFixed(2) + ")"
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.glowBloom
                    onValueChanged:
                        toolPage.sceneEnvironment.glowBloom = value
                }


                Label {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    text: "Upper Threshold (" + (toolPage.sceneEnvironment.glowHDRMaximumValue).toFixed(2) + ")"
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    from: 0.0
                    to: 256.0
                    value: toolPage.sceneEnvironment.glowHDRMaximumValue
                    onValueChanged:
                        toolPage.sceneEnvironment.glowHDRMaximumValue = value
                }


                Label {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    text: "Lower Threshold (" + (toolPage.sceneEnvironment.glowHDRMinimumValue).toFixed(2) + ")"
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    from: 0.0
                    to: 4.0
                    value: toolPage.sceneEnvironment.glowHDRMinimumValue
                    onValueChanged:
                        toolPage.sceneEnvironment.glowHDRMinimumValue = value
                }


                Label {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    text: "HDR Scale (" + (toolPage.sceneEnvironment.glowHDRScale).toFixed(2) + ")"
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    from: 0.0
                    to: 4.0
                    value: toolPage.sceneEnvironment.glowHDRScale
                    onValueChanged:
                        toolPage.sceneEnvironment.glowHDRScale = value
                }


                Label {
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    text: "Blend Mode"
                }

                ComboBox {
                    id: glowBlendModeComboBox
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                    textRole: "text"
                    valueRole: "value"
                    implicitContentWidthPolicy: ComboBox.WidestText
                    onActivated: toolPage.sceneEnvironment.glowBlendMode = currentValue
                    Component.onCompleted: currentIndex = glowBlendModeComboBox.indexOfValue(toolPage.sceneEnvironment.glowBlendMode)

                    model: [
                        { value: ExtendedSceneEnvironment.GlowBlendMode.Additive, text: "Additive"},
                        { value: ExtendedSceneEnvironment.GlowBlendMode.Screen, text: "Screen"},
                        { value: ExtendedSceneEnvironment.GlowBlendMode.SoftLight, text: "Softlight"},
                        { value: ExtendedSceneEnvironment.GlowBlendMode.Replace, text: "Replace"}
                    ]

                    Connections {
                        target: toolPage.sceneEnvironment
                        function onGlowBlendModeChanged() {
                            glowBlendModeComboBox.currentIndex = glowBlendModeComboBox.indexOfValue(toolPage.sceneEnvironment.glowBlendMode)
                        }
                    }
                }


                Label {
                    text: "Glow Level"
                    Layout.fillWidth: true
                    enabled: glowSection.glowEnabled
                }

                ColumnLayout {
                    id: glowLevelCheckBoxes
                    enabled: glowSection.glowEnabled
                    Layout.fillWidth: true
                    function updateGlowLevels(value, enable) {
                        if (enable)
                            toolPage.sceneEnvironment.glowLevel |= value
                        else
                            toolPage.sceneEnvironment.glowLevel &= ~value
                    }
                    function isChecked(flag) { return ((toolPage.sceneEnvironment.glowLevel & flag) === flag) }
                    CheckBox {
                        text: qsTr("One")
                        checked: glowLevelCheckBoxes.isChecked(ExtendedSceneEnvironment.GlowLevel.One)
                        onCheckStateChanged: {
                            glowLevelCheckBoxes.updateGlowLevels(ExtendedSceneEnvironment.GlowLevel.One, checkState === Qt.Checked)
                        }
                    }
                    CheckBox {
                        text: qsTr("Two")
                        checked: glowLevelCheckBoxes.isChecked(ExtendedSceneEnvironment.GlowLevel.Two)
                        onCheckStateChanged: {
                            glowLevelCheckBoxes.updateGlowLevels(ExtendedSceneEnvironment.GlowLevel.Two, checkState === Qt.Checked)
                        }
                    }
                    CheckBox {
                        text: qsTr("Three")
                        checked: glowLevelCheckBoxes.isChecked(ExtendedSceneEnvironment.GlowLevel.Three)
                        onCheckStateChanged: {
                            glowLevelCheckBoxes.updateGlowLevels(ExtendedSceneEnvironment.GlowLevel.Three, checkState === Qt.Checked)
                        }
                    }
                    CheckBox {
                        text: qsTr("Four")
                        checked: glowLevelCheckBoxes.isChecked(ExtendedSceneEnvironment.GlowLevel.Four)
                        onCheckStateChanged: {
                            glowLevelCheckBoxes.updateGlowLevels(ExtendedSceneEnvironment.GlowLevel.Four, checkState === Qt.Checked)
                        }
                    }
                    CheckBox {
                        text: qsTr("Five")
                        checked: glowLevelCheckBoxes.isChecked(ExtendedSceneEnvironment.GlowLevel.Five)
                        onCheckStateChanged: {
                            glowLevelCheckBoxes.updateGlowLevels(ExtendedSceneEnvironment.GlowLevel.Five, checkState === Qt.Checked)
                        }
                    }
                    CheckBox {
                        text: qsTr("Six")
                        checked: glowLevelCheckBoxes.isChecked(ExtendedSceneEnvironment.GlowLevel.Six)
                        onCheckStateChanged: {
                            glowLevelCheckBoxes.updateGlowLevels(ExtendedSceneEnvironment.GlowLevel.Six, checkState === Qt.Checked)
                        }
                    }
                    CheckBox {
                        text: qsTr("Seven")
                        checked: glowLevelCheckBoxes.isChecked(ExtendedSceneEnvironment.GlowLevel.Seven)
                        onCheckStateChanged: {
                            glowLevelCheckBoxes.updateGlowLevels(ExtendedSceneEnvironment.GlowLevel.Seven, checkState === Qt.Checked)
                        }
                    }
                }

                HorizontalSpacer {
                }
                ResetButton {
                    onReset: {
                        toolPage.sceneEnvironment.glowQualityHigh = false
                        toolPage.sceneEnvironment.glowUseBicubicUpscale = false
                        toolPage.sceneEnvironment.glowStrength = 1.0
                        toolPage.sceneEnvironment.glowIntensity = 0.8
                        toolPage.sceneEnvironment.glowBloom = 0.0
                        toolPage.sceneEnvironment.glowBlendMode = 2
                        toolPage.sceneEnvironment.glowHDRMaximumValue = 12.0
                        toolPage.sceneEnvironment.glowHDRScale = 2.0
                        toolPage.sceneEnvironment.glowHDRMinimumValue = 1.0
                        toolPage.sceneEnvironment.glowLevel = 1
                    }
                }

            }

            // ADJUSTMENT
            SectionLayout {
                id: adjustmentsSection
                title: "Adjustments"
                isExpanded: false

                property bool colorAdjustmentsEnabled: toolPage.sceneEnvironment.colorAdjustmentsEnabled

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enable Color Adjustments"
                    checked: toolPage.sceneEnvironment.colorAdjustmentsEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.colorAdjustmentsEnabled = checked
                    }
                }

                Label {
                    enabled: adjustmentsSection.colorAdjustmentsEnabled
                    text: "Brightness (" + (toolPage.sceneEnvironment.adjustmentBrightness).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: adjustmentsSection.colorAdjustmentsEnabled
                    Layout.fillWidth: true
                    from: 0.01
                    to: 8.0
                    stepSize: 0.01
                    value: toolPage.sceneEnvironment.adjustmentBrightness
                    onValueChanged:
                        toolPage.sceneEnvironment.adjustmentBrightness = value
                }


                Label {
                    enabled: adjustmentsSection.colorAdjustmentsEnabled
                    text: "Contrast (" + (toolPage.sceneEnvironment.adjustmentContrast).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: adjustmentsSection.colorAdjustmentsEnabled
                    Layout.fillWidth: true
                    from: 0.01
                    to: 8.0
                    stepSize: 0.01
                    value: toolPage.sceneEnvironment.adjustmentContrast
                    onValueChanged:
                        toolPage.sceneEnvironment.adjustmentContrast = value
                }


                Label {
                    enabled: adjustmentsSection.colorAdjustmentsEnabled
                    text: "Saturation (" + (toolPage.sceneEnvironment.adjustmentSaturation).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: adjustmentsSection.colorAdjustmentsEnabled
                    Layout.fillWidth: true
                    from: 0.01
                    to: 8.0
                    stepSize: 0.01
                    value: toolPage.sceneEnvironment.adjustmentSaturation
                    onValueChanged:
                        toolPage.sceneEnvironment.adjustmentSaturation = value
                }

                HorizontalSpacer {
                }
                ResetButton {
                    onReset: {
                        toolPage.sceneEnvironment.adjustmentBrightness = 1.0
                        toolPage.sceneEnvironment.adjustmentContrast = 1.0
                        toolPage.sceneEnvironment.adjustmentSaturation = 1.0
                    }
                }
            }

            // COLORGRADING
            SectionLayout {
                id: colorGradingSection
                title: "Color Grading"
                isExpanded: false

                property bool colorGradingEnabled: toolPage.sceneEnvironment.lutEnabled

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enable"
                    checked: toolPage.sceneEnvironment.lutEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.lutEnabled = checked
                    }
                }

                Label {
                    enabled: colorGradingSection.colorGradingEnabled
                    text: "Alpha Filtering (" + (toolPage.sceneEnvironment.lutFilterAlpha).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: colorGradingSection.colorGradingEnabled
                    Layout.fillWidth: true
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.lutFilterAlpha
                    onValueChanged:
                        toolPage.sceneEnvironment.lutFilterAlpha = value
                }

                Label {
                    enabled: colorGradingSection.colorGradingEnabled
                    text: "Look Up Texture (LUT)"
                    Layout.fillWidth: true
                }

                ComboBox {
                    id: lutSourceTextureComboBox
                    enabled: colorGradingSection.colorGradingEnabled
                    Layout.fillWidth: true
                    textRole: "text"
                    valueRole: "value"
                    implicitContentWidthPolicy: ComboBox.WidestText
                    onActivated: toolPage.lutTexture.source = currentValue
                    Component.onCompleted: lutSourceTextureComboBox.currentIndex = lutSourceTextureComboBox.indexOfValue(toolPage.lutTexture.source)
                    model: [
                        { value: Qt.url("qrc:/luts/grayscale.png"), text: "Greyscale"},
                        { value: Qt.url("qrc:/luts/identity.png"), text: "Identity"},
                        { value: Qt.url("qrc:/luts/inverted.png"), text: "Inverted"}
                    ]

                    Connections {
                        target: toolPage.lutTexture
                        function onSourceChanged() {
                            lutSourceTextureComboBox.currentIndex = lutSourceTextureComboBox.indexOfValue(toolPage.lutTexture.source)
                        }
                    }
                }

                HorizontalSpacer {
                }
                ResetButton {
                    onReset: {
                        toolPage.sceneEnvironment.lutFilterAlpha = 1.0;
                        toolPage.lutTexture.source = Qt.url("qrc:/luts/identity.png");
                    }
                }
            }

            // VIGNETTE
            SectionLayout {
                id: vignetteSection
                title: "Vignette"
                isExpanded: false

                property bool vignetteEnabled: toolPage.sceneEnvironment.vignetteEnabled

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enable"
                    checked: toolPage.sceneEnvironment.vignetteEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.vignetteEnabled = checked
                    }
                }


                Label {
                    enabled: vignetteSection.vignetteEnabled
                    text: "Vignette Strength(" + (toolPage.sceneEnvironment.vignetteStrength).toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: vignetteSection.vignetteEnabled
                    Layout.fillWidth: true
                    from: 0.0
                    to: 15.0
                    value: toolPage.sceneEnvironment.vignetteStrength
                    onValueChanged:
                        toolPage.sceneEnvironment.vignetteStrength = value
                }


                Label {
                    text: "Vignette Color"
                    enabled: vignetteSection.vignetteEnabled
                    Layout.fillWidth: true
                }
                ColorPicker {
                    enabled: vignetteSection.vignetteEnabled
                    Layout.fillWidth: true
                    color: toolPage.sceneEnvironment.vignetteColor
                    onColorChanged:
                        toolPage.sceneEnvironment.vignetteColor = color
                }

                Label {
                    text: "Vignette Radius(" + (toolPage.sceneEnvironment.vignetteRadius).toFixed(2) + ")"
                    enabled: vignetteSection.vignetteEnabled
                    Layout.fillWidth: true
                }
                Slider {
                    enabled: vignetteSection.vignetteEnabled
                    Layout.fillWidth: true
                    from: 0.0
                    to: 5.0
                    value: toolPage.sceneEnvironment.vignetteRadius
                    onValueChanged:
                        toolPage.sceneEnvironment.vignetteRadius = value
                }

                HorizontalSpacer {
                }
                ResetButton {
                    onReset: {
                        toolPage.sceneEnvironment.vignetteStrength = 15.0
                        toolPage.sceneEnvironment.vignetteColor = Qt.rgba(0.5, 0.5, 0.5, 1.0)
                        toolPage.sceneEnvironment.vignetteRadius = 0.35
                    }
                }
            }

            // LENSFLARE
            SectionLayout {
                id: lensFlareSection
                title: "Lens Flare"
                isExpanded: false

                property bool lensFlareEnabled: toolPage.sceneEnvironment.lensFlareEnabled

                CheckBox {
                    Layout.columnSpan: 2
                    text: "Enable"
                    checked: toolPage.sceneEnvironment.lensFlareEnabled
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.lensFlareEnabled = checked
                    }
                }

                Label {
                    text: "Bloom Scale(" + (toolPage.sceneEnvironment.lensFlareBloomScale).toFixed(2) + ")"
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                    from: 0.0
                    to: 20.0
                    value: toolPage.sceneEnvironment.lensFlareBloomScale
                    onValueChanged:
                        toolPage.sceneEnvironment.lensFlareBloomScale = value
                }


                Label {
                    text: "Bloom Bias(" + (toolPage.sceneEnvironment.lensFlareBloomBias).toFixed(2) + ")"
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                    from: 0.0
                    to: 10.0
                    value: toolPage.sceneEnvironment.lensFlareBloomBias
                    onValueChanged:
                        toolPage.sceneEnvironment.lensFlareBloomBias = value
                }

                Label {
                    text: "Ghost Dispersal(" + (toolPage.sceneEnvironment.lensFlareGhostDispersal).toFixed(2) + ")"
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.lensFlareGhostDispersal
                    onValueChanged:
                        toolPage.sceneEnvironment.lensFlareGhostDispersal = value
                }


                Label {
                    text: "Ghost Count(" + (toolPage.sceneEnvironment.lensFlareGhostCount) + ")"
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                    from: 0
                    to: 20
                    stepSize: 1
                    value: toolPage.sceneEnvironment.lensFlareGhostCount
                    onValueChanged:
                        toolPage.sceneEnvironment.lensFlareGhostCount = value
                }


                Label {
                    text: "Halo Width(" + (toolPage.sceneEnvironment.lensFlareHaloWidth).toFixed(2) + ")"
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.lensFlareHaloWidth
                    onValueChanged:
                        toolPage.sceneEnvironment.lensFlareHaloWidth = value
                }


                Label {
                    text: "Stretch Aspect(" + (toolPage.sceneEnvironment.lensFlareStretchToAspect).toFixed(2) + ")"
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                    from: 0.0
                    to: 1.0
                    value: toolPage.sceneEnvironment.lensFlareStretchToAspect
                    onValueChanged:
                        toolPage.sceneEnvironment.lensFlareStretchToAspect = value
                }


                Label {
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                    text: "Distortion(" + (toolPage.sceneEnvironment.lensFlareDistortion).toFixed(2) + ")"
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                    from: 0.0
                    to: 20.0
                    value: toolPage.sceneEnvironment.lensFlareDistortion
                    onValueChanged:
                        toolPage.sceneEnvironment.lensFlareDistortion = value
                }


                Label {
                    text: "Blur Amount(" + (toolPage.sceneEnvironment.lensFlareBlurAmount).toFixed(2) + ")"
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                }
                Slider {
                    Layout.fillWidth: true
                    enabled: lensFlareSection.lensFlareEnabled
                    from: 0.0
                    to: 50.0
                    value: toolPage.sceneEnvironment.lensFlareBlurAmount
                    onValueChanged:
                        toolPage.sceneEnvironment.lensFlareBlurAmount = value
                }

                CheckBox {
                    enabled: lensFlareSection.lensFlareEnabled
                    Layout.columnSpan: 2
                    text: "Apply Lens Dirt"
                    checked: toolPage.sceneEnvironment.lensFlareApplyDirtTexture
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.lensFlareApplyDirtTexture = checked
                    }
                }
                CheckBox {
                    enabled: lensFlareSection.lensFlareEnabled
                    Layout.columnSpan: 2
                    text: "Apply Starburst"
                    checked: toolPage.sceneEnvironment.lensFlareApplyStarburstTexture
                    onCheckedChanged: {
                        toolPage.sceneEnvironment.lensFlareApplyStarburstTexture = checked
                    }
                }
                HorizontalSpacer {
                }
                ResetButton {
                    onReset: {
                        toolPage.sceneEnvironment.lensFlareBloomScale = 10
                        toolPage.sceneEnvironment.lensFlareBloomBias = 2.75
                        toolPage.sceneEnvironment.lensFlareGhostDispersal = 0.5
                        toolPage.sceneEnvironment.lensFlareGhostCount = 4
                        toolPage.sceneEnvironment.lensFlareHaloWidth = 0.25
                        toolPage.sceneEnvironment.lensFlareStretchToAspect = 0.5
                        toolPage.sceneEnvironment.lensFlareDistortion = 5
                        toolPage.sceneEnvironment.lensFlareBlurAmount = 3
                        toolPage.sceneEnvironment.lensFlareApplyDirtTexture = true
                        toolPage.sceneEnvironment.lensFlareApplyStarburstTexture = true
                    }
                }
            }
        }
    }
}
