// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

import QtQuick.Controls.iOS
import QtQuick.Controls.iOS.impl as IOSImpl

Menu {
    id: menu
    popupType: Popup.Native

    required property var editor

    IOSImpl.CutAction {
        editor: menu.editor
    }
    IOSImpl.CopyAction {
        editor: menu.editor
    }
    IOSImpl.PasteAction {
        editor: menu.editor
    }
    IOSImpl.DeleteAction {
        editor: menu.editor
    }

    MenuSeparator {}

    IOSImpl.SelectAllAction {
        editor: menu.editor
    }
}
