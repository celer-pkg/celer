// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

SectionLayout {
    id: vertexColorMaskGroupBox
    required property int vertexColorMask

    readonly property var maskValues: {
        "NoMask" : 0,
        "RoughnessMask" : 1,
        "NormalStrengthMask" : 2,
        "SpecularAmountMask" : 4,
        "ClearcoatAmountMask" : 8,
        "ClearcoatRoughnessAmountMask" : 16,
        "ClearcoatNormalStrengthMask" : 32,
        "HeightAmountMask" : 64,
        "MetalnessMask" : 128,
        "OcclusionAmountMask" : 256,
        "ThicknessFactorMask" : 512,
        "TransmissionFactorMask" : 1024
    }

    signal vertexColorMaskModified(int value)

    CheckBox {
        Layout.columnSpan: 2
        text: "Roughness"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["RoughnessMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
    CheckBox {
        Layout.columnSpan: 2
        text: "Normal Strength"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["NormalStrengthMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
    CheckBox {
        Layout.columnSpan: 2
        text: "Specular Amount"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["SpecularAmountMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
    CheckBox {
        Layout.columnSpan: 2
        text: "Clearcoat Amount"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["ClearcoatAmountMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
    CheckBox {
        Layout.columnSpan: 2
        text: "Clearcoat Roughness"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["ClearcoatRoughnessAmountMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
    CheckBox {
        Layout.columnSpan: 2
        text: "Clearcoat Normal Strength"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["ClearcoatNormalStrengthMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
    CheckBox {
        Layout.columnSpan: 2
        text: "Height Amount"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["HeightAmountMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
    CheckBox {
        Layout.columnSpan: 2
        text: "Metalness"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["MetalnessMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
    CheckBox {
        Layout.columnSpan: 2
        text: "Occlusion Amount"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["OcclusionAmountMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
    CheckBox {
        Layout.columnSpan: 2
        text: "Thickness Factor"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["ThicknessFactorMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
    CheckBox {
        Layout.columnSpan: 2
        text: "Transmission Factor"
        readonly property int mask: vertexColorMaskGroupBox.maskValues["TransmissionFactorMask"]
        checked: vertexColorMaskGroupBox.vertexColorMask & mask
        onCheckedChanged: {
            if (checked) {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask | mask)
            } else {
                vertexColorMaskGroupBox.vertexColorMaskModified(vertexColorMaskGroupBox.vertexColorMask & ~mask)
            }
        }
    }
}

