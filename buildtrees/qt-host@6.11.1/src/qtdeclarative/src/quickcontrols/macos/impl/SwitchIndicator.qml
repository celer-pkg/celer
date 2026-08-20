// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

import QtQuick
import QtQuick.NativeStyle as NativeStyle
import QtQuick.Templates as T

Rectangle {
    id: root
    implicitWidth: NativeStyle.StyleConstants.runningWithLiquidGlass ? 55 : 38
    implicitHeight: NativeStyle.StyleConstants.runningWithLiquidGlass ? 25 : 22
    radius: height / 2

    required property T.AbstractButton control
    readonly property real downTintFactor: 1.05

    // For QQuickMacFocusFrame.
    readonly property real __focusFrameRadius: radius

    color: {
        const light = Application.styleHints.colorScheme === Qt.Light
        if (Application.state === Qt.ApplicationActive) {
            if (!control.enabled) {
                if (checked) {
                    return Qt.alpha(control.palette.accent, 0.5)
                } else {
                    if (light)
                        return control.palette.window.darker(1.08)
                    else
                        return control.palette.window.darker(1.2)
                }
            }
            if (checked) {
                if (light) {
                    if (pressed)
                        return control.palette.accent.darker(1.1)
                    else
                        return control.palette.accent
                } else {
                    if (pressed)
                        return control.palette.accent.lighter(1.1)
                    else
                        return control.palette.accent
                }
            } else { // not checked
                if (light) {
                    if (pressed)
                        return control.palette.window.darker(1.4)
                    else
                        return control.palette.window.darker(1.2)
                } else {
                    if (pressed)
                        return control.palette.window.lighter(1.4)
                    else
                        return control.palette.window.lighter(1.2)
                }
            }
        } else { // Qt.ApplicationInactive
            if (!control.enabled) {
                if (light)
                    return control.palette.window.darker(1.08)
                else
                    return control.palette.window.darker(1.2)
            }
            if (checked) {
                if (light)
                    return control.palette.window.darker(1.5)
                else
                    return control.palette.window.lighter(1.8)
            } else { // not checked
                if (light)
                    return control.palette.window.darker(1.2)
                else
                    return control.palette.window.lighter(1.2)
            }
        }
    }

    Behavior on color { ColorAnimation { duration: 226 }  }

    border.color: Application.styleHints.accessibility.contrastPreference === Qt.HighContrast ? Application.styleHints.colorScheme === Qt.Light ? "#b3000000" : "#b3ffffff" : "transparent"

    // Since an equivalent to InnerShadow doesn't exist in Qt 6 (QTBUG-116161),
    // we approximate it using semi-transparent rectangle borders.
    Rectangle {
        width: parent.width
        height: parent.height
        radius: height / 2
        color: "transparent"
        border.color: Application.styleHints.colorScheme === Qt.Light
            ? Qt.darker("#06000000", root.control.down ? root.downTintFactor : 1)
            : Qt.lighter("#1affffff", root.control.down ? root.downTintFactor : 1)

        Rectangle {
            x: 1
            y: 1
            implicitWidth: parent.width - 2
            implicitHeight: parent.height - 2
            radius: parent.radius
            color: "transparent"
            border.color: Application.styleHints.colorScheme === Qt.Light
                ? Qt.darker("#02000000", root.control.down ? root.downTintFactor : 1)
                : Qt.lighter("#04ffffff", root.control.down ? root.downTintFactor : 1)
        }
    }

    SwitchHandle {
        id: handle
        readonly property real handlePadding: NativeStyle.StyleConstants.runningWithLiquidGlass ? 2 : 0
        x: Math.max(handlePadding, Math.min(parent.width - width, root.control.visualPosition * parent.width - (width / 2)) - handlePadding)
        y: (parent.height - height) / 2
        width: NativeStyle.StyleConstants.runningWithLiquidGlass ? 32 : implicitWidth
        height: NativeStyle.StyleConstants.runningWithLiquidGlass ? 20 : root.height
        indicator: root

        Behavior on x {
            // NumberAnimation {
            SmoothedAnimation {
                velocity: 100
            }
        }
    }
}
