// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

//! [root]
Menu {
    Action { text: "Cut" }
    Action { text: "Copy" }
    Action { text: "Paste" }

    MenuSeparator { }

    Menu {
        title: "Find/Replace"
        Action { text: "Find Next" }
        Action { text: "Find Previous" }
        Action { text: "Replace" }
    }
}
//! [root]
