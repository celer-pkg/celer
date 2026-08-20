// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#444845"
            backgroundMode: SceneEnvironment.Color
        }

        camera: camera
        OrthographicCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        FileInstancing {
            id: xmlInstance
            source: "instancelist.xml"
        }

        // Verify that we default to reading from the binary file by having a source file
        // that does not match the binary. wrongsourcedontregenerate.xml.bin is in fact generated from
        // instancelist.xml above, so the left hand side should be identical to the right hand side
        FileInstancing {
            id: binaryInstance
            source: "wrongsourcedontregenerate.xml"
        }

        Node {
            x: -100
            Model {
                source: "#Sphere"
                instancing: xmlInstance
                materials: [
                    CustomMaterial {
                        vertexShader: "customdata.vert"
                    }
                ]
            }
        }

        Node {
            x: 100
            Model {
                source: "#Sphere"
                instancing: binaryInstance
                materials: [
                    CustomMaterial {
                        vertexShader: "customdata.vert"
                    }
                ]
            }
        }
    }
}
