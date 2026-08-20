// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.15

Item {
    Text {
        color: "#ffffff"
        style: Text.Outline
        styleColor: "#606060"
        font.pixelSize: 28
        property int api: GraphicsInfo.api
        text: {
            if (GraphicsInfo.api === GraphicsInfo.OpenGLRhi)
                "OpenGL on QRhi";
            else if (GraphicsInfo.api === GraphicsInfo.Direct3D11Rhi)
                "D3D11 on QRhi";
            else if (GraphicsInfo.api === GraphicsInfo.VulkanRhi)
                "Vulkan on QRhi";
            else if (GraphicsInfo.api === GraphicsInfo.MetalRhi)
                "Metal on QRhi";
            else if (GraphicsInfo.api === GraphicsInfo.Null)
                "Null on QRhi";
            else
                "Unknown API";
        }
    }

    Text {
        id: srcItem
        text: "Hello world"
        anchors.centerIn: parent
        color: "red"
        font.pixelSize: 40
    }

    ShaderEffectSource {
        id: src
        sourceItem: srcItem
        anchors.fill: srcItem
        hideSource: true
        visible: false
    }

    ShaderEffect {
        anchors.fill: src
        property variant source: src
        property real amplitude: 0.04
        property real frequency: 5
        property real time: 0
        NumberAnimation on time { from: 0; to: Math.PI * 2; duration: 600; loops: -1 }
        fragmentShader: "wobble.frag.qsb"
    }
}
