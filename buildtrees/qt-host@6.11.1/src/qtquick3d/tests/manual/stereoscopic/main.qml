import QtQuick
import QtQuick.Window
import QtQuick3D

Window {
    visible: true
    width: 800
    height: 480
    title: qsTr("Stereoscopic Rendering Example")
    color: "#404040"
    //visibility: Window.FullScreen

    Item {
        focus: true
        Keys.onPressed: {
            if (event.key === Qt.Key_0) {
                stereoCamera.stereoMode = 0;
            }
            if (event.key === Qt.Key_1) {
                stereoCamera.stereoMode = 1;
            }
            if (event.key === Qt.Key_2) {
                stereoCamera.stereoMode = 2;
            }
            if (event.key === Qt.Key_3) {
                stereoCamera.stereoMode = 3;
            }
            if (event.key === Qt.Key_4) {
                stereoCamera.stereoMode = 4;
            }
            if (event.key === Qt.Key_Plus) {
                stereoCamera.eyeSeparation += 1;
            }
            if (event.key === Qt.Key_Minus) {
                stereoCamera.eyeSeparation -= 1;
            }
        }
    }

    // Scene
    SceneNode {
        id: sceneRoot

        Node {
            id: stereoCamera

            property real convergence: 600.0
            property real eyeSeparation: 35.0
            property real fieldOfView: 60.0
            property real nearPlane: 10.0
            property real farPlane: 2000.0
            property real aspectRatio: leftEyeView.width / leftEyeView.height

            property real _fov2: Math.tan(fieldOfView * Math.PI / 180 * 0.5)
            property real top: nearPlane * _fov2
            property real a: aspectRatio * _fov2 * convergence

            // 0 = Mono, 1 = TopBottom, 2 = LeftRight, 3 = Anaglyp (red-cyan), 4 = Anaglyph (green-magenta)
            property int stereoMode: 1
            // True when views are separate, false when they need to be combined with shader
            readonly property bool separateViews: (stereoMode == 1 || stereoMode == 2)
            readonly property bool isStereo: (stereoMode != 0)
            readonly property alias monoCamera: sceneRoot.mainCamera

            FrustumCamera {
                id: leftEyeCamera
                x: stereoCamera.monoCamera.x - stereoCamera.eyeSeparation * 0.5
                y: stereoCamera.monoCamera.y
                z: stereoCamera.monoCamera.z
                clipNear: stereoCamera.nearPlane
                clipFar: stereoCamera.farPlane
                left: -(stereoCamera.a - stereoCamera.eyeSeparation * 0.5) * stereoCamera.nearPlane / stereoCamera.convergence
                right: (stereoCamera.a + stereoCamera.eyeSeparation * 0.5) * stereoCamera.nearPlane / stereoCamera.convergence
                top: stereoCamera.top
                bottom: -stereoCamera.top
            }
            FrustumCamera {
                id: rightEyeCamera
                x: stereoCamera.monoCamera.x + stereoCamera.eyeSeparation * 0.5
                y: stereoCamera.monoCamera.y
                z: stereoCamera.monoCamera.z
                clipNear: stereoCamera.nearPlane
                clipFar: stereoCamera.farPlane
                left: -(stereoCamera.a + stereoCamera.eyeSeparation * 0.5) * stereoCamera.nearPlane / stereoCamera.convergence
                right: (stereoCamera.a - stereoCamera.eyeSeparation * 0.5) * stereoCamera.nearPlane / stereoCamera.convergence
                top: stereoCamera.top
                bottom: -stereoCamera.top
            }
        }
    }

    View3D {
        id: monoView
        width: parent.width
        height: parent.height
        importScene: sceneRoot
        camera: stereoCamera.monoCamera
        visible: !stereoCamera.isStereo
        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
        }
    }

    View3D {
        id: leftEyeView
        width: parent.width
        height: parent.height
        importScene: sceneRoot
        camera: leftEyeCamera
        layer.enabled: true
        layer.smooth: true
        layer.format: ShaderEffectSource.RGBA
        visible: stereoCamera.isStereo && stereoCamera.separateViews
        transform: Scale {
            xScale: (stereoCamera.stereoMode == 2) ? 0.5 : 1.0
            yScale: (stereoCamera.stereoMode == 1) ? 0.5 : 1.0
        }
        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
        }
    }

    View3D {
        id: rightEyeView
        width: parent.width
        height: parent.height
        x: (stereoCamera.stereoMode == 2) ? parent.width/2 : 0
        y: (stereoCamera.stereoMode == 1) ? parent.height/2 : 0
        importScene: sceneRoot
        camera: rightEyeCamera
        layer.enabled: true
        layer.smooth: true
        layer.format: ShaderEffectSource.RGBA
        visible: stereoCamera.isStereo && stereoCamera.separateViews
        transform: Scale {
            xScale: (stereoCamera.stereoMode == 2) ? 0.5 : 1.0
            yScale: (stereoCamera.stereoMode == 1) ? 0.5 : 1.0
        }
        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
        }
    }

    ShaderEffect {
        id: outputView
        anchors.fill: parent
        property variant leftEye: leftEyeView
        property variant rightEye: rightEyeView
        property alias stereoMode: stereoCamera.stereoMode
        blending: true
        visible: stereoCamera.isStereo && !stereoCamera.separateViews
        fragmentShader: "anaglyph.frag.qsb"
    }
}
