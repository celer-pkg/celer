import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick3D.Helpers

Window {
    visible: true
    width: 640
    height: 480
    Item {
        anchors.fill: parent
        View3D {
            id: view
            anchors.fill: parent

            Node {
                id: scene
                PerspectiveCamera {

                }
                PointLight {

                }
                PrincipledMaterial {
                    id: material
                    baseColorMap: Texture {
                        source: "qt_logo_rect.png"
                    }
                }

                Model {
                    source: "#Cube"
                    materials: [material]
                }

                Model {
                    geometry: GridGeometry {

                    }

                    materials: [material]
                }
            }

            ParticleSystem3D {
                id: system
                running: true
                SpriteParticle3D {
                    id: sprite
                    color: "white"
                    maxAmount: 200
                }
                ParticleEmitter3D {
                    particle: sprite
                    emitRate: 10
                }
            }
        }
    }
}
