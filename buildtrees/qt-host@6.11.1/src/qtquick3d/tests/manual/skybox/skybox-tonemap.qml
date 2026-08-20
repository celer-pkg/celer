// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Helpers

Window {
    id: window
    width: 1280
    height: 720
    visible: true

    property bool linear: extBut.checked

    function alpha(i : int) : real {
        return (i + 1) / 10
    }

    property string sharedMaps: "../../baseline/data/shared/maps/"

    CubeMapTexture {
        id: skyboxTextureJPG
        source: sharedMaps + "clouds1_%p.jpg"
    }

    CubeMapTexture {
        id: skyboxTextureKTX
        source: sharedMaps + "fishpond_bc1.ktx"
    }

    property CubeMapTexture skyboxTexture: ktxBut.checked ? skyboxTextureKTX : skyboxTextureJPG

    Texture {
        id: hdrTexture
        source: sharedMaps +  "OpenfootageNET_lowerAustria01-1024.hdr"
    }

    View3D {
        id: view
        anchors.fill: parent

        ExtendedSceneEnvironment {
            id: ext
            clearColor: "skyblue"
            lightProbe: hdrTexture
            skyBoxCubeMap: skyboxTexture
            backgroundMode: skyboxBut.checked ? SceneEnvironment.SkyBoxCubeMap : SceneEnvironment.SkyBox
            tonemapMode: buttonGroup.checkedButton.tm
            exposure: exposureSlider.value
        }

        SceneEnvironment {
            id: env
            clearColor: "skyblue"
            lightProbe: hdrTexture
            skyBoxCubeMap: skyboxTexture
            backgroundMode: skyboxBut.checked ? SceneEnvironment.SkyBoxCubeMap : SceneEnvironment.SkyBox
            tonemapMode: buttonGroup.checkedButton.tm
        }

        environment: linear ?  ext : env

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 200)
        }

        Model {
            id: reference
            source: "#Cube"
            scale: "0.2,1,0.2"
            z: -50
            y: 0
            materials: PrincipledMaterial {
                baseColor: "white"
                lighting: PrincipledMaterial.NoLighting
            }
            Model {
                source: "#Cube"
                scale: "0.5,0.9,1.5"

                materials: PrincipledMaterial {
                    baseColor: "red"
                    lighting: PrincipledMaterial.NoLighting
                }
            }
        }

        component Alphas: Repeater3D {
            model: 10
            Node {
                y: -50 + 10 * index
                z: -20
                Model {
                    source: "#Cube"
                    scale: "0.2, 0.09, 0.05"
                    x: -10
                    materials: PrincipledMaterial {
                        baseColor: "black"
                        lighting: PrincipledMaterial.NoLighting
                    }
                    opacity: alpha(index)
                }
                Model {
                    source: "#Cube"
                    scale: "0.2, 0.09, 0.05"
                    x: 10
                    materials: PrincipledMaterial {
                        baseColor: "white"
                        lighting: PrincipledMaterial.NoLighting
                    }
                    opacity: alpha(index)
                }
            }
        }

        Alphas {
        }

        Model {
            x: 100
            z: -50
            scale: "2,2,2"
            source: "#Rectangle"
            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: sharedMaps + "clouds1_negz.jpg"//"/home/paul/tmp/HDRIHavenPack/abandoned_hopper_terminal_04/negz.png"
                    generateMipmaps: true
                    mipFilter: Texture.Linear
                }
                lighting: PrincipledMaterial.NoLighting
            }
        }
    }
    WasdController {
        controlledObject: camera
        speed: 0.1
    }

    ButtonGroup {
        id: buttonGroup
        buttons: column.children
    }
    Column {
        CheckBox {
            id: extBut
            text: "ExtendedSceneEnvironment"
            checked: false
        }
        CheckBox {
            id: skyboxBut
            text: "SkyBoxCubeMap"
            checked: true
        }
        CheckBox {
            id: ktxBut
            text: "KTX cube map"
            checked: false
            enabled: skyboxBut.checked
        }
        Row {
            Text {
                text: "Exposure"
                color: extBut.checked ? "black" : "lightgray"
            }
            Slider {
                id: exposureSlider
                from: 0
                to: 10
                value: 1
                enabled: extBut.checked
            }
            Text {
                text: extBut.checked ? exposureSlider.value.toFixed(2) : ""
            }
        }

        Column {
            id: column
            RadioButton {
                text: "TonemapModeNone"
                checked: false
                property int tm: SceneEnvironment.TonemapModeNone
            }
            RadioButton {
                text: "TonemapModeLinear"
                checked: true
                property int tm: SceneEnvironment.TonemapModeLinear
            }
            RadioButton {
                text: "TonemapModeAces"
                checked: false
                property int tm: SceneEnvironment.TonemapModeAces
            }
            RadioButton {
                text: "TonemapModeHejlDawson"
                checked: false
                property int tm: SceneEnvironment.TonemapModeHejlDawson
            }
            RadioButton {
                text: "TonemapModeFilmic"
                checked: false
                property int tm: SceneEnvironment.TonemapModeFilmic
            }
        }
    }
}
