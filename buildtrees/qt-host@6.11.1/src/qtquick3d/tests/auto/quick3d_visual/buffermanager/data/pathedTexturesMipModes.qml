import QtQuick
import QtQuick3D

View3D {
    width: 640
    height: 480
    id: view1
    anchors.fill: parent

    PerspectiveCamera {
        id: camera1
    }

    // There should be 3 texture buffers created even though
    // there is only 1 source image because each mip mode
    // will create a unique image

    // MipModeNone
    Texture {
        id: tex1
        source: "noise1.jpg"
        generateMipmaps: false
    }

    // MipModeGenerated
    Texture {
        id: tex2
        source: "noise1.jpg"
        generateMipmaps: true
    }

    environment: SceneEnvironment {
        // MipModeBsdf
        lightProbe: Texture {
            id: tex3
            source: "noise1.jpg"
        }
    }

    Model {
        source: "#Cube"
        materials: DefaultMaterial {
            diffuseMap: tex1
        }
    }

    Model {
        source: "#Cube"
        materials: DefaultMaterial {
            diffuseMap: tex2
            
        }
    }

}