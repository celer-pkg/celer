// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: root

    property string currentAnswerText: ""
    property string nextAnswerText: ""
    property bool canRequestAnswer: true

    state: "DISABLED"
    states: [
        State {
            name: "DISABLED"
            PropertyChanges {
                root.canRequestAnswer: true
                answerText.visible: false
                waitingPlaceholder.visible: false
            }
        },
        State {
            name: "WAITING"
            PropertyChanges {
                root.canRequestAnswer: false
                answerText.visible: false
                waitingPlaceholder.visible: true
            }
        },
        State {
            name: "SHOWING"
            PropertyChanges {
                root.canRequestAnswer: false
                answerText.visible: true
                waitingPlaceholder.visible: false
            }
        },
        State {
            name: "PAUSED"
            PropertyChanges {
                root.canRequestAnswer: true
                answerText.visible: true
                waitingPlaceholder.visible: false
            }
        },
        State {
            name: "HIDING"
            PropertyChanges {
                root.canRequestAnswer: false
                answerText.visible: true
                waitingPlaceholder.visible: false
            }
        }
    ]

    transitions: [
        Transition {
            from: "DISABLED,HIDING"
            to: "WAITING"
            SequentialAnimation {
                id: waitingAnimation
                loops: Animation.Infinite

                ScaleAnimation {
                    target: root
                    mode: "ZoomIn"
                }

                ScaleAnimation {
                    target: root
                    mode: "ZoomOut"
                }
            }

            onRunningChanged: {
                if (!running) {
                    root.currentAnswerText = root.nextAnswerText;
                    root.nextAnswerText = "";
                    if (!root.currentAnswerText) {
                        root.state = "DISABLED";
                    }
                }
            }
        },
        Transition {
            from: "WAITING,HIDING"
            to: "SHOWING"

            ScaleAnimation {
                target: root
                mode: "ZoomIn"
            }

            onRunningChanged: {
                if (!running) {
                    root.state = "PAUSED";
                }
            }
        },
        Transition {
            from: "PAUSED"
            to: "HIDING"

            ScaleAnimation {
                target: root
                mode: "ZoomOut"
            }

            onRunningChanged: {
                if (!running) {
                    if (root.nextAnswerText) {
                        root.currentAnswerText = root.nextAnswerText;
                        root.nextAnswerText = "";
                        root.state = "SHOWING";
                    } else {
                        root.state = "WAITING";
                    }
                }
            }
        }
    ]

    function addAnswer(answer: string): void {
        root.nextAnswerText = answer;
        if (root.state == "WAITING") {
            root.state = "SHOWING";
        }
    }

    function startWaiting(): void {
        if (root.state == "PAUSED") {
            root.state = "HIDING";
            return;
        }
        root.state = "WAITING";
    }

    function cancelAnimation(): void {
        root.state = "DISABLED";
    }

    MagicText {
        id: answerText
        anchors.centerIn: parent
        font.pointSize: 20
        color: "#2E53B6"
        text: root.currentAnswerText
    }

    Row {
        id: waitingPlaceholder
        anchors.centerIn: parent
        spacing: 12

        Repeater {
            model: 3
            Rectangle {
                width: 11
                height: width
                color: "#264BAF"
                radius: 100
            }
        }
    }
}
