// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D

// qmllint disable missing-property
// Disabling missing-property because the targetMaterial property
// will either be a PrincipaledMaterial or SpecularGlossyMaterial
// but the shared properties are not part of the common base class
ScrollView {
    id: rootView
    required property Material targetMaterial
    required property Model linesModel
    required property Model pointsModel
    property bool specularGlossyMode: false

    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    width: availableWidth

    ColumnLayout {
        width: rootView.availableWidth
        MarkdownLabel {
            text: `# Special Stuff
The content in this section is as titled \"Special\" because you should really
only use this stuff for fairly specific reasons.
## Lighting Mode
The lighting mode property defines how the material handles the shading of
lights. The default mode is *Fragment Lighting* which perform all light shading
in the fragment shader porsion of the graphics pipeline. In this case all both
image based lighting as well as any punctual lights are used to shade the material.
The other case *No Lighting* which ignores all light sources in the scene and
the material is rendered without shading.  This mode can be useful with models
that should not be shaded, for example tools (gizmos) inside of a 3D editor.
`
        }

        ComboBox {
            id: lightingComboBox
            textRole: "text"
            valueRole: "value"
            implicitContentWidthPolicy: ComboBox.WidestText
            onActivated: rootView.targetMaterial.lighting = currentValue
            Component.onCompleted: currentIndex = indexOfValue(rootView.targetMaterial.lighting)
            model: [
                { value: PrincipledMaterial.NoLighting, text: "No Lighting"},
                { value: PrincipledMaterial.FragmentLighting, text: "Fragment Lighting"}
            ]
        }

        VerticalSectionSeparator {}

        MarkdownLabel {
            text: `## Non Triangle Primitive Sizes
Generally if you use the Mesh file format baked by balsam, or the runtime
loader for loading glTF2 scenes, the geometry primitive will be individual
triangles, so these properties are not useful. However it is possible to
create your own mesh geometry where the primitive is instead either Points
or Lines, and in these cases it is possible to change the size of these
primitives when rendered. These properties will only have an affect on a
material when the geometry primitive is set to Line or Point respectively.
The behavior of these properties will depend on which graphics API the is
being used, and is possible that they will do nothing at all.`
        }

        Button {
            text: checked ? "Hide Wireframe Model" : "Show Wireframe Model"
            checkable: true
            checked: rootView.linesModel.visible
            onClicked: {
                rootView.linesModel.visible = !rootView.linesModel.visible
            }
        }

        RowLayout {
            Label {
                text: "Line Width (" + rootView.targetMaterial.lineWidth.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 25
                value: rootView.targetMaterial.lineWidth
                onValueChanged: rootView.targetMaterial.lineWidth = value
            }
        }

        Button {
            text: checked ? "Hide Point Cloud" : "Show Point Cloud"
            checkable: true
            checked: rootView.pointsModel.visible
            onClicked: {
                rootView.pointsModel.visible = !rootView.pointsModel.visible
            }
        }

        RowLayout {
            Label {
                text: "Point Size (" + rootView.targetMaterial.pointSize.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 25
                value: rootView.targetMaterial.pointSize
                onValueChanged: rootView.targetMaterial.pointSize = value
            }
        }

        ColumnLayout {
            VerticalSectionSeparator {}

            MarkdownLabel {
                text: `## Specular Overrides
If you are using this material for Physically Based Rendering (PBR) then
these properties should not be touched as they are not energy conserving and
will make materials look less realistic. These exist for adjusting the material
for artistic or stylistic reasons.`
            }

            MarkdownLabel {
                visible: !rootView.specularGlossyMode
                text: `### Specular Amount
This Specular Amount property controls the strength of specularity (highlights
and reflections) of a dielectric material (non-metallic). Normally this should
be left at 1.0, but can be adjusted to reduce the specularity. Adjusting this
property will not affect reflections caused by Image Based Lighting.`
            }
            RowLayout {
                visible: !rootView.specularGlossyMode
                Label {
                    text: "Specular Amount  (" + rootView.targetMaterial.specularAmount.toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    from: 0
                    to: 1
                    value: rootView.targetMaterial.specularAmount
                    onValueChanged: rootView.targetMaterial.specularAmount = value
                }
            }

            MarkdownLabel {
                text: `### Fresnel Power
This property decreases head-on reflections (looking directly at the
surface) while maintaining reflections seen at grazing angles.`
            }
            RowLayout {
                Label {
                    text: "Fresnel Power  (" + (rootView.targetMaterial.fresnelPower !== undefined ? rootView.targetMaterial.fresnelPower.toFixed(2) : 5.0) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    from: 0.0
                    to: 10.0
                    value: (rootView.targetMaterial.fresnelPower !== undefined ? rootView.targetMaterial.fresnelPower : 5.0)
                    onValueChanged: rootView.targetMaterial.fresnelPower = value
                }
            }

            MarkdownLabel {
                text: `## Enable Fresnel ScaleBias
The material will take Fresnel Scale and Fresnel Bias into account.`
            }
            RowLayout {
                Label {
                    text: "Enable fresnel ScaleBias"
                }
                Switch {
                    checked: rootView.targetMaterial.fresnelScaleBiasEnabled
                    onCheckedChanged: {
                        fresnelScaleLabel.enabled = checked
                        fresnelScale.enabled = checked
                        fresnelBiasLabel.enabled = checked
                        fresnelBias.enabled = checked
                        rootView.targetMaterial.fresnelScaleBiasEnabled = checked
                    }
                }
            }

            MarkdownLabel {
                id: fresnelScaleLabel
                enabled: false
                text: `### Fresnel Scale
This property scale head-on reflections (looking directly at the
surface) while maintaining reflections seen at grazing angles.`
            }
            RowLayout {
                id: fresnelScale
                enabled: false
                Label {
                    text: "Fresnel scale  (" + (rootView.targetMaterial.fresnelScale !== undefined ? rootView.targetMaterial.fresnelScale.toFixed(2) : 1.0) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    from: 0.0
                    to: 10.0
                    value: (rootView.targetMaterial.fresnelScale !== undefined ? rootView.targetMaterial.fresnelScale : 1.0)
                    onValueChanged: rootView.targetMaterial.fresnelScale = value
                }
            }

            MarkdownLabel {
                id: fresnelBiasLabel
                enabled: false
                text: `### Fresnel Bias
This property push forward head-on reflections (looking directly at the
surface) while maintaining reflections seen at grazing angles.`
            }
            RowLayout {
                id: fresnelBias
                enabled: false
                Label {
                    text: "Fresnel Bias  (" + (rootView.targetMaterial.fresnelBias !== undefined ? rootView.targetMaterial.fresnelBias.toFixed(2) : 0.0) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    from: -1.0
                    to: 1.0
                    value: (rootView.targetMaterial.fresnelBias !== undefined ? rootView.targetMaterial.fresnelBias : 0.0)
                    onValueChanged: rootView.targetMaterial.fresnelBias = value
                }
            }
            MarkdownLabel {
                visible: !rootView.specularGlossyMode
                text: `### Specular Tint
The Specular Tint property defines how much of the Base Color of the material
contributes to specular reflections.  This property only has an affect with
dielectric materials (non-metallic) as metallic material's specular reflections
are already primarily tinted by the materials base color. This property allows
the Base Color to also tint the specular reflections for dielectrics as well for
artistic reasons.`
            }
            RowLayout {
                visible: !rootView.specularGlossyMode
                Label {
                    text: "Specular Tint (" + rootView.targetMaterial.specularTint.toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    from: 0
                    to: 1
                    value: rootView.targetMaterial.specularTint
                    onValueChanged: rootView.targetMaterial.specularTint = value
                }
            }

            MarkdownLabel {
                visible: !rootView.specularGlossyMode
                text: `### Specular Map
The Specular Map property defines color (RGB) texture to modulate the amount
and the color of specularity across the surface of the material. These values
are multiplied by the Specular Amount property. This property will only have
an affect if the material is a dielectric (non-metallic).`
            }
            RowLayout {
                visible: !rootView.specularGlossyMode
                MarkdownLabel {
                    text: `### Enable specular single channel Map
The material will use the single value of the specularChannel from
the specularMap as RGB value.`
                }
                Switch {
                    checked: rootView.targetMaterial.specularSingleChannelEnabled
                    onCheckedChanged: {
                        rootView.targetMaterial.specularSingleChannelEnabled = checked
                    }
                }
            }
            ComboBox {
                visible: !rootView.specularGlossyMode
                enabled: rootView.targetMaterial.specularSingleChannelEnabled
                textRole: "text"
                valueRole: "value"
                implicitContentWidthPolicy: ComboBox.WidestText
                onActivated: rootView.targetMaterial.specularChannel = currentValue
                Component.onCompleted: currentIndex = indexOfValue(rootView.targetMaterial.specularChannel)
                model: [
                    { value: PrincipledMaterial.R, text: "Red Channel"},
                    { value: PrincipledMaterial.G, text: "Green Channel"},
                    { value: PrincipledMaterial.B, text: "Blue Channel"},
                    { value: PrincipledMaterial.A, text: "Alpha Channel"}
                ]
            }
            TextureSourceControl {
                visible: !rootView.specularGlossyMode
                defaultClearColor: "black"
                defaultTexture: "maps/metallic/metallic.jpg"
                onTargetTextureChanged: {
                    rootView.targetMaterial.specularMap = targetTexture
                }
            }

            MarkdownLabel {
                visible: !rootView.specularGlossyMode
                text: `### Specular Reflection Map
The Specular Reflection Map property is for providing a static environment map
for cheap reflection calculations.  Unlike most properties which take textures
in PrincipledMaterial, this property requires the source Texture to use the
EnvironmentMap mapping mode to look correct. In addition this means that the
source image file should be an equirectangular projection. This is an alternative
to using Imaged Based Lighting via lightProbes to get static reflections.`
            }

            TextureSourceControl {
                visible: !rootView.specularGlossyMode
                defaultClearColor: "black"
                defaultTexture: "maps/small_envmap.jpg"
                envMapMode: true
                onTargetTextureChanged: {
                    rootView.targetMaterial.specularReflectionMap = targetTexture
                }
            }
        }
    }
}
// qmllint enable missing-property
