import QtQuick
import QtQuick3D

SpotLight {
    color: "#ff0000"
    position: Qt.vector3d(0, 250, 0)
    eulerRotation.x: -90
    shadowMapFar: 2000
    shadowMapQuality: Light.ShadowMapQualityHigh
    coneAngle: 30
    innerConeAngle: 10
}
