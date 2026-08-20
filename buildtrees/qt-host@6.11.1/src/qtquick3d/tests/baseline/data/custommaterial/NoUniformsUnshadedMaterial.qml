import QtQuick3D
import QtQuick

CustomMaterial {
    shadingMode: CustomMaterial.Unshaded
    vertexShader: "customunshaded_no_uniforms.vert"
    fragmentShader: "customunshaded_no_uniforms.frag"
}
