// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
import QtQml
import Config

QtObject {
    function iconSource(fileName, addSuffix = true) {
        return Qt.resolvedUrl("icons/" + Config.iconName(fileName, addSuffix))
    }
}
