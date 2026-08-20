// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

import QtQuick
import Qt.labs.StyleKit
import Qt.labs.StyleKit.impl

BaseStyle {
    id: style

    fallbackStyle: FallbackStyle {}

    /* This function is a work-around (called from c++) since we haven't found a way to
     * instantiate a Theme inside the context of a Style from c++ otherwise. Doing so is
     * needed in order to allow custom style properties to be added as children of a
     * Style, and at the same time, be able to access them from within a Theme instance.
     * For this to work, a style with custom properties also needs to set
     * 'pragma ComponentBehavior: Bound'. */
    function createThemeInsideStyle(themeComponent) {
        return themeComponent.createObject(style)
    }
}
