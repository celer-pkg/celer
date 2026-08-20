// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick

Item {
    id: fader
    property real value: 1

    function blink(fadedOutLambda, fadedInLambda, duration) {
        blinkAnimation.stop()
        blinkAnimation.fadedOutLambda = fadedOutLambda
        blinkAnimation.fadedInLambda = fadedInLambda
        blinkAnimation.duration = duration
        blinkAnimation.start()
    }

    SequentialAnimation {
        id: blinkAnimation
        property var fadedOutLambda
        property var fadedInLambda
        property int duration
        NumberAnimation {
            target: fader; property: "value";
            easing.type: Easing.OutCubic
            duration: blinkAnimation.duration
            to: 0
        }
        ScriptAction {
            script: {
                if (blinkAnimation.fadedOutLambda &&
                        typeof blinkAnimation.fadedOutLambda === "function") {
                    let lambda = ()=>{} //to suppress the linter warning
                    lambda = blinkAnimation.fadedOutLambda
                    lambda();
                    blinkAnimation.fadedOutLambda = null;
                }
            }
        }
        NumberAnimation {
            target: fader; property: "value";
            to: 1
            duration: blinkAnimation.duration
            easing.type: Easing.InCubic

        }
        ScriptAction {
            script: {
                if (blinkAnimation.fadedInLambda &&
                        typeof blinkAnimation.fadedInLambda === "function") {
                    let lambda = ()=>{} //to suppress the linter warning
                    lambda = blinkAnimation.fadedInLambda
                    lambda();
                    blinkAnimation.fadedInLambda = null;
                }
            }
        }
        running: false
    }
}
