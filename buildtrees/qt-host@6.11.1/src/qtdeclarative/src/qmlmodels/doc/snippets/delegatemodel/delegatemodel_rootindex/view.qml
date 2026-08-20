// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
//![0]
import QtQuick
import QtQml.Models
import FileSystemModule

pragma ComponentBehavior: Bound

ListView {
    id: view
    width: 300
    height: 400

    model: DelegateModel {
        id: delegateModel
        model: FileSystemModel // singleton

        delegate: Rectangle {
            id: delegate
            required property int index
            required property string filePath
            required property bool hasModelChildren

            width: 300; height: 25
            color: index % 2 ? palette.alternateBase : palette.base

            Text {
                anchors.verticalCenter: parent.verticalCenter
                color: palette.text
                text: delegate.filePath
            }

            TapHandler {
                onTapped: if (delegate.hasModelChildren)
                              delegateModel.rootIndex = delegateModel.modelIndex(delegate.index)
            }
        }
    }
}
//![0]
