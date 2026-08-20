// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QtGrpc

import QtGrpcChat

pragma ComponentBehavior: Bound

//! [interceptor-5a]
Dialog {
    id: root
    title: "Interceptor Logs"
//! [interceptor-5a]
    standardButtons: Dialog.Close
    modal: true

    function levelToColor(level) : color {
        switch (level) {
        case LogModel.Level.Debug:   return "#9E9E9E"
        case LogModel.Level.Info:    return "#2196F3"
        case LogModel.Level.Warning: return "#FB8C00"
        case LogModel.Level.Error:   return "#E53935"
        default: return "transparent"
        }
    }
    function levelToString(level) : string {
        switch (level) {
        case LogModel.Level.Debug:   return "DEBUG"
        case LogModel.Level.Info:    return "INFO"
        case LogModel.Level.Warning: return "WARNING"
        case LogModel.Level.Error:   return "ERROR"
        default: return "UNKNOWN"
        }
    }
    function rpcTypeToString(type) : string {
        switch (type) {
        case QtGrpc.RpcType.UnaryCall:
            return "UnaryCall"
        case QtGrpc.RpcType.ServerStreaming:
            return "ServerStreaming"
        case QtGrpc.RpcType.ClientStreaming:
            return "ClientStreaming"
        case QtGrpc.RpcType.BidiStreaming:
            return "BidiStreaming"
        }
    }

//! [interceptor-5b]
    ListView {
        ScrollIndicator.horizontal: ScrollIndicator { }
        ScrollIndicator.vertical: ScrollIndicator { }

        anchors.fill: parent
        model: ChatEngine.logModel
        clip: true

        delegate: ItemDelegate {
            id: delegate

            required property int level
            required property string timestamp
            required property int operationId
            required property string service
            required property string method
            required property int rpcType
            required property string message

            width: ListView.view.width

            contentItem: ColumnLayout {
                RowLayout {
                    Label {
                        font.bold: true
                        text: root.levelToString(delegate.level)
                        color: root.levelToColor(delegate.level)
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: delegate.timestamp
                        opacity: 0.7
                    }
                }
                // further entries for visualizing the RPC context and message.
//! [interceptor-5b]
                RowLayout {
                    RowLayout {
                        Label {
                            text: "ID: " + delegate.operationId;
                            opacity: 0.7
                        }
                        Label {
                            text: root.rpcTypeToString(delegate.rpcType);
                            opacity: 0.7
                        }
                    }
                    Item { Layout.fillWidth: true }
                    RowLayout {
                        Label {
                            text: delegate.service;
                            opacity: 0.7
                        }
                        Label {
                            text: delegate.method;
                            opacity: 0.7
                        }
                    }
                }
                Label {
                    text: delegate.message
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
//! [interceptor-5c]
            }
        }
    }
}
//! [interceptor-5c]
