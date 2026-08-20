// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

SectionLayout {
    id: vertexAttributeEditor

    title: "Vertex Attributes"

    required property int vertexAttributes

    readonly property var vertexAttributeTable: {
        "Position" : 1,
        "Normal" : 2,
        "TexCoord0" : 4,
        "TexCoord1" : 8,
        "Tangent" : 16,
        "Binormal" : 32,
        "Color" : 64,
        "JointAndWeight" : 128,
        "TexCoordLightmap" : 256
    }

    signal vertexAttributesModified(int value)

    CheckBox {
        Layout.columnSpan: 2
        text: "Position"
        readonly property int mask: vertexAttributeEditor.vertexAttributeTable.Position
        checked: vertexAttributeEditor.vertexAttributes & mask
        onCheckedChanged: {
            if (checked) {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes | mask)
            } else {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "Normal"
        readonly property int mask: vertexAttributeEditor.vertexAttributeTable.Normal
        checked: vertexAttributeEditor.vertexAttributes & mask
        onCheckedChanged: {
            if (checked) {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes | mask)
            } else {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "TexCoord0"
        readonly property int mask: vertexAttributeEditor.vertexAttributeTable.TexCoord0
        checked: vertexAttributeEditor.vertexAttributes & mask
        onCheckedChanged: {
            if (checked) {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes | mask)
            } else {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "TexCoord1"
        readonly property int mask: vertexAttributeEditor.vertexAttributeTable.TexCoord1
        checked: vertexAttributeEditor.vertexAttributes & mask
        onCheckedChanged: {
            if (checked) {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes | mask)
            } else {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "Tangent"
        readonly property int mask: vertexAttributeEditor.vertexAttributeTable.Tangent
        checked: vertexAttributeEditor.vertexAttributes & mask
        onCheckedChanged: {
            if (checked) {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes | mask)
            } else {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "Binormal"
        readonly property int mask: vertexAttributeEditor.vertexAttributeTable.Binormal
        checked: vertexAttributeEditor.vertexAttributes & mask
        onCheckedChanged: {
            if (checked) {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes | mask)
            } else {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "Color"
        readonly property int mask: vertexAttributeEditor.vertexAttributeTable.Color
        checked: vertexAttributeEditor.vertexAttributes & mask
        onCheckedChanged: {
            if (checked) {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes | mask)
            } else {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "JointAndWeight"
        readonly property int mask: vertexAttributeEditor.vertexAttributeTable.JointAndWeight
        checked: vertexAttributeEditor.vertexAttributes & mask
        onCheckedChanged: {
            if (checked) {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes | mask)
            } else {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "TexCoordLightmap"
        readonly property int mask: vertexAttributeEditor.vertexAttributeTable.TexCoordLightmap
        checked: vertexAttributeEditor.vertexAttributes & mask
        onCheckedChanged: {
            if (checked) {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes | mask)
            } else {
                vertexAttributeEditor.vertexAttributesModified(vertexAttributeEditor.vertexAttributes & ~mask)
            }
        }
    }
}
