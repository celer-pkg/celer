// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D.Xr

XrItem {
    id: prompter
    property int prompt: Prompter.None
    enum Prompt {
        None,
        DarknessFalling,
        PickupTorch,
        LightupTorch,
        DefendCrop
    }

    property var prompts : [
        "",
        "Darkness is descending, enveloping the field.",
        "Lift the torch and prepare for the journey ahead.",
        "Light up the torch and illuminate your path.",
        "Defend the field by igniting the fire pits."
    ]

    property int hasShown: 0
    onPromptChanged: {
        var bitmask = (1<<prompt)
        if (hasShown & bitmask)
            return
        if (textTransition.running)
            textTransition.startAfterFinished = true
        else
            textTransition.restart()
        hasShown |= bitmask
    }

    SequentialAnimation {
        property bool startAfterFinished: false
        id: textTransition
        ScriptAction {
            script: {
                text.text = prompter.prompts[prompter.prompt]
            }
        }
        NumberAnimation {
            duration: 600
            from: 0
            to: 1
            target: text
            easing.type: Easing.OutCubic
            properties: "animateValue"
        }
        NumberAnimation {
            duration: 10000
        }
        NumberAnimation {
            duration: 600
            from: 1
            to: 0
            easing.type: Easing.OutCubic
            target: text
            properties: "animateValue"
        }
        onRunningChanged: {
            if (!running && startAfterFinished) {
                startAfterFinished = false
                restart()
            }
        }
    }

    z: -40
    x: -50
    y: 20
    width: 100
    height: 40
    color: "transparent"
    contentItem: Item {
        width: 100
        height: 50
        Text {
            id: text
            property real animateValue: 0
            opacity: animateValue
            anchors.verticalCenterOffset: (1-animateValue) * 10
            anchors.centerIn: parent
            font.pointSize: 1
            color: "#4a0a0a"
            text: ""
            styleColor: "#000000"
            style: Text.Outline
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
