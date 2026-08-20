// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

RenderPass {
    id: gbufferPass
    clearColor: Qt.rgba(0.0, 0.0, 0.0, 0.0)

    property alias layerMask: filter.layerMask
    required property RenderPassTexture depthTexture

    RenderPassTexture {
        id: gbuffer0
        format: RenderPassTexture.RGBA16F
        // rgb: baseColor (linear), a: metalness
    }

    RenderPassTexture {
        id: gbuffer1
        format: RenderPassTexture.RGBA16F
        // rgb: normal, a: roughness
    }

    RenderPassTexture {
        id: gbuffer2
        format: RenderPassTexture.RGBA16F
        // rgb: emissive, a: ao/spare
    }

    commands: [
        ColorAttachment { target: gbuffer0; name: "GBUFFER0" },
        ColorAttachment { target: gbuffer1; name: "GBUFFER1" },
        ColorAttachment { target: gbuffer2; name: "GBUFFER2" },
        DepthTextureAttachment { target: gbufferPass.depthTexture },
        RenderablesFilter {
            id: filter
            renderableTypes: RenderablesFilter.Opaque
        }
    ]

    materialMode: RenderPass.AugmentMaterial
    augmentShader: "gbuffer_augment.glsl"
}
