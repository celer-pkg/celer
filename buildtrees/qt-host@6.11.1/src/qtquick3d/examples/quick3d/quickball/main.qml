// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick3D
import QtQuick.Particles
import QtQuick3D.Particles3D
import QtQuick.Controls

Window {
    id: mainWindow

    //! [main properties]
    // Scaling helpper
    readonly property real px: 0.2 + Math.min(width, height) / 800
    // This is false until the first game has started
    property bool playingStarted: false
    // This is true whenever game is on
    property bool gameOn: false
    // Sizes of our 3D models
    readonly property real ballSize: 40
    readonly property real targetSize: 120
    // Playing time in seconds
    readonly property real gameTime: 60
    property real currentTime: 0
    // Amount of balls per game
    readonly property int gameBalls: 20
    property int currentBalls: 0
    // Scores
    property int score: 0
    property int timeBonus: 0
    property int ballsBonus: 0
    //! [main properties]

    width: 800
    height: 600
    visible: true
    title: qsTr("Quick3D Quick Ball")
    color: "#000000"

    View3D {
        id: view3D
        anchors.fill: parent

        //! [view functions]
        function createLevel1() {
            // Simple level of target items
            var level1 = [{ "x": 0, "y": 100, "z": -100, "points": 10 },
                          { "x": -300, "y": 100, "z": -400, "points": 10 },
                          { "x": 300, "y": 100, "z": -400, "points": 10 },
                          { "x": -200, "y": 400, "z": -600, "points": 20 },
                          { "x": 0, "y": 400, "z": -600, "points": 20 },
                          { "x": 200, "y": 400, "z": -600, "points": 20 },
                          { "x": 0, "y": 700, "z": -600, "points": 30 }];
            targetsNode.addTargets(level1);
        }

        function startGame() {
            ballModel.resetBall();
            targetsNode.resetTargets();
            createLevel1();
            mainWindow.score = mainWindow.timeBonus = mainWindow.ballsBonus = 0;
            mainWindow.currentBalls = mainWindow.gameBalls;
            mainWindow.gameOn = true;
            mainWindow.playingStarted = true;
        }

        function endGame() {
            if (targetsNode.currentTargets == 0) {
                // If we managed to get all targets down -> bonus points!
                mainWindow.timeBonus = mainWindow.currentTime;
                mainWindow.ballsBonus = mainWindow.currentBalls * 10;
            }
            mainWindow.gameOn = false;
            ballModel.resetBall();
        }
        //! [view functions]

        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        camera: viewCamera

        PerspectiveCamera {
            id: viewCamera
            position: Qt.vector3d(0, 200, 800);

            // Rotate camera a bit
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                NumberAnimation {
                    to: 2
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: -2
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
            }
        }

        //! [lights]
        PointLight {
            x: 400
            y: 1200
            castsShadow: true
            shadowMapQuality: Light.ShadowMapQualityHigh
            shadowFactor: 50
            quadraticFade: 2
            ambientColor: "#202020"
            brightness: mainWindow.gameOn ? 200 : 40
            Behavior on brightness {
                NumberAnimation {
                    duration: 1000
                    easing.type: Easing.InOutQuad
                }
            }
        }
       //! [lights]

        //! [ball handling]
        MouseArea {
            anchors.fill: parent
            enabled: mainWindow.gameOn && !ballModel.ballMoving
            onPressed: {
                ballModel.moveBall(mouseX, mouseY);
            }
            onPositionChanged: {
                ballModel.moveBall(mouseX, mouseY);
            }
            onReleased: {
                ballModel.throwBall();
            }
        }
        //! [ball handling]

        //! [ball model]
        Model {
            id: ballModel
            property real directionX: 0
            property real directionY: 0
            // How many ms the ball flies
            readonly property real speed: 2000
            readonly property real ballScale: mainWindow.ballSize / 100
            property var moves: []
            readonly property int maxMoves: 5
            readonly property bool ballMoving: ballAnimation.running

            source: "#Sphere"
            scale: Qt.vector3d(ballScale, ballScale, ballScale)

            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: "images/ball.jpg"
                }
                normalMap: Texture {
                    source: "images/ball_n.jpg"
                }
                normalStrength: 1.0
            }
            //! [ball model]

            //! [ball functions]
            function resetBall() {
                moves = [];
                x = 0;
                y = mainWindow.ballSize/2;
                z = 400;
            }

            function moveBall(posX, posY) {
                if (!mainWindow.gameOn) return;
                var pos = view3D.mapTo3DScene(Qt.vector3d(posX, posY, ballModel.z + mainWindow.ballSize));
                pos.y = Math.max(mainWindow.ballSize / 2, pos.y);
                var point = {"x": pos.x, "y": pos.y };
                moves.push(point);
                if (moves.length > maxMoves) moves.shift();
                // Apply position into ball model
                ballModel.x = pos.x;
                ballModel.y = pos.y;
            }

            function throwBall() {
                if (!mainWindow.gameOn) return;
                mainWindow.currentBalls--;
                var moveX = 0;
                var moveY = 0;
                if (moves.length >= 2) {
                    var first = moves.shift();
                    var last = moves.pop();
                    moveX = last.x - first.x;
                    moveY = last.y - first.y;
                    if (moveY < 0) moveY = 0;
                }
                directionX = moveX * 20;
                directionY = moveY * 4;
                ballAnimation.start();
            }
            //! [ball functions]

            //! [ball animations]
            ParallelAnimation {
                id: ballAnimation
                running: false
                // Move forward
                NumberAnimation {
                    target: ballModel
                    property: "z"
                    duration: ballModel.speed
                    to: -ballModel.directionY * 5
                    easing.type: Easing.OutQuad
                }
                // Move up & down with a bounce
                SequentialAnimation {
                    NumberAnimation {
                        target: ballModel
                        property: "y"
                        duration: ballModel.speed * (1 / 3)
                        to: ballModel.y + ballModel.directionY
                        easing.type: Easing.OutQuad
                    }
                    NumberAnimation {
                        target: ballModel
                        property: "y"
                        duration: ballModel.speed * (2 / 3)
                        to: mainWindow.ballSize / 4
                        easing.type: Easing.OutBounce
                    }
                }
                // Move sideways
                NumberAnimation {
                    target: ballModel
                    property: "x"
                    duration: ballModel.speed
                    to: ballModel.x + ballModel.directionX
                }

                onFinished: {
                    if (mainWindow.currentBalls <= 0)
                        view3D.endGame();
                    ballModel.resetBall();
                }
            }

            NumberAnimation on eulerRotation.z {
                running: ballModel.ballMoving
                loops: Animation.Infinite
                from: ballModel.directionX < 0 ? 0 : 720
                to: 360
                duration: 10000 / (2 + Math.abs(ballModel.directionX * 0.05))
            }
            //! [ball animations]

            //! [ball collisions]
            onZChanged: {
                // Loop through target items and detect collisions
                var hitMargin = mainWindow.ballSize / 2 + mainWindow.targetSize / 2;
                for (var i = 0; i < targetsNode.targets.length; ++i) {
                    var target = targetsNode.targets[i];
                    var targetPos = target.scenePosition;
                    var hit = ballModel.scenePosition.fuzzyEquals(targetPos, hitMargin);
                    if (hit) {
                        target.hit();
                        if (targetsNode.currentTargets <= 0)
                            view3D.endGame();
                    }
                }
            }
            //! [ball collisions]
        }

        //! [targets node]
        Node {
            id: targetsNode

            property var targets: []
            property int currentTargets: 0

            function addTargets(items) {
                items.forEach(function (item) {
                    let instance = targetComponent.createObject(
                            targetsNode, { "x": item.x, "startPosY": item.y, "z": item.z, "points": item.points});
                    targets.push(instance);
                });
                currentTargets = targets.length;
            }

            function removeTarget(item) {
                var index = targets.indexOf(item);
                targets.splice(index, 1);
                currentTargets = targets.length;
            }

            function resetTargets() {
                while (targets.length > 0)
                    targets.pop().destroy();
                currentTargets = targets.length;
            }
        }
        //! [targets node]

        //! [target component]
        Component {
            id: targetComponent
            Node {
                id: targetNode

                property int points: 0
                property real hide: 0
                property real startPosY: 0
                property real posY: 0
                property real pointsOpacity: 0

                function hit() {
                    targetsNode.removeTarget(this);
                    mainWindow.score += points;
                    hitAnimation.start();
                    var burstPos = targetNode.mapPositionToScene(Qt.vector3d(0, 0, 0));
                    hitParticleEmitter.burst(100, 200, burstPos);
                }

                y: startPosY + posY
                SequentialAnimation {
                    running: mainWindow.gameOn && !hitAnimation.running
                    loops: Animation.Infinite
                    NumberAnimation {
                        target: targetNode
                        property: "posY"
                        from: 0
                        to: 150
                        duration: 3000
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: targetNode
                        property: "posY"
                        to: 0
                        duration: 1500
                        easing.type: Easing.InOutQuad
                    }
                }

                SequentialAnimation {
                    id: hitAnimation
                    NumberAnimation {
                        target: targetNode
                        property: "hide"
                        to: 1
                        duration: 800
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: targetNode
                        property: "pointsOpacity"
                        to: 1
                        duration: 1000
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: targetNode
                        property: "pointsOpacity"
                        to: 0
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                    ScriptAction {
                        script: targetNode.destroy();
                    }
                }

                Model {
                    id: targetModel

                    readonly property real targetScale: (1 + targetNode.hide) * (mainWindow.targetSize / 100)

                    source: "#Cube"
                    scale: Qt.vector3d(targetScale, targetScale, targetScale)
                    opacity: 0.99 - targetNode.hide * 2
                    materials: PrincipledMaterial {
                        baseColorMap: Texture {
                            source: "images/qt_logo.jpg"
                        }
                        normalMap: Texture {
                            source: "images/qt_logo_n.jpg"
                        }
                        normalStrength: 1.0
                    }
                    Vector3dAnimation on eulerRotation {
                        loops: Animation.Infinite
                        duration: 5000
                        from: Qt.vector3d(0, 0, 0)
                        to: Qt.vector3d(360, 360, 360)
                    }
                }
                Text {
                    anchors.centerIn: parent
                    scale: 1 + targetNode.pointsOpacity
                    opacity: targetNode.pointsOpacity
                    text: targetNode.points
                    font.pixelSize: 60 * mainWindow.px
                    color: "#808000"
                    style: Text.Outline
                    styleColor: "#f0f000"
                }
            }
        }
        //! [target component]

        //! [ground model]
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(50, 50, 1)
            eulerRotation.x: -90
            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: "images/grass.jpg"
                    tilingModeHorizontal: Texture.Repeat
                    tilingModeVertical: Texture.Repeat
                    scaleU: 25.0
                    scaleV: 25.0
                }
                normalMap: Texture {
                    source: "images/grass_n.jpg"
                }
                normalStrength: 0.6
            }
        }
        //! [ground model]

        //! [sky model]
        Model {
            id: sky
            property real scaleX: 100
            property real scaleY: 20
            source: "#Rectangle"
            scale: Qt.vector3d(sky.scaleX, sky.scaleY, 1)
            position: Qt.vector3d(0, 960, -2000)
            // We don't want shadows casted into sky
            receivesShadows: false
            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: "images/sky.jpg"
                }
            }
            // Star particles
            Node {
                z: 500
                y: 30
                // Stars are far away, scale up to half the resolution
                scale: Qt.vector3d(2 / sky.scaleX, 2 / sky.scaleY, 1)
                ParticleSystem {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    width: 3000
                    height: 400
                    ImageParticle {
                        source: "qrc:///particleresources/star.png"
                        rotationVariation: 360
                        color: "#ffffa0"
                        colorVariation: 0.1
                    }
                    Emitter {
                        anchors.fill: parent
                        emitRate: 4
                        lifeSpan: 6000
                        lifeSpanVariation: 4000
                        size: 30
                        sizeVariation: 20
                    }
                }
            }
        }
        //! [sky model]

        //! [hit particles]
        ParticleSystem3D {
            id: psystem
            SpriteParticle3D {
                id: sprite
                sprite: Texture {
                    source: "images/particle.png"
                }
                color: Qt.rgba(1.0, 1.0, 0.0, 1.0)
                colorVariation: Qt.vector4d(0.4, 0.6, 0.0, 0.0)
                unifiedColorVariation: true
                maxAmount: 200
            }
            ParticleEmitter3D {
                id: hitParticleEmitter
                particle: sprite
                particleScale: 4.0
                particleScaleVariation: 2.0
                particleRotationVariation: Qt.vector3d(0, 0, 180)
                particleRotationVelocityVariation: Qt.vector3d(0, 0, 250)
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 300, 0)
                    directionVariation: Qt.vector3d(200, 150, 100)
                }
                lifeSpan: 800
                lifeSpanVariation: 200
                depthBias: 100
            }
            Gravity3D {
                magnitude: 600
            }
        }
        //! [hit particles]
    }

    // Game time counter
    NumberAnimation {
        target: mainWindow
        property: "currentTime"
        running: mainWindow.gameOn
        duration: mainWindow.gameTime * 1000
        from: mainWindow.gameTime
        to: 0
        onFinished: {
            view3D.endGame();
        }
    }

    // Show time, balls and score
    Item {
        width: parent.width
        height: 60 * mainWindow.px
        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 20 * mainWindow.px
            font.pixelSize: 26 * mainWindow.px
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            text: mainWindow.currentTime.toFixed(2)
        }
        Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 1 * mainWindow.px
            anchors.right: ballCountText.left
            anchors.rightMargin: 8 * mainWindow.px
            width: 26 * mainWindow.px
            height: width
            mipmap: true
            source: "images/ball_icon.png"
        }
        Text {
            id: ballCountText
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 20 * mainWindow.px
            font.pixelSize: 26 * mainWindow.px
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            text: mainWindow.currentBalls
        }
        Text {
            anchors.centerIn: parent
            font.pixelSize: 36 * mainWindow.px
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            text: mainWindow.score
        }
    }

    // Game logo
    Image {
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.6, sourceSize.width)
        height: width * 0.6
        fillMode: Image.PreserveAspectFit
        source: "images/quickball.png"
        opacity: !mainWindow.gameOn
        scale: 2.0 - opacity
        Behavior on opacity {
            NumberAnimation {
                duration: 400
                easing.type: Easing.InOutQuad
            }
        }
    }

    // Show bonus and total score when the game ends
    Item {
        property bool show: mainWindow.playingStarted && !mainWindow.gameOn

        anchors.centerIn: parent
        anchors.verticalCenterOffset: -200 * mainWindow.px
        onShowChanged: {
            if (show) {
                showScoreAnimation.start();
            } else {
                showScoreAnimation.stop();
                timeBonusText.opacity = 0;
                ballsBonusText.opacity = 0;
                totalScoreText.opacity = 0;
            }
        }

        SequentialAnimation {
            id: showScoreAnimation
            NumberAnimation {
                target: timeBonusText
                property: "opacity"
                to: 1
                duration: 1000
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: ballsBonusText
                property: "opacity"
                to: 1
                duration: 1000
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: totalScoreText
                property: "opacity"
                to: 1
                duration: 1000
                easing.type: Easing.InOutQuad
            }
        }

        Text {
            id: timeBonusText
            anchors.horizontalCenter: parent.horizontalCenter
            y: opacity * 60 * mainWindow.px
            font.pixelSize: 26 * mainWindow.px
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            textFormat: Text.StyledText
            text: qsTr("TIME BONUS <b>%1</b>").arg(mainWindow.timeBonus)
            opacity: 0
        }
        Text {
            id: ballsBonusText
            anchors.horizontalCenter: parent.horizontalCenter
            y: timeBonusText.y + opacity * 40 * mainWindow.px
            font.pixelSize: 26 * mainWindow.px
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            textFormat: Text.StyledText
            text: qsTr("BALLS BONUS <b>%1</b>").arg(mainWindow.ballsBonus)
            opacity: 0
        }
        Text {
            id: totalScoreText
            anchors.horizontalCenter: parent.horizontalCenter
            y: ballsBonusText.y + opacity * 60 * mainWindow.px
            font.pixelSize: 66 * mainWindow.px
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            textFormat: Text.StyledText
            text: qsTr("SCORE <b>%1</b>").arg(mainWindow.score + mainWindow.timeBonus + mainWindow.ballsBonus)
            opacity: 0
        }
    }

    RoundButton {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40 * mainWindow.px
        width: 140 * mainWindow.px
        height: 60 * mainWindow.px
        visible: !mainWindow.gameOn
        font.pixelSize: 26 * mainWindow.px
        text: qsTr("START")
        onClicked: {
            view3D.startGame();
        }
    }
}
