import QtQuick
import QtQuick3D

View3D {
    width: 640
    height: 480

    environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.Color
        clearColor: "black"
    }

    PerspectiveCamera {
        id: camera
        z: 600
    }

    Model {
        source: "#Cube"
        scale: Qt.vector3d(2, 2, 2)
        eulerRotation.x: 30
        materials: PrincipledMaterial { }
    }

    Model {
        source: "#Rectangle"
        y: -150
        scale: Qt.vector3d(6, 6, 6)
        eulerRotation.x: -70
        materials: PrincipledMaterial { }
    }

    Node {
        id: lightContainer
    }

    function addDirectionalLight() {
        var comp = Qt.createComponent("TestDirLight.qml");
        return comp.createObject(lightContainer, {
            "shadowFactor": 80
        });
    }

    function addShadowCastingDirectionalLight() {
        var comp = Qt.createComponent("TestDirLight.qml");
        return comp.createObject(lightContainer, {
            "castsShadow": true,
            "eulerRotation.x": -90,
            "shadowFactor": 100
        });
    }

    function moveCamera() {
        camera.position = Qt.vector3d(0, 400, 600);
        camera.eulerRotation.x = -30;
        camera.clipFar = 2000;
    }

    function addPointLight() {
        var comp = Qt.createComponent("TestPointLight.qml");
        return comp.createObject(lightContainer, {});
    }

    function addSpotLight() {
        var comp = Qt.createComponent("TestSpotLight.qml");
        return comp.createObject(lightContainer, {});
    }
}
