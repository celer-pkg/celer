// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick.Window 2.12
import QtQuick3D

Window {
    visible: true
    width: 800
    height: 600
    View3D {
        anchors.fill: parent
        PerspectiveCamera {
            z: 600
        }

        InstanceListEntry {
            id: instanceListEntryOne
            position: Qt.vector3d(100, 0, 0)
            color: "red"
        }

        InstanceListEntry {
            id: instanceListEntryTwo
            position: Qt.vector3d(-100, 0, 0)
            color: "green"
        }

        InstanceList {
            id: instanceList
            instances: [ instanceListEntryOne, instanceListEntryTwo ]
        }

        Model {
            source: "#Cube"
            instancing: instanceList
            materials: DefaultMaterial {
                lighting: DefaultMaterial.NoLighting
            }
        }
    }
}
