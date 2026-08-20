import QtQuick
import QtQuick3D

CustomMaterial {
    property TextureInput baseColor: TextureInput {
        texture: Texture {
            source: "../shared/maps/texture_withAlpha.png"
            generateMipmaps: true
            mipFilter: Texture.Linear
        }
    }
    sourceBlend: CustomMaterial.SrcAlpha
    destinationBlend: CustomMaterial.OneMinusSrcAlpha
    cullMode: CustomMaterial.BackFaceCulling

    fragmentShader: "depthdraw.frag"
}
