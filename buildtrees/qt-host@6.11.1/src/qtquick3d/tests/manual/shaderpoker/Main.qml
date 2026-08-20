// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: 1280
    height: 720
    visible: true
    title: qsTr("Poke-A-Shader")

    component HorizontalSpacer : Item {
        Layout.fillWidth: true
    }

    // Done as a JS Object to work around some V4 engine bug
    property var featureHelper: {
        "feature" : {
            "LightProbe" : (1 << 8),
            "IblOrientation" : (1 << 9) + 1,
            "Ssm" : (1 << 10) + 2,
            "Ssao" : (1 << 11) + 3,
            "DepthPass" : (1 << 12) + 4,
            "OrthoShadowPass" : (1 << 13) + 5,
            "PerspectiveShadowPass" : (1 << 14) + 6,
            "LinearTonemapping" : (1 << 15) + 7,
            "AcesTonemapping" : (1 << 16) + 8,
            "HejlDawsonTonemapping" : (1 << 17) + 9,
            "FilmicTonemapping" : (1 << 18) + 10,
            "RGBELightProbe" : (1 << 19) + 11,
            "OpaqueDepthPrePass" : (1 << 20) + 12,
            "ReflectionProbe" : (1 << 21) + 13,
            "ReduceMaxNumLights" : (1 << 22) + 14,
            "Lightmap" : (1 << 23) + 15,
            "DisableMultiView" : (1 << 24) + 16,
            "ForceIblExposure" : (1 << 25) + 17,
            "NormalPass" : (1 << 26) + 18,
        },
        "indexMask" : 0x000000FF,

        "isSet" : function(featureName) {
            return shaderPoker.featureBitfield & this.feature[featureName]
        },

        "setFeature" : function (featureName, value) {
            if (value) {
                shaderPoker.featureBitfield |= (this.feature[featureName] & ~this.indexMask)
            } else {
                shaderPoker.featureBitfield &= ~(this.feature[featureName] & ~this.IndexMask)
            }
        }
    }

    ShaderPoker {
        id: shaderPoker
        materialType: ShaderPoker.MaterialType.Principled
        shaderIndex: shaderVersionListView.currentIndex
    }

    SplitView {
        id: editorViews
        anchors.fill: parent
        orientation: Qt.Horizontal

        Page {
            id: settingsPage
            SplitView.preferredWidth: editorViews.width * 0.2
            title: "Settings"
            header: ToolBar {
                width: parent.width
                Label {
                    width: parent.width
                    text: settingsPage.title
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 20
                }
            }
            ScrollView {
                anchors.fill: parent
                contentHeight: editableSections.implicitHeight

                ColumnLayout {
                    id: editableSections
                    width: parent.width
                    spacing: 0

                    SectionLayout {
                        id: shaderSection
                        title: "Shader"

                        ListView {
                            id: shaderVersionListView
                            clip: true
                            Layout.fillWidth: true
                            height: 100
                            model: shaderPoker.availableShadersList
                            delegate: ItemDelegate {
                                required property string modelData
                                required property int index
                                text: modelData
                                highlighted: ListView.isCurrentItem
                                onClicked: shaderVersionListView.currentIndex = index
                            }
                            ScrollIndicator.vertical: ScrollIndicator { }
                        }
                    }

                    SectionLayout {
                        id: featureSection
                        title: "Features"

                        CheckBox {
                            id: lightProbeFeatureCheckbox
                            text: "LightProbe"
                            checked: featureHelper.isSet("LightProbe")
                            onCheckedChanged: {
                                featureHelper.setFeature("LightProbe", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: iblOrientationFeatureCheckbox
                            text: "IblOrientation"
                            checked: featureHelper.isSet("IblOrientation")
                            onCheckedChanged: {
                                featureHelper.setFeature("IblOrientation", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: ssmFeatureCheckbox
                            text: "Ssm"
                            checked: featureHelper.isSet("Ssm")
                            onCheckedChanged: {
                                featureHelper.setFeature("Ssm", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: ssaoFeatureCheckbox
                            text: "Ssao"
                            checked: featureHelper.isSet("Ssao")
                            onCheckedChanged: {
                                featureHelper.setFeature("Ssao", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: depthPassFeatureCheckbox
                            text: "DepthPass"
                            checked: featureHelper.isSet("DepthPass")
                            onCheckedChanged: {
                                featureHelper.setFeature("DepthPass", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: orthoShadowPassFeatureCheckbox
                            text: "OrthoShadowPass"
                            checked: featureHelper.isSet("OrthoShadowPass")
                            onCheckedChanged: {
                                featureHelper.setFeature("OrthoShadowPass", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: perspectiveShadowPassFeatureCheckbox
                            text: "PerspectiveShadowPass"
                            checked: featureHelper.isSet("PerspectiveShadowPass")
                            onCheckedChanged: {
                                featureHelper.setFeature("PerspectiveShadowPass", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: linearTonemappingFeatureCheckbox
                            text: "LinearTonemapping"
                            checked: featureHelper.isSet("LinearTonemapping")
                            onCheckedChanged: {
                                featureHelper.setFeature("LinearTonemapping", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: acesTonemappingFeatureCheckbox
                            text: "AcesTonemapping"
                            checked: featureHelper.isSet("AcesTonemapping")
                            onCheckedChanged: {
                                featureHelper.setFeature("AcesTonemapping", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: hejlDawsonTonemappingFeatureCheckbox
                            text: "HejlDawsonTonemapping"
                            checked: featureHelper.isSet("HejlDawsonTonemapping")
                            onCheckedChanged: {
                                featureHelper.setFeature("HejlDawsonTonemapping", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: filmicTonemappingFeatureCheckbox
                            text: "FilmicTonemapping"
                            checked: featureHelper.isSet("FilmicTonemapping")
                            onCheckedChanged: {
                                featureHelper.setFeature("FilmicTonemapping", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: rgbeLightProbeFeatureCheckbox
                            text: "RGBELightProbe"
                            checked: featureHelper.isSet("RGBELightProbe")
                            onCheckedChanged: {
                                featureHelper.setFeature("RGBELightProbe", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: opaqueDepthPrePassFeatureCheckbox
                            text: "OpaqueDepthPrePass"
                            checked: featureHelper.isSet("OpaqueDepthPrePass")
                            onCheckedChanged: {
                                featureHelper.setFeature("OpaqueDepthPrePass", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: reflectionProbeFeatureCheckbox
                            text: "ReflectionProbe"
                            checked: featureHelper.isSet("ReflectionProbe")
                            onCheckedChanged: {
                                featureHelper.setFeature("ReflectionProbe", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: reducedMaxNumLightsFeatureCheckbox
                            text: "ReduceMaxNumLights"
                            checked: featureHelper.isSet("ReduceMaxNumLights")
                            onCheckedChanged: {
                                featureHelper.setFeature("ReduceMaxNumLights", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: lightmapFeatureCheckbox
                            text: "Lightmap"
                            checked: featureHelper.isSet("Lightmap")
                            onCheckedChanged: {
                                featureHelper.setFeature("Lightmap", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: disableMultiViewFeatureCheckbox
                            text: "DisableMultiView"
                            checked: featureHelper.isSet("DisableMultiView")
                            onCheckedChanged: {
                                featureHelper.setFeature("DisableMultiView", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: forceIblExposureFeatureCheckbox
                            text: "ForceIblExposure"
                            checked: featureHelper.isSet("ForceIblExposure")
                            onCheckedChanged: {
                                featureHelper.setFeature("ForceIblExposure", checked)
                            }
                        }
                        HorizontalSpacer {}

                        CheckBox {
                            id: normalPassFeatureCheckbox
                            text: "NormalPass"
                            checked: featureHelper.isSet("NormalPass")
                            onCheckedChanged: {
                                featureHelper.setFeature("NormalPass", checked)
                            }
                        }
                        HorizontalSpacer {}

                        Button {
                            text: "Clear Features"
                            onClicked: shaderPoker.featureBitfield = 0
                        }

                    }

                    SectionLayout {
                        id: propertiesSection
                        title: "Properties"

                        CheckBox {
                            id: hasLightingCheckbox
                            Layout.columnSpan: 2
                            text: "hasLighting"
                            checked: shaderPoker.shaderProperties.hasLighting
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.hasLighting = checked
                            }
                        }

                        CheckBox {
                            id: hasPunctualLightsCheckbox
                            Layout.columnSpan: 2
                            text: "hasPunctualLights"
                            checked: shaderPoker.shaderProperties.hasPunctualLights
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.hasPunctualLights = checked
                            }
                        }

                        CheckBox {
                            id: hasShadowsCheckbox
                            Layout.columnSpan: 2
                            text: "hasShadows"
                            checked: shaderPoker.shaderProperties.hasShadows
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.hasShadows = checked
                            }
                        }

                        CheckBox {
                            id: hasIblCheckbox
                            Layout.columnSpan: 2
                            text: "hasIBL"
                            checked: shaderPoker.shaderProperties.hasIbl
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.hasIbl = checked
                            }
                        }


                        CheckBox {
                            id: specularEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "specularEnabled"
                            checked: shaderPoker.shaderProperties.specularEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.specularEnabled = checked
                            }
                        }

                        CheckBox {
                            id: fresnelScaleBiasEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "fresnelScaleBiasEnabled"
                            checked: shaderPoker.shaderProperties.fresnelScaleBiasEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.fresnelScaleBiasEnabled = checked
                            }
                        }

                        CheckBox {
                            id: clearcoatFresnelScaleBiasEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "clearcoatFresnelScaleBiasEnabled"
                            checked: shaderPoker.shaderProperties.clearcoatFresnelScaleBiasEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.clearcoatFresnelScaleBiasEnabled = checked
                            }
                        }

                        CheckBox {
                            id: fresnelEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "fresnelEnabled"
                            checked: shaderPoker.shaderProperties.fresnelEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.fresnelEnabled = checked
                            }
                        }

                        CheckBox {
                            id: baseColorSingleChannelEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "baseColorSingleChannelEnabled"
                            checked: shaderPoker.shaderProperties.baseColorSingleChannelEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.baseColorSingleChannelEnabled = checked
                            }
                        }

                        CheckBox {
                            id: specularSingleChannelEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "specularSingleChannelEnabled"
                            checked: shaderPoker.shaderProperties.specularSingleChannelEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.specularSingleChannelEnabled = checked
                            }
                        }

                        CheckBox {
                            id: emissiveSingleChannelEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "emissiveSingleChannelEnabled"
                            checked: shaderPoker.shaderProperties.emissiveSingleChannelEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.emissiveSingleChannelEnabled = checked
                            }
                        }

                        CheckBox {
                            id: invertOpacityMapValueCheckbox
                            Layout.columnSpan: 2
                            text: "invertOpacityMapValue"
                            checked: shaderPoker.shaderProperties.invertOpacityMapValue
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.invertOpacityMapValue = checked
                            }
                        }

                        CheckBox {
                            id: vertexColorsEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "vertexColorsEnabled"
                            checked: shaderPoker.shaderProperties.vertexColorsEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.vertexColorsEnabled = checked
                            }
                        }

                        CheckBox {
                            id: vertexColorsMaskEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "vertexColorsMaskEnabled"
                            checked: shaderPoker.shaderProperties.vertexColorsMaskEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.vertexColorsMaskEnabled = checked
                            }
                        }

                        VertexColorMaskEditor {
                            id: vertexColorRedMaskGroupBox
                            width: parent.availableWidth
                            isExpanded: vertexColorsMaskEnabledCheckbox.checked
                            title: "Vertex Color Red Mask"
                            Layout.columnSpan: 2
                            vertexColorMask: shaderPoker.shaderProperties.vertexColorRedMask
                            onVertexColorMaskModified: (value) => {
                                shaderPoker.shaderProperties.vertexColorRedMask = value
                            }
                        }

                        VertexColorMaskEditor {
                            id: vertexColorGreenMaskGroupBox
                            width: parent.availableWidth
                            isExpanded: vertexColorsMaskEnabledCheckbox.checked
                            title: "Vertex Color Green Mask"
                            Layout.columnSpan: 2
                            vertexColorMask: shaderPoker.shaderProperties.vertexColorGreenMask
                            onVertexColorMaskModified: (value) => {
                                shaderPoker.shaderProperties.vertexColorGreenMask = value
                            }
                        }

                        VertexColorMaskEditor {
                            id: vertexColorBlueMaskGroupBox
                            width: parent.availableWidth
                            isExpanded: vertexColorsMaskEnabledCheckbox.checked
                            title: "Vertex Color Blue Mask"
                            Layout.columnSpan: 2
                            vertexColorMask: shaderPoker.shaderProperties.vertexColorBlueMask
                            onVertexColorMaskModified: (value) => {
                                shaderPoker.shaderProperties.vertexColorBlueMask = value
                            }
                        }

                        VertexColorMaskEditor {
                            id: vertexColorAlphaMaskGroupBox
                            width: parent.availableWidth
                            isExpanded: vertexColorsMaskEnabledCheckbox.checked
                            title: "Vertex Color Alpha Mask"
                            Layout.columnSpan: 2
                            vertexColorMask: shaderPoker.shaderProperties.vertexColorAlphaMask
                            onVertexColorMaskModified: (value) => {
                                shaderPoker.shaderProperties.vertexColorAlphaMask = value
                            }
                        }

                        Label {
                            text: "Specular Model"
                        }

                        ComboBox {
                            id: specularModelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.specularModel = currentValue
                            Component.onCompleted: specularModelComboBox.currentIndex = specularModelComboBox.indexOfValue(shaderPoker.shaderProperties.specularModel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "BlinnPhong"},
                                { value: 1, text: "SchlickGGX"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onSpecularModelChanged() {
                                    specularModelComboBox.currentIndex = specularModelComboBox.indexOfValue(shaderPoker.shaderProperties.specularModel)
                                }
                            }
                        }

                        Label {
                            text: "Diffuse Model"
                        }

                        ComboBox {
                            id: diffuseModelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.diffuseModel = currentValue
                            Component.onCompleted: diffuseModelComboBox.currentIndex = diffuseModelComboBox.indexOfValue(shaderPoker.shaderProperties.diffuseModel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "Burley"},
                                { value: 1, text: "Lambert"},
                                { value: 2, text: "LambertWrap"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onDiffuseModelChanged() {
                                    diffuseModelComboBox.currentIndex = diffuseModelComboBox.indexOfValue(shaderPoker.shaderProperties.diffuseModel)
                                }
                            }
                        }

                        ImageMapEditor {
                            id: diffuseMapEditor
                            Layout.columnSpan: 2
                            title: "Diffuse Map"
                            imageMapProperties: shaderPoker.shaderProperties.diffuseMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.diffuseMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: emissiveMapEditor
                            Layout.columnSpan: 2
                            title: "Emissive Map"
                            imageMapProperties: shaderPoker.shaderProperties.emissiveMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.emissiveMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: specularMapEditor
                            Layout.columnSpan: 2
                            title: "Specular Map"
                            imageMapProperties: shaderPoker.shaderProperties.specularMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.specularMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: baseColorMapEditor
                            Layout.columnSpan: 2
                            title: "Base Color Map"
                            imageMapProperties: shaderPoker.shaderProperties.baseColorMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.baseColorMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: bumpMapEditor
                            Layout.columnSpan: 2
                            title: "Bump Map"
                            imageMapProperties: shaderPoker.shaderProperties.bumpMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.bumpMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: specularAmountMapEditor
                            Layout.columnSpan: 2
                            title: "Specular Amount Map"
                            imageMapProperties: shaderPoker.shaderProperties.specularAmountMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.specularAmountMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: normalMapEditor
                            Layout.columnSpan: 2
                            title: "Normal Map"
                            imageMapProperties: shaderPoker.shaderProperties.normalMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.normalMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: clearcoatNormalMapEditor
                            Layout.columnSpan: 2
                            title: "Clearcoat Normal Map"
                            imageMapProperties: shaderPoker.shaderProperties.clearcoatNormalMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.clearcoatNormalMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: opacityMapEditor
                            Layout.columnSpan: 2
                            title: "Opacity Map"
                            imageMapProperties: shaderPoker.shaderProperties.opacityMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.opacityMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: roughnessMapEditor
                            Layout.columnSpan: 2
                            title: "Roughness Map"
                            imageMapProperties: shaderPoker.shaderProperties.roughnessMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.roughnessMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: metalnessMapEditor
                            Layout.columnSpan: 2
                            title: "Metalness Map"
                            imageMapProperties: shaderPoker.shaderProperties.metalnessMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.metalnessMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: occlusionMapEditor
                            Layout.columnSpan: 2
                            title: "Occlusion Map"
                            imageMapProperties: shaderPoker.shaderProperties.occlusionMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.occlusionMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: translucencyMapEditor
                            Layout.columnSpan: 2
                            title: "Translucency Map"
                            imageMapProperties: shaderPoker.shaderProperties.translucencyMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.translucencyMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: heightMapEditor
                            Layout.columnSpan: 2
                            title: "Height Map"
                            imageMapProperties: shaderPoker.shaderProperties.heightMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.heightMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: clearcoatMapEditor
                            Layout.columnSpan: 2
                            title: "Clearcoat Map"
                            imageMapProperties: shaderPoker.shaderProperties.clearcoatMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.clearcoatMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: clearcoatRoughnessMapEditor
                            Layout.columnSpan: 2
                            title: "Clearcoat Roughness Map"
                            imageMapProperties: shaderPoker.shaderProperties.clearcoatRoughnessMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.clearcoatRoughnessMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: transmissionMapEditor
                            Layout.columnSpan: 2
                            title: "Transmission Map"
                            imageMapProperties: shaderPoker.shaderProperties.transmissionMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.transmissionMap = newValue
                            }
                        }

                        ImageMapEditor {
                            id: thicknessMapEditor
                            Layout.columnSpan: 2
                            title: "Thickness Map"
                            imageMapProperties: shaderPoker.shaderProperties.thicknessMap
                            onImageMapPropertiesModified: (newValue) => {
                                shaderPoker.shaderProperties.thicknessMap = newValue
                            }
                        }

                        Label {
                            text: "Opacity Channel"
                        }

                        ComboBox {
                            id: opacityChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.opacityChannel = currentValue
                            Component.onCompleted: opacityChannelComboBox.currentIndex = opacityChannelComboBox.indexOfValue(shaderPoker.shaderProperties.opacityChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onOpacityChannelChanged() {
                                    opacityChannelComboBox.currentIndex = opacityChannelComboBox.indexOfValue(shaderPoker.shaderProperties.opacityChannel)
                                }
                            }
                        }


                        Label {
                            text: "Roughness Channel"
                        }

                        ComboBox {
                            id: roughnessChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.roughnessChannel = currentValue
                            Component.onCompleted: roughnessChannelComboBox.currentIndex = roughnessChannelComboBox.indexOfValue(shaderPoker.shaderProperties.roughnessChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onRoughnessChannelChanged() {
                                    roughnessChannelComboBox.currentIndex = roughnessChannelComboBox.indexOfValue(shaderPoker.shaderProperties.roughnessChannel)
                                }
                            }
                        }

                        Label {
                            text: "Metalness Channel"
                        }

                        ComboBox {
                            id: metalnessChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.metalnessChannel = currentValue
                            Component.onCompleted: metalnessChannelComboBox.currentIndex = metalnessChannelComboBox.indexOfValue(shaderPoker.shaderProperties.metalnessChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onMetalnessChannelChanged() {
                                    metalnessChannelComboBox.currentIndex = metalnessChannelComboBox.indexOfValue(shaderPoker.shaderProperties.metalnessChannel)
                                }
                            }
                        }

                        Label {
                            text: "Occlusion Channel"
                        }

                        ComboBox {
                            id: occlusionChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.occlusionChannel = currentValue
                            Component.onCompleted: occlusionChannelComboBox.currentIndex = occlusionChannelComboBox.indexOfValue(shaderPoker.shaderProperties.occlusionChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onOcclusionChannelChanged() {
                                    occlusionChannelComboBox.currentIndex = occlusionChannelComboBox.indexOfValue(shaderPoker.shaderProperties.occlusionChannel)
                                }
                            }
                        }

                        Label {
                            text: "Translucency Channel"
                        }

                        ComboBox {
                            id: translucencyChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.translucencyChannel = currentValue
                            Component.onCompleted: translucencyChannelComboBox.currentIndex = translucencyChannelComboBox.indexOfValue(shaderPoker.shaderProperties.translucencyChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onTranslucencyChannelChanged() {
                                    translucencyChannelComboBox.currentIndex = translucencyChannelComboBox.indexOfValue(shaderPoker.shaderProperties.translucencyChannel)
                                }
                            }
                        }

                        Label {
                            text: "Height Channel"
                        }

                        ComboBox {
                            id: heightChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.heightChannel = currentValue
                            Component.onCompleted: heightChannelComboBox.currentIndex = heightChannelComboBox.indexOfValue(shaderPoker.shaderProperties.heightChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onHeightChannelChanged() {
                                    heightChannelComboBox.currentIndex = heightChannelComboBox.indexOfValue(shaderPoker.shaderProperties.heightChannel)
                                }
                            }
                        }

                        Label {
                            text: "Clearcoat Channel"
                        }

                        ComboBox {
                            id: clearcoatChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.clearcoatChannel = currentValue
                            Component.onCompleted: clearcoatChannelComboBox.currentIndex = clearcoatChannelComboBox.indexOfValue(shaderPoker.shaderProperties.clearcoatChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onClearcoatChannelChanged() {
                                    clearcoatChannelComboBox.currentIndex = clearcoatChannelComboBox.indexOfValue(shaderPoker.shaderProperties.clearcoatChannel)
                                }
                            }
                        }

                        Label {
                            text: "Clearcoat Roughness Channel"
                        }

                        ComboBox {
                            id: clearcoatRoughnessChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.clearcoatRoughnessChannel = currentValue
                            Component.onCompleted: clearcoatRoughnessChannelComboBox.currentIndex = clearcoatRoughnessChannelComboBox.indexOfValue(shaderPoker.shaderProperties.clearcoatRoughnessChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onClearcoatRoughnessChannelChanged() {
                                    clearcoatRoughnessChannelComboBox.currentIndex = clearcoatRoughnessChannelComboBox.indexOfValue(shaderPoker.shaderProperties.clearcoatRoughnessChannel)
                                }
                            }
                        }

                        Label {
                            text: "Transmission Channel"
                        }

                        ComboBox {
                            id: transmissionChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.transmissionChannel = currentValue
                            Component.onCompleted: transmissionChannelComboBox.currentIndex = transmissionChannelComboBox.indexOfValue(shaderPoker.shaderProperties.transmissionChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onTransmissionChannelChanged() {
                                    transmissionChannelComboBox.currentIndex = transmissionChannelComboBox.indexOfValue(shaderPoker.shaderProperties.transmissionChannel)
                                }
                            }
                        }

                        Label {
                            text: "Thickness Channel"
                        }

                        ComboBox {
                            id: thicknessChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.thicknessChannel = currentValue
                            Component.onCompleted: thicknessChannelComboBox.currentIndex = thicknessChannelComboBox.indexOfValue(shaderPoker.shaderProperties.thicknessChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onThicknessChannelChanged() {
                                    thicknessChannelComboBox.currentIndex = thicknessChannelComboBox.indexOfValue(shaderPoker.shaderProperties.thicknessChannel)
                                }
                            }
                        }

                        Label {
                            text: "Base Color Channel"
                        }

                        ComboBox {
                            id: baseColorChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.baseColorChannel = currentValue
                            Component.onCompleted: baseColorChannelComboBox.currentIndex = baseColorChannelComboBox.indexOfValue(shaderPoker.shaderProperties.baseColorChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onBaseColorChannelChanged() {
                                    baseColorChannelComboBox.currentIndex = baseColorChannelComboBox.indexOfValue(shaderPoker.shaderProperties.baseColorChannel)
                                }
                            }
                        }

                        Label {
                            text: "Specular Amount Channel"
                        }

                        ComboBox {
                            id: specularAmountChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.specularAmountChannel = currentValue
                            Component.onCompleted: specularAmountChannelComboBox.currentIndex = specularAmountChannelComboBox.indexOfValue(shaderPoker.shaderProperties.specularAmountChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onSpecularAmountChannelChanged() {
                                    specularAmountChannelComboBox.currentIndex = specularAmountChannelComboBox.indexOfValue(shaderPoker.shaderProperties.specularAmountChannel)
                                }
                            }
                        }

                        Label {
                            text: "Emissive Channel"
                        }

                        ComboBox {
                            id: emissiveChannelComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.emissiveChannel = currentValue
                            Component.onCompleted: emissiveChannelComboBox.currentIndex = emissiveChannelComboBox.indexOfValue(shaderPoker.shaderProperties.emissiveChannel)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "R"},
                                { value: 1, text: "G"},
                                { value: 2, text: "B"},
                                { value: 3, text: "A"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onEmissiveChannelChanged() {
                                    emissiveChannelComboBox.currentIndex = emissiveChannelComboBox.indexOfValue(shaderPoker.shaderProperties.emissiveChannel)
                                }
                            }
                        }

                        Label {
                            text: "boneCount"
                        }

                        SpinBox {
                            from: 0
                            // to: 16
                            stepSize: 1
                            value: shaderPoker.shaderProperties.boneCount
                            onValueModified: {
                                shaderPoker.shaderProperties.boneCount = value
                            }
                        }

                        CheckBox {
                            id: isDoubleSidedCheckbox
                            Layout.columnSpan: 2
                            text: "isDoubleSided"
                            checked: shaderPoker.shaderProperties.isDoubleSided
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.isDoubleSided = checked
                            }
                        }

                        CheckBox {
                            id: overridesPositionCheckbox
                            Layout.columnSpan: 2
                            text: "overridesPosition"
                            checked: shaderPoker.shaderProperties.overridesPosition
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.overridesPosition = checked
                            }
                        }

                        CheckBox {
                            id: usesProjectionMatrixCheckbox
                            Layout.columnSpan: 2
                            text: "usesProjectionMatrix"
                            checked: shaderPoker.shaderProperties.usesProjectionMatrix
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.usesProjectionMatrix = checked
                            }
                        }

                        CheckBox {
                            id: usesInverseProjectionMatrixCheckbox
                            Layout.columnSpan: 2
                            text: "usesInverseProjectionMatrix"
                            checked: shaderPoker.shaderProperties.usesInverseProjectionMatrix
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.usesInverseProjectionMatrix = checked
                            }
                        }

                        CheckBox {
                            id: usesPointsTopologyCheckbox
                            Layout.columnSpan: 2
                            text: "usesPointsTopology"
                            checked: shaderPoker.shaderProperties.usesPointsTopology
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.usesPointsTopology = checked
                            }
                        }

                        CheckBox {
                            id: usesVarColorCheckbox
                            Layout.columnSpan: 2
                            text: "usesVarColor"
                            checked: shaderPoker.shaderProperties.usesVarColor
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.usesVarColor = checked
                            }
                        }

                        Label {
                            text: "Alpha Mode"
                        }


                        ComboBox {
                            id: alphaModeComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.alphaMode = currentValue
                            Component.onCompleted: alphaModeComboBox.currentIndex = alphaModeComboBox.indexOfValue(shaderPoker.shaderProperties.alphaMode)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "Default"},
                                { value: 1, text: "Mask"},
                                { value: 2, text: "Blend"},
                                { value: 3, text: "Oqaque"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onAlphaModeChanged() {
                                    alphaModeComboBox.currentIndex = alphaModeComboBox.indexOfValue(shaderPoker.shaderProperties.alphaMode)
                                }
                            }
                        }

                        VertexAttributeEditor {
                            id: vertexAttributeEditor
                            Layout.columnSpan: 2
                            vertexAttributes: shaderPoker.shaderProperties.vertexAttributes
                            onVertexAttributesModified: (newValue) => {
                                shaderPoker.shaderProperties.vertexAttributes = newValue
                            }

                        }


                        CheckBox {
                            id: usesFloatJointIndicesCheckbox
                            Layout.columnSpan: 2
                            text: "usesFloatJointIndices"
                            checked: shaderPoker.shaderProperties.usesFloatJointIndices
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.usesFloatJointIndices = checked
                            }
                        }

                        CheckBox {
                            id: usesInstancingCheckbox
                            Layout.columnSpan: 2
                            text: "usesInstancing"
                            checked: shaderPoker.shaderProperties.usesInstancing
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.usesInstancing = checked
                            }
                        }

                        Label {
                            text: "targetCount"
                        }

                        SpinBox {
                            from: 0
                            to: 255
                            stepSize: 1
                            value: shaderPoker.shaderProperties.targetCount
                            onValueModified: {
                                shaderPoker.shaderProperties.targetCount = value
                            }
                        }

                        Label {
                            text: "targetPositionOffset"
                        }

                        SpinBox {
                            from: 0
                            to: 255
                            stepSize: 1
                            value: shaderPoker.shaderProperties.targetPositionOffset
                            onValueModified: {
                                shaderPoker.shaderProperties.targetPositionOffset = value
                            }
                        }

                        Label {
                            text: "targetNormalOffset"
                        }

                        SpinBox {
                            from: 0
                            to: 255
                            stepSize: 1
                            value: shaderPoker.shaderProperties.targetNormalOffset
                            onValueModified: {
                                shaderPoker.shaderProperties.targetNormalOffset = value
                            }
                        }

                        Label {
                            text: "targetTangentOffset"
                        }

                        SpinBox {
                            from: 0
                            to: 255
                            stepSize: 1
                            value: shaderPoker.shaderProperties.targetTangentOffset
                            onValueModified: {
                                shaderPoker.shaderProperties.targetTangentOffset = value
                            }
                        }

                        Label {
                            text: "targetBinormalOffset"
                        }

                        SpinBox {
                            from: 0
                            to: 255
                            stepSize: 1
                            value: shaderPoker.shaderProperties.targetBinormalOffset
                            onValueModified: {
                                shaderPoker.shaderProperties.targetBinormalOffset = value
                            }
                        }

                        Label {
                            text: "targetTexCoord0Offset"
                        }

                        SpinBox {
                            from: 0
                            to: 255
                            stepSize: 1
                            value: shaderPoker.shaderProperties.targetTexCoord0Offset
                            onValueModified: {
                                shaderPoker.shaderProperties.targetTexCoord0Offset = value
                            }
                        }

                        Label {
                            text: "targetTexCoord1Offset"
                        }

                        SpinBox {
                            from: 0
                            to: 255
                            stepSize: 1
                            value: shaderPoker.shaderProperties.targetTexCoord1Offset
                            onValueModified: {
                                shaderPoker.shaderProperties.targetTexCoord1Offset = value
                            }
                        }

                        Label {
                            text: "targetColorOffset"
                        }

                        SpinBox {
                            from: 0
                            to: 255
                            stepSize: 1
                            value: shaderPoker.shaderProperties.targetColorOffset
                            onValueModified: {
                                shaderPoker.shaderProperties.targetColorOffset = value
                            }
                        }

                        CheckBox {
                            id: blendParticlesCheckbox
                            Layout.columnSpan: 2
                            text: "blendParticles"
                            checked: shaderPoker.shaderProperties.blendParticles
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.blendParticles = checked
                            }
                        }

                        CheckBox {
                            id: clearcoatEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "clearcoatEnabled"
                            checked: shaderPoker.shaderProperties.clearcoatEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.clearcoatEnabled = checked
                            }
                        }

                        CheckBox {
                            id: transmissionEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "transmissionEnabled"
                            checked: shaderPoker.shaderProperties.transmissionEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.transmissionEnabled = checked
                            }
                        }

                        CheckBox {
                            id: specularAAEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "specularAAEnabled"
                            checked: shaderPoker.shaderProperties.specularAAEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.specularAAEnabled = checked
                            }
                        }

                        CheckBox {
                            id: lightmapEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "lightmapEnabled"
                            checked: shaderPoker.shaderProperties.lightmapEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.lightmapEnabled = checked
                            }
                        }

                        CheckBox {
                            id: specularGlossyEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "specularGlossyEnabled"
                            checked: shaderPoker.shaderProperties.specularGlossyEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.specularGlossyEnabled = checked
                            }
                        }

                        CheckBox {
                            id: metallicRoughnessEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "metallicRoughnessEnabled"
                            checked: shaderPoker.shaderProperties.metallicRoughnessEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.metallicRoughnessEnabled = checked
                            }
                        }

                        Label {
                            text: "debugMode"
                        }

                        ComboBox {
                            id: debugModeComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.debugMode = currentValue
                            Component.onCompleted: debugModeComboBox.currentIndex = debugModeComboBox.indexOfValue(shaderPoker.shaderProperties.debugMode)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "None"},
                                { value: 1, text: "BaseColor"},
                                { value: 2, text: "Roughness"},
                                { value: 3, text: "Metalnessl"},
                                { value: 4, text: "Diffuse"},
                                { value: 5, text: "Specular"},
                                { value: 6, text: "ShadowOcclusion"},
                                { value: 7, text: "Emission"},
                                { value: 8, text: "AmbientOcclusion"},
                                { value: 9, text: "Normal"},
                                { value: 10, text: "Tangent"},
                                { value: 11, text: "Binormal"},
                                { value: 12, text: "F0"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onDebugModeChanged() {
                                    debugModeComboBox.currentIndex = debugModeComboBox.indexOfValue(shaderPoker.shaderProperties.debugMode)
                                }
                            }
                        }

                        CheckBox {
                            id: fogEnabledCheckbox
                            Layout.columnSpan: 2
                            text: "fogEnabled"
                            checked: shaderPoker.shaderProperties.fogEnabled
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.fogEnabled = checked
                            }
                        }

                        Label {
                            text: "viewCount"
                        }

                        SpinBox {
                            from: 0
                            to: 255
                            stepSize: 1
                            value: shaderPoker.shaderProperties.viewCount
                            onValueModified: {
                                shaderPoker.shaderProperties.viewCount = value
                            }
                        }

                        CheckBox {
                            id: usesViewIndexCheckbox
                            Layout.columnSpan: 2
                            text: "usesViewIndex"
                            checked: shaderPoker.shaderProperties.usesViewIndex
                            onCheckedChanged: {
                                shaderPoker.shaderProperties.usesViewIndex = checked
                            }
                        }

                        Label {
                            text: "OIT Mode"
                        }

                        ComboBox {
                            id: orderIndependentTransparencyComboBox
                            textRole: "text"
                            valueRole: "value"
                            implicitContentWidthPolicy: ComboBox.WidestText
                            onActivated: shaderPoker.shaderProperties.orderIndependentTransparency = currentValue
                            Component.onCompleted: orderIndependentTransparencyComboBox.currentIndex = orderIndependentTransparencyComboBox.indexOfValue(shaderPoker.shaderProperties.orderIndependentTransparency)
                            Layout.fillWidth: true
                            model: [
                                { value: 0, text: "None"},
                                { value: 1, text: "WeightedBlended"}
                            ]

                            Connections {
                                target: shaderPoker.shaderProperties
                                function onOrderIndependentTransparencyChanged() {
                                    orderIndependentTransparencyComboBox.currentIndex = orderIndependentTransparencyComboBox.indexOfValue(shaderPoker.shaderProperties.orderIndependentTransparency)
                                }
                            }
                        }

                    }
                }
            }
        }

        Page {
            id: shaderKeyPage
            SplitView.preferredWidth: editorViews.width * 0.2
            title: "Shader Key"
            header: ToolBar {
                width: parent.width
                Label {
                    width: parent.width
                    text: shaderKeyPage.title
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 20
                }
            }

            ListView {
                anchors.fill: parent
                model: shaderPoker.shaderKeysList
                delegate: ItemDelegate {
                    required property string modelData
                    text: modelData
                }
                ScrollIndicator.vertical: ScrollIndicator { }
            }
        }

        Page {
            id: vertexShaderViewer
            SplitView.preferredWidth: editorViews.width * 0.3
            title: "Vertex Shader"

            header: ToolBar {
                width: parent.width
                Label {
                    width: parent.width
                    text: vertexShaderViewer.title
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 20
                }
            }

            ShaderEditor {
                anchors.fill: parent
                text: shaderPoker.vertexShader
            }
        }

        Page {
            id: fragmentShaderViewer
            SplitView.preferredWidth: editorViews.width * 0.3
            title: "Fragment Shader"

            header: ToolBar {
                width: parent.width
                Label {
                    width: parent.width
                    text: fragmentShaderViewer.title
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 20
                }
            }

            ShaderEditor {
                anchors.fill: parent
                text: shaderPoker.fragmentShader
            }
        }
    }
}
