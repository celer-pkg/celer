// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

RenderPass {
    id: normalRoughnessPass
    clearColor: Qt.rgba(0.0, 0.0, 0.0, 0.0)

    property vector3d animatingColor: vector3d(1.0, 1.0, 1.0)


    NumberAnimation on animatingColor.x {
        from: 0.0
        to: 1.0
        duration: 2000
        loops: Animation.Infinite
        easing.type: Easing.InOutQuad
        running: true
    }


    RenderPassTexture {
        id: outputTexture0
        format: RenderPassTexture.RGBA16F
    }

    RenderPassTexture {
        id: outputTexture1
        format: RenderPassTexture.RGBA16F
    }

    RenderPassTexture {
        id: outputTexture2
        format: RenderPassTexture.RGBA8
    }

    RenderPassTexture {
        id: depthStencilTexture
        format: RenderPassTexture.Depth24Stencil8
    }

    // output Color Attachment (maybe [])
    // output Depth, either a depthTexture or depthStencil, maybe from another unowned source

    // output: outputTexture

    commands: [
        ColorAttachment {
            target: outputTexture0
            name: "NORMAL_ROUGHNESS"
        },
        ColorAttachment {
            target: outputTexture1
            name: "BASECOLOR_OUTPUT"
        },
        ColorAttachment {
            target: outputTexture2
            name: "DIFFUSELIGHT_OUTPUT"
        },
        DepthTextureAttachment {
            target: depthStencilTexture
        },
        RenderablesFilter {
            // Filter the renderagles so only Opaque objects in layer0 are rendered
            layerMask: ContentLayer.Layer0 | ContentLayer.Layer1
            renderableTypes: RenderablesFilter.Opaque
        }
    ]

    materialMode: RenderPass.AugmentMaterial
    augmentShader: "normal_roughness_augment.glsl"

}
