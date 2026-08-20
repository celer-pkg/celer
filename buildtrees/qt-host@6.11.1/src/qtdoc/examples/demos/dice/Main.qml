// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtSensors

ApplicationWindow {
    id: root
    width: 800
    height: 600
    visible: true
    property real currDrawerWidth: drawer.width * drawer.position

    Accelerometer {
        id: accelerometer
        dataRate: 25
        active: true
        readonly property vector3d force: {
            if (!reading)
                return Qt.vector3d(0, 0, 0)
            let r = reading as AccelerometerReading
            return Qt.vector3d(r.x, r.y, r.z)
        }
        readonly property bool highForce : Math.abs(force.length() - 9.81) > 16

        onHighForceChanged: {
            if (!highForce)
                shakeTimeout.restart()
        }
    }

    Timer {
        id: shakeTimeout
        interval: 200
        running: false
        repeat: false;
        onTriggered: {
            scene.rollForce = accelerometer.force.times(100.0)
            scene.spawnDice()
        }
    }

    Scene {
        id: scene
        width: parent.width
        height: parent.height

        settingsStaticFriction: staticFrictionSlider.value
        settingsDynamicFriction: dynamicFrictionSlider.value
        settingsRestitution: restitutionSlider.value
        settingsDiceWidth: diceSize.value
        settingsDiceCount: diceSlider.value
        cameraControllerEnabled: !drawer.visible

        Label {
            id: tapLabel
            anchors.fill: parent
            padding: parent.width * 0.1
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            fontSizeMode: Text.Fit
            font.bold: true
            font.pixelSize: 64
            text: qsTr("Tap, click or shake to throw dice")
            wrapMode: Text.NoWrap
            style: Text.Raised
            color: "white"
            minimumPixelSize: 10
            visible: opacity > 0

            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                    easing.type: Easing.OutCirc
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                tapLabel.opacity = 0
                scene.rollForce = Qt.vector3d(0, 0, 0)
                scene.spawnDice()
            }
        }

        function updateGravity() {
            if (gravityZero.checked) {
                scene.settingGravity = 0
            }
            if (gravityMoon.checked) {
                scene.settingGravity = 162
            }
            if (gravityMars.checked) {
                scene.settingGravity = 371
            }
            if (gravityEarth.checked) {
                scene.settingGravity = 980.7
            }
        }
    }

    Drawer {
        id: drawer
        height: root.height
        width: column.width + 40
        Flickable {
            anchors.fill: parent
            contentHeight: column.implicitHeight
            flickableDirection: Flickable.AutoFlickIfNeeded
            ScrollBar.vertical: ScrollBar {
                policy: Screen.height < 600 ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            }
            ColumnLayout {
                id: column
                anchors.horizontalCenter: parent.horizontalCenter

                // Static friction
                Label {
                    text: qsTr("Static friction: %1").arg(staticFrictionSlider.value.toFixed(2))
                    Layout.fillWidth: true
                }
                Slider {
                    id: staticFrictionSlider
                    focusPolicy: Qt.NoFocus
                    from: 0
                    to: 1
                    value: 0.5
                    stepSize: 0.05
                }

                // Dynamic friction
                Label {
                    text: qsTr("Dynamic friction: %1").arg(dynamicFrictionSlider.value.toFixed(2))
                    Layout.fillWidth: true
                }
                Slider {
                    id: dynamicFrictionSlider
                    focusPolicy: Qt.NoFocus
                    from: 0
                    to: 1
                    value: 0.4
                    stepSize: 0.05
                }

                // Restitution
                Label {
                    text: qsTr("Restitution: %1").arg(restitutionSlider.value.toFixed(2))
                    Layout.fillWidth: true
                }
                Slider {
                    id: restitutionSlider
                    focusPolicy: Qt.NoFocus
                    from: 0
                    to: 1
                    value: 0.4
                    stepSize: 0.05
                }

                // Gravity
                Label {
                    text: qsTr("Gravity")
                }
                GridLayout {
                    columns: 2
                    RadioButton {
                        id: gravityZero
                        text: qsTr("Zero")
                        onCheckedChanged: scene.updateGravity()
                    }
                    RadioButton {
                        id: gravityMoon
                        text: qsTr("Moon")
                        onCheckedChanged: scene.updateGravity()
                    }
                    RadioButton {
                        id: gravityMars
                        text: qsTr("Mars")
                        onCheckedChanged: scene.updateGravity()
                    }
                    RadioButton {
                        id: gravityEarth
                        text: qsTr("Earth")
                        onCheckedChanged: scene.updateGravity()
                        checked: true
                    }
                }

                // Number of dice
                Label {
                    text: qsTr("Number of dice: %1").arg(diceSlider.value)
                    Layout.fillWidth: true
                }
                Slider {
                    id: diceSlider
                    focusPolicy: Qt.NoFocus
                    from: 1
                    to: 20
                    value: 5
                    stepSize: 1
                }

                // Dice size
                Label {
                    text: qsTr("Dice size: %1cm").arg(diceSize.value.toFixed(1))
                    Layout.fillWidth: true
                }
                Slider {
                    id: diceSize
                    focusPolicy: Qt.NoFocus
                    from: 1
                    to: 10
                    value: 3.5
                    stepSize: 0.5
                }

                // Throw dice
                Button {
                    id: throwButton
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Throw dice")
                    onClicked: {
                        scene.rollForce = Qt.vector3d(0, 0, 0)
                        scene.spawnDice()
                    }
                }
            }
        }
    }

    RoundButton {
        id: iconOpen
        icon.source: "Menu_Icon.svg"
        x: root.currDrawerWidth
        opacity: 1.0 - drawer.position
        scale: opacity
        transformOrigin: Item.Center
        onClicked: {
            tapLabel.opacity = 0
            drawer.open()
        }
    }
}
