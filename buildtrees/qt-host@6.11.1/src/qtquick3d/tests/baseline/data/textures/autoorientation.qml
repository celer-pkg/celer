// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick3D

Rectangle {
    width: 500
    height: 400
    color: "lightgray"

    View3D {
        anchors.fill: parent
        renderMode: View3D.Offscreen

        OrthographicCamera {
            z: 500
        }

        DirectionalLight {
            eulerRotation.y: 180
            ambientColor: Qt.rgba(1.0, 1.0, 1.0, 1.0);
        }

        Text {
            text: "Default   |   autoOrientation: false   |   flipV: true   |   autoOrientation: false; flipV: true"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Row 1

        // A #Rectangle model textured with a Texture sourced from a .png file.
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(-150, 100, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { source: "../shared/maps/t2.png" }
                } ]
        }

        // autoOrientation is false but has no visible effect.
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(-50, 100, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture {
                        source: "../shared/maps/t2.png"
                        autoOrientation: false
                    }
                } ]
        }

        // flipV is set to true (appears upside down).
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(50, 100, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture {
                        source: "../shared/maps/t2.png"
                        flipV: true
                    }
                } ]
        }

        // autoOrientation=false and flipV=true (same as just setting flipV=true)
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(150, 100, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture {
                        source: "../shared/maps/t2.png"
                        autoOrientation: false
                        flipV: true
                    }
                } ]
        }

        // Row 2

        // A #Rectangle model textured with a Texture sourced from a .ktx file.
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(-150, 0, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture { source: "../shared/maps/o1_bc1.ktx" }
                } ]
        }

        // autoOrientation is false so it should show up as upside down (as if flipV was set to true).
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(-50, 0, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture {
                        source: "../shared/maps/o1_bc1.ktx"
                        autoOrientation: false
                    }
                } ]
        }

        // flipV is set to true (appears upside down).
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(50, 0, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture {
                        source: "../shared/maps/o1_bc1.ktx"
                        flipV: true
                    }
                } ]
        }

        // autoOrientation=false and flipV=true (same as column #1 because the effective flipV is false).
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(150, 0, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture {
                        source: "../shared/maps/o1_bc1.ktx"
                        autoOrientation: false
                        flipV: true
                    }
                } ]
        }

        // Row 3

        // A #Rectangle model textured with a Texture with a sourceItem.
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(-150, -100, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture {
                        sourceItem: Rectangle {
                            id: miniQuickScene
                            width: 128
                            height: 128
                            color: "green"
                            Image {
                                source: "../shared/maps/t2.png"
                                anchors.centerIn: parent
                            }
                        }
                    }
                } ]
        }

        // autoOrientation is false so it should show up as upside down (as if flipV was set to true).
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(-50, -100, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture {
                        autoOrientation: false
                        sourceItem: miniQuickScene
                    }
                } ]
        }

        // flipV is set to true (appears upside down).
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(50, -100, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture {
                        flipV: true
                        sourceItem: miniQuickScene
                    }
                } ]
        }

        // autoOrientation=false and flipV=true (same as column #1 because the effective flipV is false).
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(0.7, 0.7, 1)
            position: Qt.vector3d(150, -100, 0)
            materials: [ DefaultMaterial {
                    diffuseMap: Texture {
                        autoOrientation: false
                        flipV: true
                        sourceItem: miniQuickScene
                    }
                } ]
        }
    }
}
