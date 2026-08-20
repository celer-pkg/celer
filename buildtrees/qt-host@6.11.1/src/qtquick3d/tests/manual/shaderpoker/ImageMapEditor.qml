// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

SectionLayout {
    id: imageMapEditor
    required property int imageMapProperties
    isExpanded: false

    readonly property var imageMapBits: {
        "Enabled": 1 << 0,
        "EnvMap": 1 << 1,
        "LightProbe": 1 << 2,
        "Identity": 1 << 3,
        "UsesUV1": 1 << 4,
        "Linear": 1 << 5
    }

    signal imageMapPropertiesModified(int value)

    CheckBox {
        Layout.columnSpan: 2
        text: "Enabled"
        readonly property int mask: imageMapEditor.imageMapBits.Enabled
        checked: imageMapEditor.imageMapProperties & mask
        onCheckedChanged: {
            if (checked) {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties | mask)
            } else {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "EnvMap"
        readonly property int mask: imageMapEditor.imageMapBits.EnvMap
        checked: imageMapEditor.imageMapProperties & mask
        onCheckedChanged: {
            if (checked) {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties | mask)
            } else {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "LightProbe"
        readonly property int mask: imageMapEditor.imageMapBits.LightProbe
        checked: imageMapEditor.imageMapProperties & mask
        onCheckedChanged: {
            if (checked) {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties | mask)
            } else {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "Identity"
        readonly property int mask: imageMapEditor.imageMapBits.Identity
        checked: imageMapEditor.imageMapProperties & mask
        onCheckedChanged: {
            if (checked) {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties | mask)
            } else {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "UsesUV1"
        readonly property int mask: imageMapEditor.imageMapBits.UsesUV1
        checked: imageMapEditor.imageMapProperties & mask
        onCheckedChanged: {
            if (checked) {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties | mask)
            } else {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties & ~mask)
            }
        }
    }

    CheckBox {
        Layout.columnSpan: 2
        text: "Linear"
        readonly property int mask: imageMapEditor.imageMapBits.Linear
        checked: imageMapEditor.imageMapProperties & mask
        onCheckedChanged: {
            if (checked) {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties | mask)
            } else {
                imageMapEditor.imageMapPropertiesModified(imageMapEditor.imageMapProperties & ~mask)
            }
        }
    }
}
