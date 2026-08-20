// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

import QtQuick
import QtQuick.NativeStyle as NativeStyle
import QtQuick.Controls.macOS.impl

NativeStyle.DefaultTextField {
    id: control
    readonly property Item __focusFrameTarget: control

    ContextMenu.menu: TextEditingContextMenu {
        editor: control
    }
}
