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
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    width: availableWidth
    property bool specularGlossyMode: false

    ColumnLayout {
        width: rootView.availableWidth

        MarkdownLabel {
            text: `# Refraction
The properties in this section would probably be best described as advanced
transparency. In the previous section on transparency we discussed alpha
blending, which is about blending colors together using the alpha channel of
the material's color. What makes the transparency effects in this section
different is that the goal is to handle transparency in a way that more
physically represents how light works. To achieve this blending requires that
all content that is blended with needs to be rendered to a texture in separate
pass. Using any properties on this page is as expensive as rendering all opaque
content in the scene at least twice. Once to get the background items, and again
including the items using the refractive transparency effects. The advantage of
this approach though is that we are not limited in how we can blend, but comes
with the caveat that only opaque items are visible through refracted objects.`
        }

        MarkdownLabel {
            text: `## Transmission
Transmission refers to lights ability to transmit, or pass through a surface.  Not all
light will penetrate a surface and some will still be reflected as a specular
reflection. This ability to transmit light only concerns the surface of a material, and
not its depth. Without the use of further properties in this section, a material that
has transmission alone can be assumed to be infinitely thin.
### Transmission Factor
The Transmission Factor property controls the percentage of light that is transmitted by
a materials surface.  This value is a single value between 0.0 meaning no light is
transmitted and 1.0 meaning that 100% of the light that penetrates the surface of the
material is transmitted through.

Note: If you adjust the Transmission Factor to 1.0 and you still can't see
through the models, it could be that your material is metallic. Metallic materials
cannot transmit light.`
        }
        Button {
            text: "Reset Metalness to 0.0"
            onClicked: rootView.targetMaterial.metalness = 0.0
        }

        RowLayout {
            Label {
                text: "Transmission Factor (" + rootView.targetMaterial.transmissionFactor.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 1
                value: rootView.targetMaterial.transmissionFactor
                onValueChanged: rootView.targetMaterial.transmissionFactor = value
            }
        }
        MarkdownLabel {
            text: `### Transmission Map
Like most other single floating point value properties, the Transmission property
also allows for the use of a single channel of a texture to map transmission values
to a mesh. And like many other textures, the final value of transmission will be
the multiplication of Transmission Factor and the value sampled from Transmission Map.
So when using a Transmission Map, it typically makes sense to set the Transmission
Factor to 1.0.
`
        }
        Button {
            text: "Reset Transmission"
            onClicked: rootView.targetMaterial.transmissionFactor = 1.0
        }

        ComboBox {
            id: transmissionChannelComboBox
            textRole: "text"
            valueRole: "value"
            implicitContentWidthPolicy: ComboBox.WidestText
            onActivated: rootView.targetMaterial.transmissionChannel = currentValue
            Component.onCompleted: currentIndex = indexOfValue(rootView.targetMaterial.transmissionChannel)
            model: [
                { value: PrincipledMaterial.R, text: "Red Channel"},
                { value: PrincipledMaterial.G, text: "Green Channel"},
                { value: PrincipledMaterial.B, text: "Blue Channel"},
                { value: PrincipledMaterial.A, text: "Alpha Channel"}
            ]
        }
        TextureSourceControl {
            defaultTexture: "maps/noise.png"
            defaultClearColor: "black"
            onTargetTextureChanged: {
                rootView.targetMaterial.transmissionMap = targetTexture
            }
        }

        VerticalSectionSeparator {}

        ColumnLayout {
            visible: !rootView.specularGlossyMode
            MarkdownLabel {
                text: `## Index of Refraction (IOR)
The Index of Refraction or refraction index refers to the physical property of
how fast light passes through a material. This number then is used to determine
how light is bent or refracted when it enters a material. Since this value is
a physical value, it's possible to plug in the same values as real life
materials as well. The default value that the PrincipledMaterial uses for all
lighting calculations is 1.5, which is very close to window glass. Below are
several other materials' IOR values that will produce different results when
used with a refractive material (especially ones with thickness).`
            }

            ComboBox {
                id: iorChannelComboBox
                textRole: "text"
                valueRole: "value"
                implicitContentWidthPolicy: ComboBox.WidestText
                onActivated: rootView.targetMaterial.indexOfRefraction = currentValue
                Component.onCompleted: currentIndex = 0
                model: [
                    { value: 1.5, text: "Custom"},
                    { value: 1.4, text: "Acrylic glass"},
                    { value: 1.0, text: "Air"},
                    { value: 1.33, text: "Water"},
                    { value: 1.76, text: "Sapphire"},
                    { value: 2.42, text: "Diamond"}
                ]
            }
            RowLayout {
                Label {
                    text: "IOR (" + iorSlider.value.toFixed(2) + ")"
                    Layout.fillWidth: true
                }
                Slider {
                    id: iorSlider
                    from: 1.0
                    to: 3.0
                    value: rootView.targetMaterial.indexOfRefraction ?? 1.5
                    onValueChanged: {
                        if (iorChannelComboBox.currentValue != value)
                            iorChannelComboBox.currentIndex = 0;
                        rootView.targetMaterial.indexOfRefraction = value
                    }
                }
            }

            VerticalSectionSeparator {}
        }

        MarkdownLabel {
            text: `## Thickness
The Thickness properties are for giving refractive materials volume.  A
transmissive material alone is considered to be infinitely thin so any
Index of Refraction values will only affect the specular and fresnel
effects of a material.  However when a transmissive material is given
volume via the thickness properties, then light passing through the
material is bent as it passes through.
### Thickness Factor
The Thickness Factor property defines the thickness of the volume beneath
the surface of the mesh.  Unlike other factors, the Thickness Factor
Property is not clipped at 1.0, but rather refers to the distance in the
coordinate space of the mesh itself.  When used in conjunction with the
Thickness Map, the Thickness Factor would be the point of maximum thickness.
`
        }

        RowLayout {
            Label {
                text: "Thickness Factor (" + rootView.targetMaterial.thicknessFactor.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 100.0
                value: rootView.targetMaterial.thicknessFactor
                onValueChanged: rootView.targetMaterial.thicknessFactor = value
            }
        }

        MarkdownLabel {
            text: `### Thickness Map
The Thickness Map is a single channel (greyscale) texture that defines
the thickness (or volume) of a mesh.  The values sampled from the
Thickness Map are multiplied against the value of Thickness Factor to
get the thickness of the mesh under the surface in the meshe's coordinate
space.  Thickness Maps are baked in 3D content creation tools using ray
tracers. The process of baking thickness is similar to the process for
baking ambient occlusion, but the rays are cast in the opposite direction
of the surface normal (into the mesh).  Darker values represent thin
sections, and lighter values will be thicker.  Provided is a baked thickness
map of the Monkey model. (The other models would have uniform thicknesses).`
        }

        ComboBox {
            id: thicknessChannelComboBox
            textRole: "text"
            valueRole: "value"
            implicitContentWidthPolicy: ComboBox.WidestText
            onActivated: rootView.targetMaterial.thicknessChannel = currentValue
            Component.onCompleted: currentIndex = indexOfValue(rootView.targetMaterial.thicknessChannel)
            model: [
                { value: PrincipledMaterial.R, text: "Red Channel"},
                { value: PrincipledMaterial.G, text: "Green Channel"},
                { value: PrincipledMaterial.B, text: "Blue Channel"},
                { value: PrincipledMaterial.A, text: "Alpha Channel"}
            ]
        }
        TextureSourceControl {
            defaultTexture: "maps/monkey_thickness.jpg"
            defaultClearColor: "black"
            onTargetTextureChanged: {
                rootView.targetMaterial.thicknessMap = targetTexture
            }
        }

        VerticalSectionSeparator {}

        MarkdownLabel {
            text: `## Attenuation
As light passes through a volume it will be subject to absorption and scattering.
To simulate this interaction, two properties are provided for determining this
attenuation.
### Attenuation Color
The Attenuation Color property refers to the color that white light turns into
due to the absorption when reaching the attenuation distance.
`
        }
        RowLayout {
            Label {
                text: "Red (" + rootView.targetMaterial.attenuationColor.r.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 1
                value: rootView.targetMaterial.attenuationColor.r
                onValueChanged: rootView.targetMaterial.attenuationColor.r = value
            }
        }
        RowLayout {
            Label {
                text: "Green  (" + rootView.targetMaterial.attenuationColor.g.toFixed(2) + ")"
                Layout.fillWidth: true
            }

            Slider {
                from: 0
                to: 1
                value: rootView.targetMaterial.attenuationColor.g
                onValueChanged: rootView.targetMaterial.attenuationColor.g = value
            }
        }
        RowLayout {
            Label {
                text: "Blue (" + rootView.targetMaterial.attenuationColor.b.toFixed(2) + ")"
                Layout.fillWidth: true
            }

            Slider {
                from: 0
                to: 1
                value: rootView.targetMaterial.attenuationColor.b
                onValueChanged: rootView.targetMaterial.attenuationColor.b = value
            }
        }
        MarkdownLabel {
            text: `### Attenuation Distance
Attenuation Distance defines material density, but does so by describing the
average distance light must travel through the medium before interacting with a
particle (absorption). In this case the distance is specified in world
coordinate space (scene space). This distance can be any positive floating point
value. This means the attenuation color will start to appear when the thickness
is greater than the attenuation distance, with the caveat that the Attenuation
Color assumes white light is passing through the model, so any other light will
create a blended result. For this demonstration the slider value is limited to
100, which should be the maximum thickness for all 3 models.`
        }

        RowLayout {
            Label {
                text: "Attenuation Distance (" + rootView.targetMaterial.attenuationDistance.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 100
                value: rootView.targetMaterial.attenuationDistance
                onValueChanged:  {
                    if (value != rootView.targetMaterial.attenuationDistance)
                        rootView.targetMaterial.attenuationDistance = value
                }
            }
        }
    }
}
// qmllint enable missing-property
