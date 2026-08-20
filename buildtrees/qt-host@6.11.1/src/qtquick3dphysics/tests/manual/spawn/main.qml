// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: appWindow
    width: 1280
    height: 720
    visible: true
    title: qsTr("PhysX Physics Demo")

    PhysicsWorld {
        id: physicsWorld
        gravity: Qt.vector3d(0, -9.81, 0)
        running: false
        forceDebugDraw: true
        minimumTimestep: minTimestepSlider.value
        maximumTimestep: maxTimestepSlider.value
        typicalLength: 1
        typicalSpeed: 10
        scene: viewport.scene
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
        }

        focus: true
        Keys.onSpacePressed: {
            console.log("toggle physics")
            physicsWorld.running = !physicsWorld.running
        }

        Node {
            id: scene1
            PerspectiveCamera {
                id: camera1
                position: Qt.vector3d(-2, 1, 5)
                eulerRotation : Qt.vector3d(-20, -20, 0)
                clipFar: 50
                clipNear: 0.01
            }

            DirectionalLight {
                eulerRotation: Qt.vector3d(-45, 45, 0)
                castsShadow: true
                brightness: 1
                shadowMapQuality: Light.ShadowMapQualityVeryHigh
                shadowBias: 0.05
                pcfFactor: 0.005
            }

            TriggerBody {
                id: movingArea
                position: Qt.vector3d(4, -0.25, 1)

                onBodyEntered: (body) => {
                                   if (body.hasOwnProperty('inArea'))
                                       body.inArea = true;
                               }
                onBodyExited: (body) => {
                                  if (body.hasOwnProperty('inArea'))
                                      body.inArea = false;
                              }

                Model {
                    source: "#Cube"
                    scale: Qt.vector3d(0.02, 0.015, 0.02)
                    materials: PrincipledMaterial {
                        baseColor: Qt.rgba(0, 0, 1, 0.2)
                        alphaMode: PrincipledMaterial.Blend
                    }
                }

                SequentialAnimation {
                    running: true
                    paused: !physicsWorld.running
                    loops: -1
                    NumberAnimation {
                        target: movingArea
                        property: "position.x"
                        duration: 5000
                        from: 4
                        to: -4
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: movingArea
                        property: "position.x"
                        duration: 5000
                        from: -4
                        to: 4
                        easing.type: Easing.InOutQuad
                    }
                }

                collisionShapes: BoxShape {
                    extents: Qt.vector3d(2, 1.5, 2)
                }
            }

            DynamicRigidBody {
                id: impeller
                position: Qt.vector3d(-3, -0.5, 0)
                isKinematic: true
                receiveContactReports: true

                SequentialAnimation {
                     running: physicsWorld.running
                     loops: Animation.Infinite;
                     NumberAnimation {target: impeller; property: "kinematicPosition.x"; from: -3; to:  3; duration: 2000 }
                     NumberAnimation {target: impeller; property: "kinematicPosition.x"; from:  3; to: -3; duration: 2000 }
                }

                Model {
                    source: "#Sphere"
                    materials: PrincipledMaterial {
                       baseColor: "yellow"
                    }
                    scale: Qt.vector3d(0.01, 0.01, 0.01)
                }

                collisionShapes: SphereShape {
                    diameter: 1
                }

                onBodyContact: (body, positions, impulses, normals) => {
                    for (var normal of normals) {
                        let force = normal.times(-0.001);
                        body.applyCentralImpulse(force);
                    }
                }
            }

            StaticRigidBody {
                id: floor
                position: Qt.vector3d(0, -1, 0)
                eulerRotation: Qt.vector3d(-90, 0, 0)
                collisionShapes: PlaneShape {}
                Model {
                    source: "#Rectangle"
                    scale: Qt.vector3d(5, 5, 1)
                    materials: PrincipledMaterial {
                        baseColor: "green"
                    }
                    castsShadows: false
                    receivesShadows: true
                }
            }

            StaticRigidBody {
                id: leftWall
                eulerRotation: Qt.vector3d(0, 90, 0)
                position: Qt.vector3d(-15, 3, 0)
                collisionShapes: PlaneShape {
                    scale: Qt.vector3d(0.2, 0.2, 0.2)
                }
            }

            StaticRigidBody {
                id: rightWall
                position: Qt.vector3d(15, 3, 0)
                eulerRotation: Qt.vector3d(0, 270, 0)
                collisionShapes: PlaneShape {
                    scale: Qt.vector3d(0.2, 0.2, 0.2)
                }
            }

            StaticRigidBody {
                id: backWall
                position: Qt.vector3d(0, 3, -15)
                collisionShapes: PlaneShape {
                    scale: Qt.vector3d(0.2, 0.2, 0.2)
                }
            }
            StaticRigidBody {
                id: frontWall
                position: Qt.vector3d(0, 3, 10)
                eulerRotation: Qt.vector3d(0, 180, 0)
                collisionShapes: PlaneShape {
                    scale: Qt.vector3d(0.2, 0.2, 0.2)
                }
            }
        }

        Node {
            id: shapeSpawner
            property var instancesBoxes: []
            property var instancesSpheres: []

            function createBox() {
                var component = Qt.createComponent("box.qml");
                let box = component.createObject(scene1, {y: 3});
                instancesBoxes.push(box);

                if (box === null) {
                    // Error Handling
                    console.log("Error creating object");
                }
            }

            function deleteBox() {
                if (instancesBoxes.length > 0) {
                    let box = instancesBoxes.pop();
                    box.destroy();
                }
            }

            function updateBoxes() {
                while (numberBoxes.value > instancesBoxes.length) {
                    createBox();
                }
                while (numberBoxes.value < instancesBoxes.length) {
                    deleteBox();
                }
                updateLocks();
            }

            function doImpulse() {
                for (var i = 0; i < instancesBoxes.length; i++) {
                    let box = instancesBoxes[i];
                    box.applyCentralImpulse(Qt.vector3d(0,0.01,0));
                }
                for (i = 0; i < instancesSpheres.length; i++) {
                    let sphere = instancesSpheres[i];
                    sphere.applyCentralImpulse(Qt.vector3d(0,0.01,0));
                }
            }

            //// Sphere

            function createSphere() {
                var component = Qt.createComponent("sphere.qml");
                let sphere = component.createObject(scene1, {y: 3});
                instancesSpheres.push(sphere);

                if (sphere === null) {
                    // Error Handling
                    console.log("Error creating object");
                }
            }

            function deleteSphere() {
                if (instancesSpheres.length > 0) {
                    let sphere = instancesSpheres.pop();
                    sphere.destroy();
                }
            }

            function updateSpheres() {
                while (numberSpheres.value > instancesSpheres.length) {
                    createSphere();
                }
                while (numberSpheres.value < instancesSpheres.length) {
                    deleteSphere();
                }

                updateLocks();
            }

            function updateLocks() {
                var i;
                for (i = 0; i < instancesSpheres.length; i++) {
                    let sphere = instancesSpheres[i];
                    sphere.angularAxisLock =
                            (lockAngularX.checked ? DynamicRigidBody.LockX : 0) |
                            (lockAngularY.checked ? DynamicRigidBody.LockY : 0) |
                            (lockAngularZ.checked ? DynamicRigidBody.LockZ : 0);
                    sphere.linearAxisLock =
                            (lockLinearX.checked ? DynamicRigidBody.LockX : 0) |
                            (lockLinearY.checked ? DynamicRigidBody.LockY : 0) |
                            (lockLinearZ.checked ? DynamicRigidBody.LockZ : 0);
                }
                for (i = 0; i < instancesBoxes.length; i++) {
                    let box = instancesBoxes[i];
                    box.angularAxisLock =
                            (lockAngularX.checked ? DynamicRigidBody.LockX : 0) |
                            (lockAngularY.checked ? DynamicRigidBody.LockY : 0) |
                            (lockAngularZ.checked ? DynamicRigidBody.LockZ : 0);
                    box.linearAxisLock =
                            (lockLinearX.checked ? DynamicRigidBody.LockX : 0) |
                            (lockLinearY.checked ? DynamicRigidBody.LockY : 0) |
                            (lockLinearZ.checked ? DynamicRigidBody.LockZ : 0);
                }
            }

            function setKinematic() {
                for (var i = 0; i < instancesSpheres.length; i++) {
                    let sphere = instancesSpheres[i];
                    sphere.isKinematic = isKinematicCheck.checked;
                }
                for (i = 0; i < instancesBoxes.length; i++) {
                    let box = instancesBoxes[i];
                    box.isKinematic = isKinematicCheck.checked;
                }
            }
        }
    }


    WasdController {
        keysEnabled: true
        property bool controllingUnit: false
        controlledObject: camera1
        speed: 0.02
        shiftSpeed: 0.2

        Keys.onPressed: (event)=> {
            if (keysEnabled) handleKeyPress(event);
            if (event.key === Qt.Key_Space) {
                physicsWorld.running = !physicsWorld.running
            } else if (event.key === Qt.Key_H) {
                physicsWorld.forceDebugDraw = !physicsWorld.forceDebugDraw
            }
            else if (event.key === Qt.Key_J) {
                floor.eulerRotation.x = (360 + floor.eulerRotation.x - 1) % 360
            }
            else if (event.key === Qt.Key_K) {
                floor.eulerRotation.x = floor.eulerRotation.x + 1 % 360
            }
        }
        Keys.onReleased: (event)=> { if (keysEnabled) handleKeyRelease(event) }
    }

    ColumnLayout {
        Label {
            text: "Number of boxes: " + numberBoxes.value
        }
        Slider {
            id: numberBoxes
            focusPolicy: Qt.NoFocus
            from: 0
            to: 20
            value: 0
            stepSize: 1
            onMoved: { shapeSpawner.updateBoxes() }
        }

        Label {
            text: "Number of spheres: " + numberSpheres.value
        }
        Slider {
            id: numberSpheres
            focusPolicy: Qt.NoFocus
            from: 0
            to: 20
            value: 0
            stepSize: 1
            onMoved: { shapeSpawner.updateSpheres() }
        }


        CheckBox {
            id: lockAngularX
            text: qsTr("Lock angular X")
            onCheckedChanged: shapeSpawner.updateLocks()
        }

        CheckBox {
            id: lockAngularY
            text: qsTr("Lock angular Y")
            onCheckedChanged: shapeSpawner.updateLocks()
        }

        CheckBox {
            id: lockAngularZ
            text: qsTr("Lock angular Z")
            onCheckedChanged: shapeSpawner.updateLocks()
        }

        CheckBox {
            id: lockLinearX
            text: qsTr("Lock linear X")
            onCheckedChanged: shapeSpawner.updateLocks()
        }

        CheckBox {
            id: lockLinearY
            text: qsTr("Lock linear Y")
            onCheckedChanged: shapeSpawner.updateLocks()
        }

        CheckBox {
            id: lockLinearZ
            text: qsTr("Lock linear Z")
            onCheckedChanged: shapeSpawner.updateLocks()
        }

        CheckBox {
            id: isKinematicCheck
            text: qsTr("Is Kinematic")
            onCheckedChanged: shapeSpawner.setKinematic()
        }

        Button {
            text:  "Add impulse"
            onClicked: shapeSpawner.doImpulse()
        }

        Label {
            text: "Min timestep: " + minTimestepSlider.value
        }
        Slider {
            id: minTimestepSlider
            focusPolicy: Qt.NoFocus
            from: 1
            to: 100
            value: 5
        }

        Label {
            text: "Max timestep: " + maxTimestepSlider.value
        }
        Slider {
            id: maxTimestepSlider
            focusPolicy: Qt.NoFocus
            from: 1
            to: 100
            value: 50
        }

        Label {
            text: "Debug view (forced): " + physicsWorld.forceDebugDraw
        }
    }
}
