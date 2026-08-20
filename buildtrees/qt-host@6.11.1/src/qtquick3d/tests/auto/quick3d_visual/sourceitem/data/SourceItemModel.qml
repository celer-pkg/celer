import QtQuick
import QtQuick3D

Model {
    id: rootModel
    required property Item sourceItem;
    source: "#Rectangle"
    materials: PrincipledMaterial {
        baseColorMap: Texture {
            sourceItem: rootModel.sourceItem
        }
        lighting: PrincipledMaterial.NoLighting
    }
}
