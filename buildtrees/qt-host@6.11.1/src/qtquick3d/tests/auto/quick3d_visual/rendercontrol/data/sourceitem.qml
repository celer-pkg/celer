import QtQuick
import QtQuick3D

Item {
    width: 640
    height: 480

    function makeThirdReferToStandaloneSourceItem() {
        thirdTexture.sourceItem = standaloneSourceItem;
    }

    function makeThirdReferToExplicitLayerBasedSourceItem() {
        thirdTexture.sourceItem = explicitLayerBasedSourceItem;
    }

    function makeThirdReferToImageSourceItem() {
        thirdTexture.sourceItem = imageSourceItem;
    }

    function makeThirdReferToSemiTransparentSourceItem() {
        thirdTexture.sourceItem = semiTransparentSourceItem;
    }

    function makeSemiTransparentSourceItemSmaller() {
        semiTransparentSourceItem.width /= 2;
        semiTransparentSourceItem.height /= 2;
    }

    Item {
        id: standaloneSourceItem
        visible: false
        width: 100; height: 100
        Rectangle {
            anchors.fill: parent
            color: "red"
        }
    }

    Rectangle {
        id: explicitLayerBasedSourceItem
        layer.enabled: true
        visible: false
        width: 100; height: 100
        color: "gray"
    }

    Image {
        id: imageSourceItem
        visible: false
        source: "qt_logo_rect.png"
    }

    Item {
        id: semiTransparentSourceItem
        visible: false
        width: 100; height: 100
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            Rectangle {
                width: 25; height: 25
                anchors.centerIn: parent
                color: "blue"
            }
        }
    }

    View3D {
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
        }
        PerspectiveCamera { z: 600 }
        Model {
            source: "#Sphere"
            x: -200
            materials: PrincipledMaterial {
                lighting: PrincipledMaterial.NoLighting
                baseColorMap: Texture {
                    sourceItem: standaloneSourceItem
                }
            }
        }
        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                lighting: PrincipledMaterial.NoLighting
                baseColorMap: Texture {
                    sourceItem: Rectangle {
                        width: 100; height: 100
                        color: "green"
                    }
                }
            }
        }
        Model {
            source: "#Sphere"
            x: 200
            materials: DefaultMaterial {
                lighting: PrincipledMaterial.NoLighting
                diffuseMap: Texture {
                    id: thirdTexture
                    sourceItem: Rectangle {
                        width: 100; height: 100
                        color: "blue"
                    }
                }
            }
        }
    }
}
