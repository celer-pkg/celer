// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtGrpc
import QtQuick.Controls
import QtQuick.Layouts

import qtgrpc.examples
import qtgrpc.examples.magic8ball

ApplicationWindow {
    id: root

    property answerRequest answerReq
    property string errorText: ""
    property int errorCode: 0

//! [requestAnswerFunction]
    function requestAnswer(question: string): void {
//! [requestAnswerFunction]
        root.errorText = "";
        magicBall.startWaiting();

//! [requestAnswerFunctionBody]
        root.answerReq.question = question;
        grpcClient.answerMethod(root.answerReq, finishCallback, errorCallback, grpcCallOptions);
    }
//! [requestAnswerFunctionBody]

    function finishCallback(response: answerResponse): void {
        magicBall.addAnswer(response.message);
    }

    function errorCallback(error): void {
        // error is received as a JavaScript object, but it is a QGrpcStatus instance
        magicBall.cancelAnimation();
        console.log(
            `Error callback executed. Error message: "${error.message}" Code: ${error.code}`
        );
        root.errorText = error.message;
        root.errorCode = error.code;
    }

    minimumWidth: rootLayout.implicitWidth + rootLayout.anchors.margins * 2
    minimumHeight: rootLayout.implicitHeight + rootLayout.anchors.margins * 2

    visible: true
    title: qsTr("Magic-8-ball Qt GRPC Example")
    font.pointSize: 18

//! [channelOptions]
    GrpcHttp2Channel {
        id: grpcChannel
        hostUri: "http://localhost:50051"
        // Optionally, you can specify custom channel options here
        // options: GrpcChannelOptions {}
    }
//! [channelOptions]

//! [exampleServiceClient]
    ExampleServiceClient {
        id: grpcClient
        channel: grpcChannel.channel
    }
//! [exampleServiceClient]

//! [callOptions]
    GrpcCallOptions {
        id: grpcCallOptions
        deadlineTimeout: 6000
    }
//! [callOptions]

    ColumnLayout {
        id: rootLayout
        anchors.margins: 10
        anchors.fill: parent
        spacing: 12

        MagicText {
            color: "black"
            text: qsTr("Ask the ball a yes-no question and press the button.")
        }

        MagicBall {
            id: magicBall
            Layout.alignment: Qt.AlignCenter
        }

        TextField {
            id: questionInput
            Layout.alignment: Qt.AlignCenter
            Layout.minimumWidth: 300
            leftPadding: 10
            rightPadding: 10
            placeholderText: qsTr("Type here a question...")
        }

        Button {
            onClicked: root.requestAnswer(questionInput.text)
            enabled: magicBall.canRequestAnswer
            Layout.alignment: Qt.AlignCenter
            leftPadding: 16
            rightPadding: 16
            text: qsTr("Ask")
        }

        MagicText {
            visible: root.errorText
            text:
                qsTr("Error: %1\n%2")
                .arg(root.errorText)
                .arg(root.errorCode == QtGrpc.StatusCode.Unavailable
                    ? qsTr("Please, restart the server")
                    : "")
        }
    }
}
