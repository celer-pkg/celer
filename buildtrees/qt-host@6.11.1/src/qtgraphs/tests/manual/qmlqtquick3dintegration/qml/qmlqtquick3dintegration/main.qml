// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtQuick3D
import QtQuick3D.Helpers

pragma ComponentBehavior: Bound

Item {
    id: mainview
    width: 1280
    height: 1024

    // Use these for live data
    // property int xRotationTime: 30000
    // property int yRotationTime: 10000
    property int yMin: 75 // intentionally inverted
    property int yMax: 15 // intentionally inverted
    // Use these for non-live data
    property int xRotationTime: 10000
    property int yRotationTime: 3000
    // property int yMin: 10
    // property int yMax: 40

    // Data {
    //     id: graphData
    //     // Use this for non-live data, which animates the valueaxis max
    // }

    // Axes {
    //     id: graphAxes
    //     // Use this for non-live data, which animates the valueaxis max
    // }

    Bars3D {
        id: barGraph
        anchors.fill: parent
        shadowQuality: Graphs3D.ShadowQuality.Medium
        selectionMode: Graphs3D.SelectionFlag.Item
        msaaSamples: 8
        theme: Theme3D {
            type: Theme3D.Theme.Ebony
            labelBorderEnabled: true
            font.pointSize: 35
            labelBackgroundEnabled: true
            backgroundEnabled: false
            gridVisible: false
            labelsEnabled: false
            colorStyle: Theme3D.ColorStyle.RangeGradient
            singleHighlightGradient: customGradient

            Gradient {
                id: customGradient
                GradientStop { position: 1.0; color: "#FFFF00" }
                GradientStop { position: 0.0; color: "#000000" }
            }
        }
        barThickness: 0.7
        barSpacing: Qt.size(0.5, 0.5)
        barSpacingRelative: false
        valueAxis: Value3DAxis {
            // Use this for live data
            min: 64
            max: 256
        }
        // valueAxis: graphAxes.value // Use this for non-live data, which animates the valueaxis max

        NumberAnimation on cameraXRotation {
            duration: xRotationTime
            running: true
            from: 0
            to: 360
            loops: Animation.Infinite
        }

        SequentialAnimation on cameraYRotation {
            running: true
            loops: Animation.Infinite
            NumberAnimation {
                duration: yRotationTime
                from: yMin
                to: yMax
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                duration: yRotationTime
                from: yMax
                to: yMin
                easing.type: Easing.InOutQuad
            }
        }

        importScene: Node {
            Model {
                scale: Qt.vector3d(0.01, 0.01, 0.01)
                source: "#Sphere"
                x: 2.5
                z: 2
                y: 1
                castsReflections: false
                receivesReflections: true
                materials: [
                    PrincipledMaterial {
                        baseColor: "gold"
                        metalness: 1.0
                        roughness: 0.1
                    }
                ]
                ReflectionProbe {
                    // debugView: true
                    boxSize: Qt.vector3d(6, 3, 5)
                    boxOffset: Qt.vector3d(-1.5, -1, -1.5)
                    parallaxCorrection: true
                    quality: ReflectionProbe.High
                }
            }
            Model {
                scale: Qt.vector3d(0.01, 0.01, 0.01)
                source: "#Sphere"
                x: -2.5
                z: -2
                y: 1
                castsReflections: false
                receivesReflections: true
                materials: [
                    PrincipledMaterial {
                        baseColor: "white"
                        metalness: 0.0
                        roughness: 0.0
                        transmissionFactor: 1.0
                        thicknessFactor: 50
                    }
                ]
            }
        }

        environment: ExtendedSceneEnvironment {
            aoEnabled: true
            aoDither: true
            ditheringEnabled: true
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData {
                    groundBottomColor: "black"
                    skyTopColor: "white"
                }
            }
            backgroundMode: SceneEnvironment.SkyBox
            // glowEnabled: true
            // glowStrength: 0.5
            // glowBloom: 1.0
            // glowBlendMode: ExtendedSceneEnvironment.GlowBlendMode.Additive
            // lensFlareEnabled: true
            // lensFlareBloomScale: 1
            // lensFlareGhostCount: 5//10
            // lensFlareGhostDispersal: 0.25
            // lensFlareApplyStarburstTexture: true
            // //lensFlareBloomBias: 0.4 // Needed if using Default optimzationHint
            // lensFlareBloomBias: 0.5 // Needed if using Default optimzationHint and live video data
            lensFlareEnabled: true
            // lensFlareBloomScale: 1
            lensFlareGhostCount: 10
            // lensFlareGhostDispersal: 0.25
            lensFlareApplyStarburstTexture: true
            lensFlareBloomBias: 0.4 // Needed if using Default optimzationHint
        }

        // Use these for live video data
        Bar3DSeries {
            id: videoSeries
            baseGradient: liveDataGradient
            mesh: Abstract3DSeries.Mesh.Bar
            Gradient {
                id: liveDataGradient
                GradientStop { position: 1.0; color: "gold" }
                GradientStop { position: 0.33; color: "#000000" }
                GradientStop { position: 0.0; color: "#FF0000" }
            }
        }

        Component.onCompleted: {
            dataGenerator.setSeries(videoSeries);
            dataGenerator.start();
        }

        /*
        // Use these for non-live data, which animates the valueaxis max
        Bar3DSeries {
            id: secondarySeries
            visible: true
            itemLabelFormat: "Expenses, @colLabel, @rowLabel: -@valueLabel"
            baseGradient: secondaryGradient

            ItemModelBarDataProxy {
                id: secondaryProxy
                itemModel: graphData.model
                rowRole: "timestamp"
                columnRole: "timestamp"
                valueRole: "expenses"
                rowRolePattern: /^(\d\d\d\d).*$/
                columnRolePattern: /^.*-(\d\d)$/
                valueRolePattern: /-/
                rowRoleReplace: "\\1"
                columnRoleReplace: "\\1"
                multiMatchBehavior: ItemModelBarDataProxy.MultiMatchBehavior.Cumulative
            }

            Gradient {
                id: secondaryGradient
                GradientStop { position: 1.0; color: "#FF0000" }
                GradientStop { position: 0.0; color: "#000000" }
            }
        }

        Bar3DSeries {
            id: barSeries
            itemLabelFormat: "Income, @colLabel, @rowLabel: @valueLabel"
            baseGradient: barGradient

            ItemModelBarDataProxy {
                id: modelProxy
                itemModel: graphData.model
                rowRole: "timestamp"
                columnRole: "timestamp"
                valueRole: "income"
                rowRolePattern: /^(\d\d\d\d).*$/
                columnRolePattern: /^.*-(\d\d)$/
                rowRoleReplace: "\\1"
                columnRoleReplace: "\\1"
                multiMatchBehavior: ItemModelBarDataProxy.MultiMatchBehavior.Cumulative
            }

            Gradient {
                id: barGradient
                GradientStop { position: 1.0; color: "#00FF00" }
                GradientStop { position: 0.0; color: "#000000" }
            }
        }
        */
    }
}
