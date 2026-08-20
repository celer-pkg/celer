import QtQuick
import QtQuick3D

PointLight {
    color: "#00ff00"
    brightness: 10
    position: Qt.vector3d(0, 300, 0)
    shadowMapFar: 2000
    shadowMapQuality: Light.ShadowMapQualityHigh
}
