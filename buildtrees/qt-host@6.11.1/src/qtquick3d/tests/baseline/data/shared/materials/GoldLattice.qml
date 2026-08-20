import QtQuick
import QtQuick3D

PrincipledMaterial {

    Texture {
        id: baseColorTexture
        source: "GoldLattice_baseColor.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
    }
    Texture {
        id: occlusionRoughnessMetallicTexture
        source: "GoldLattice_occlusionRoughnessMetallic.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
    }
    Texture {
        id: normalTexture
        source: "GoldLattice_normal.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
    }
    Texture {
        id: heightTexture
        source: "GoldLattice_height.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
    }

    baseColor: Qt.rgba(1, 1, 1, 1)
    baseColorMap: baseColorTexture
    occlusionAmount: 1
    occlusionMap: occlusionRoughnessMetallicTexture
    occlusionChannel: Material.R
    roughness: 1
    roughnessMap: occlusionRoughnessMetallicTexture
    roughnessChannel: Material.G
    metalness: 1
    normalStrength: 1
    normalMap: normalTexture
    heightMap: heightTexture
    heightAmount: 0.1
}
