// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import Qt.labs.folderlistmodel

QtObject {
    id: loader

    property url fontDirectory

    function loadFont(url) {
        var fontLoader = Qt.createQmlObject('import QtQuick; FontLoader { source: "' + url + '"; }',
                                            loader,
                                            "dynamicFontLoader");
    }

    property FolderListModel folderModel: FolderListModel {
        id: folderModel
        folder: loader.fontDirectory
        nameFilters: [ "*.ttf", "*.otf" ]
        showDirs: false

        onStatusChanged: {
            if (folderModel.status == FolderListModel.Ready) {
                for (let i = 0; i < count; i++)
                    loader.loadFont(folderModel.get(i, "fileUrl"))
            }
        }
    }
}

