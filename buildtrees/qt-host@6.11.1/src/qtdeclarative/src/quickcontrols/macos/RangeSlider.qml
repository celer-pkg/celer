// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl
import QtQuick.Controls.macOS.impl
import QtQuick.NativeStyle as NativeStyle

T.RangeSlider {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
        Math.max(first.implicitHandleWidth, second.implicitHandleWidth) + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
        Math.max(first.implicitHandleHeight, second.implicitHandleHeight) + topPadding + bottomPadding)

    readonly property bool __notCustomizable: true

    first.handle: SliderHandle {
        x: Math.round(control.horizontal
            ? control.leftPadding + (control.first.position * (control.availableWidth - width))
            : (control.width - width) / 2)
        y: Math.round(control.horizontal
            ? (control.height - height) / 2
            : control.leftPadding + (control.availableHeight - height - (control.first.position * (control.availableHeight - height + 4)) + 2))
        width: NativeStyle.StyleConstants.runningWithLiquidGlass ? (control.horizontal ? 20 : 16) : implicitWidth
        height: NativeStyle.StyleConstants.runningWithLiquidGlass ? (control.horizontal ? 16 : 20) : implicitHeight

        palette: control.palette
        pressed: control.first.pressed
        progress: control.first.position
        orientation: control.orientation
        isRangeSlider: true
        isLeftHandle: true
        readonly property bool __ignoreNotCustomizable: true
    }

    second.handle: SliderHandle {
        x: Math.round(control.horizontal
            ? control.leftPadding + (control.second.position * (control.availableWidth - width))
            : (control.width - width) / 2)
        y: Math.round(control.horizontal
            ? (control.height - height) / 2
            : control.leftPadding + (control.availableHeight - height - (control.second.position * (control.availableHeight - height + 4)) + 2))
        width: NativeStyle.StyleConstants.runningWithLiquidGlass ? (control.horizontal ? 20 : 16) : implicitWidth
        height: NativeStyle.StyleConstants.runningWithLiquidGlass ? (control.horizontal ? 16 : 20) : implicitHeight

        palette: control.palette
        pressed: control.second.pressed
        progress: control.second.position
        orientation: control.orientation
        isRangeSlider: true
        isLeftHandle: false

        readonly property bool __ignoreNotCustomizable: true
    }

    background: Item {
        implicitWidth: control.horizontal ? 124 : 24
        implicitHeight: control.horizontal ? 24 : 124

        readonly property bool __ignoreNotCustomizable: true
        readonly property int barThickness: NativeStyle.StyleConstants.runningWithLiquidGlass ? 6 : 4

        // Groove background.
        Rectangle {
            x: control.horizontal ? 0 : (parent.width - width) / 2
            y: control.horizontal ? (parent.height - height) / 2 : 0
            width: control.horizontal ? parent.width : parent.barThickness
            height: control.horizontal ? parent.barThickness : parent.height
            radius: height / 2
            color: control.palette.window

            Rectangle {
                width: parent.width
                height: parent.height
                radius: parent.radius
                // No border in dark mode, instead we fill.
                color: NativeStyle.StyleConstants.runningWithLiquidGlass
                    ? NativeStyle.StyleConstants.tertiarySystemFillColor
                    : Application.styleHints.colorScheme === Qt.Light
                      ? "transparent" : Qt.lighter(control.palette.window, 1.6)
                border.color: NativeStyle.StyleConstants.runningWithLiquidGlass
                              ? NativeStyle.StyleConstants.tertiarySystemFillColor
                              : Application.styleHints.colorScheme === Qt.Light
                                ? Qt.darker(control.palette.window, 1.1) : "transparent"
            }
        }

        // Progress bar.
        Rectangle {
            x: control.horizontal ? control.first.position * parent.width : (parent.width - width) / 2
            y: control.horizontal ? (parent.height - height) / 2 : control.second.visualPosition * parent.height
            width: control.horizontal
                ? control.second.position * parent.width - control.first.position * parent.width
                : parent.barThickness
            height: control.horizontal
                ? parent.barThickness
                : control.second.position * parent.height - control.first.position * parent.height
            radius: height / 2
            color: {
                const light = Application.styleHints.colorScheme === Qt.Light
                if (!control.enabled)
                    return light ? "transparent" : Qt.lighter(control.palette.window, 1.4)
                if (Application.state !== Qt.ApplicationActive)
                    return Qt.lighter(control.palette.window, light ? 0.9 : 1.8)
                return control.palette.accent
            }
       }
    }
}
