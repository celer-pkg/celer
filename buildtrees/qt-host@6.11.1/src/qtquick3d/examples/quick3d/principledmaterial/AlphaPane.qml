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
    property string colorString: specularGlossyMode ? "Albedo" : "Base"

    ColumnLayout {
        width: rootView.availableWidth

        MarkdownLabel {

            text: `# Alpha Transparency
Material transparency can be achieved through Alpha Blending. The preferred method
is to just use the Alpha channel of the ${rootView.colorString} Color property. This is just part
of the ${rootView.colorString} Color and can be set either through the scalar ${rootView.colorString} Color value or by
using a Texture for the ${rootView.colorString} Color that contains an alpha channel. When using this
method it is important to set the correct Alpha mode to get the desired effect.`
        }

        MarkdownLabel {
            text: `## ${rootView.colorString} Color Alpha`
        }
        RowLayout {
            Label {
                text: "Alpha (" + rootView.targetMaterial.baseColor.a.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 1
                value: rootView.targetMaterial.baseColor.a
                onValueChanged: rootView.targetMaterial.baseColor.a = value
            }
        }

        MarkdownLabel {
            text: `## Alpha Mode
The Alpha Mode defines how the alpha channel of the ${rootView.colorString} Color is used by the
material.  If the mode is set to *Default* and you adjust the alpha value of ${rootView.colorString}
Color you should notice a grid pattern.  That is because the *Default* mode will
just write the alpha value to the output surface without blending. In our case
there just so happens to be a grid pattern behind the 3D Viewport to demonstrate
this effect. In this case the blend is with the 2D scene, not the 3D scene.

To do Alpha Blending with the 3D scene the mode should be set to *Blend*.  If you
know an item should always be opaque and you want to just ignore the alpha
value all together, then the mode should be set to *Opaque* which will avoid the
alpha passthrough effect you get with the *Default* mode.

The last mode is *Mask* which works in conjunction with the Alpha Cutoff property.
If the Alpha is greater than the value in Alpha Cutoff, it will be rendered, and
if it is not then it will not. This is useful for certain effects, as well as
rendering leaves using on a plane and an image with alpha.`
        }
        ComboBox {
            id: alphaModeComboBox
            textRole: "text"
            valueRole: "value"
            implicitContentWidthPolicy: ComboBox.WidestText
            onActivated: rootView.targetMaterial.alphaMode = currentValue
            Component.onCompleted: currentIndex = indexOfValue(rootView.targetMaterial.alphaMode)
            model: [
                { value: PrincipledMaterial.Default, text: "Default"},
                { value: PrincipledMaterial.Blend, text: "Blend"},
                { value: PrincipledMaterial.Opaque, text: "Opaque"},
                { value: PrincipledMaterial.Mask, text: "Mask"}
            ]
        }

        VerticalSectionSeparator {}

        MarkdownLabel {
            text: `## Alpha Cutoff
To demonstrate the behavior of Alpha Cutoff with the *Mask* Alpha Mode we need to
have a ${rootView.colorString} Color map with an Alpha map. Pressing the \"Enable Alpha Mask\"
button will setup a ${rootView.specularGlossyMode ? "AlbedoMap" : "BaseColorMap"} that looks like this:`
        }
        Item {
            implicitHeight: 256
            implicitWidth: 256
            Image {
                anchors.fill: parent
                source: "maps/grid.png"
                fillMode: Image.Tile
                horizontalAlignment: Image.AlignLeft
                verticalAlignment: Image.AlignTop
                Image {
                    anchors.fill: parent
                    source: "maps/alpha_gradient.png"
                }
            }
        }

        Button {
            property bool isEnabled: false
            property Texture revertTexture: null
            text: isEnabled ? rootView.specularGlossyMode ? "Revert Albedo Map" : "Revert Base Color Map" : "Enable Alpha Mask"
            onClicked:  {
                if (!isEnabled) {
                    if (rootView.specularGlossyMode) {
                        revertTexture = rootView.targetMaterial.albedoMap
                        rootView.targetMaterial.albedoColor.a = 1.0
                        rootView.targetMaterial.albedoMap = alphaGradientTexture
                    } else {
                        revertTexture = rootView.targetMaterial.baseColorMap
                        rootView.targetMaterial.baseColor.a = 1.0
                        rootView.targetMaterial.baseColorMap = alphaGradientTexture
                    }
                    rootView.targetMaterial.alphaMode = PrincipledMaterial.Mask
                    alphaModeComboBox.currentIndex = alphaModeComboBox.indexOfValue(rootView.targetMaterial.alphaMode)
                    isEnabled = true
                } else {
                    if (rootView.specularGlossyMode)
                        rootView.targetMaterial.albedoMap = revertTexture
                    else
                        rootView.targetMaterial.baseColorMap = revertTexture
                    revertTexture = null
                    isEnabled = false
                }
            }
        }

        Texture {
            id: alphaGradientTexture
            source: "maps/alpha_gradient.png"
        }

        RowLayout {
            Label {
                text: "Alpha Cutoff (" + rootView.targetMaterial.alphaCutoff.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 1
                value: rootView.targetMaterial.alphaCutoff
                onValueChanged: rootView.targetMaterial.alphaCutoff = value
            }
        }

        VerticalSectionSeparator {}

        MarkdownLabel {
            text: `## Culling
While not strictly related to transparency the concept of face culling is
relevant to getting the desired results. If you cut holes into the models
you see that the inside faces of the models don't render.  This is because
*Back Face* culling is on by default. The culling property decides which side
of a triangle being rendered gets culled (discarded). By changing the cull
mode of the material to *No Culling* both sides of geometry will be rendered`
        }
        ComboBox {
            id: cullModeComboBox
            textRole: "text"
            valueRole: "value"
            implicitContentWidthPolicy: ComboBox.WidestText
            onActivated: rootView.targetMaterial.cullMode = currentValue
            Component.onCompleted: currentIndex = indexOfValue(rootView.targetMaterial.cullMode)
            model: [
                { value: Material.BackFaceCulling, text: "Back Face"},
                { value: Material.FrontFaceCulling, text: "Front Face"},
                { value: Material.NoCulling, text: "None"}
            ]
        }

        VerticalSectionSeparator {}

        MarkdownLabel {
            text: `## Depth Draw Mode
Maybe you noticed that when the Blend Alpha Mode is enabled that one of the
models doesn't always look correct depending on the angle of viewing. That is
because while the rendering order of individual models are determined based on
distance they are to the camera, so models have multiple parts and how they are
rendered depends on the order the triangles appear in.  This isn't something
that can be fixed for every model, so instead we use a feature called the depth
buffer. We do not normally write to the depth buffer for transparent items though,
but sometimes it is still necessary to get the correct rendering.

The default Mode is *Opaque Only*, which means the material will only write to the
depth buffer if the material doesn't use transparency.

*Always* means that the material will write to the Depth buffer no matter what it does.

*Never* means that the material will never write to the Depth buffer even though it may
be opaque.

The special mode, and the one likely best suited to fix Alpha Cutoff related depth
errors is *Opaque Prepass*. In this case before any item is rendered, a separate pass is
done where materials will write their opaque pixels to the depth buffer while skipping
any transparent pixels. Then in the main pass everything is done as normal, but now will
be rendered correctly.
`
        }
        ComboBox {
            id: depthDrawModeComboBox
            textRole: "text"
            valueRole: "value"
            implicitContentWidthPolicy: ComboBox.WidestText
            onActivated: rootView.targetMaterial.depthDrawMode = currentValue
            Component.onCompleted: currentIndex = indexOfValue(rootView.targetMaterial.depthDrawMode)
            model: [
                { value: Material.OpaqueOnlyDepthDraw, text: "Opaque Only"},
                { value: Material.AlwaysDepthDraw, text: "Always"},
                { value: Material.NeverDepthDraw, text: "Never"},
                { value: Material.OpaquePrePassDepthDraw, text: "Opaque Prepass"}
            ]
        }

        VerticalSectionSeparator {}

        MarkdownLabel {
            text: `## Opacity
Another option for transparency is through the Opacity properties. Most effects
can be achieved using only the above properties, but these additional properties
will set the minimum level of opacity for the properties above.  It is also import
to point out that by using any of these Opacity properties will force alpha blending.`
        }

        RowLayout {
            Label {
                text: "Opacity Factor  (" + rootView.targetMaterial.opacity.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 1
                value: rootView.targetMaterial.opacity
                onValueChanged: rootView.targetMaterial.opacity = value
            }
        }
        MarkdownLabel {
            text: `### Opacity (Map)
The Opacity Map property specifies a texture to sample the Opacity value
from. Since the Opacity property is only a single floating point value between
0.0 and 1.0, it's only necessary to use a single color channel of the image, or
a greyscale image. By default PrincipledMaterial will use the value in the alpha
channel of the texture, but it's possible to change which color channel is used.
`
        }
        ComboBox {
            id: opacityChannelComboBox
            textRole: "text"
            valueRole: "value"
            implicitContentWidthPolicy: ComboBox.WidestText
            onActivated: rootView.targetMaterial.opacityChannel = currentValue
            Component.onCompleted: currentIndex = indexOfValue(rootView.targetMaterial.opacityChannel)
            model: [
                { value: PrincipledMaterial.R, text: "Red Channel"},
                { value: PrincipledMaterial.G, text: "Green Channel"},
                { value: PrincipledMaterial.B, text: "Blue Channel"},
                { value: PrincipledMaterial.A, text: "Alpha Channel"}
            ]
        }
        MarkdownLabel {
            text: `
When using a Opacity Map the value sampled from the map file is multiplied by
the value of the Opacity property. In practice this means that the maximum
Opacity value possible will be the value set by the Opacity map is the
value in the Opacity property. So most of the time when using a Opacity
Map it will make sense to leave the value of Opacity to 1.0.
`
        }
        RowLayout {
            Label {
                text: "Inverts the opacity value of the opacityMap"
            }
            Switch {
                checked: rootView.targetMaterial.invertOpacityMapValue
                onCheckedChanged: {
                    rootView.targetMaterial.invertOpacityMapValue = checked
                }
            }
        }
        Button {
            text: "Reset Opacity Value"
            onClicked: rootView.targetMaterial.opacity = 1.0
        }
        TextureSourceControl {
            defaultClearColor: "white"
            defaultTexture: "maps/metallic/metallic.jpg"
            onTargetTextureChanged: {
                rootView.targetMaterial.opacityMap = targetTexture
            }
        }
    }
}
// qmllint enable missing-property
