// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

SimpleButton {
    id: keyItem

    property bool purchasable: false

    signal keyActivated(string letter)

    onClicked: {
        if (available) {
            keyActivated(text);
        }
    }

}
