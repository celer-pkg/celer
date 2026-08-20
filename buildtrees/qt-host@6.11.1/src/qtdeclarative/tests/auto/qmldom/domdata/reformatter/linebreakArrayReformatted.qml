// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    readonly property var a:
        [123456789123456789,
        223456789123456789,
        323456789123456789,
        44632623467236]
    readonly property list<Item> b: [
        Item {
        },
        Item {
        },
        Item {
        },
        Item {
        }
    ]
}
