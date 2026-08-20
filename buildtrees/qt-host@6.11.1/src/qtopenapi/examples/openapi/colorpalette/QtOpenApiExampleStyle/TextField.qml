// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T

T.TextField {
    id: control
    placeholderText: ""

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    background: Rectangle {
        implicitWidth: 200
        radius: 5

        color: control.readOnly
               ? UIStyle.buttonGray
               : UIStyle.background

        border.color: UIStyle.buttonOutline
    }

    color: control.readOnly
              ? Qt.rgba(UIStyle.textColor.r,
                        UIStyle.textColor.g,
                        UIStyle.textColor.b,
                        0.6)
              : UIStyle.textColor
}
