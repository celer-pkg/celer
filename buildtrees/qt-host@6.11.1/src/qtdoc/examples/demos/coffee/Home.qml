// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick

HomeForm {
    state: Config.mode

    grid.states: [
        State {
            name: "small"
            when: ((Screen.height * Screen.devicePixelRatio)
                   + (Screen.width * Screen.devicePixelRatio)) < 2000
            PropertyChanges {
                target: header
                font.pixelSize: 28
            }
            PropertyChanges {
                target: caption
                font.pixelSize: 14
            }
        }
    ]
}
