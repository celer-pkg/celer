// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

import QtQuick
import QtQuick.Templates as T
import QtQuick.NativeStyle as NativeStyle
import QtQuick.Shapes
import QtQuick.Effects

Item {
    id: handle
    implicitWidth: 22
    implicitHeight: 22

    required property SwitchIndicator indicator
    readonly property T.AbstractButton control: indicator.control
    property color handleColor: Application.styleHints.colorScheme === Qt.Light ? palette.base : "#cdcbc9"

    Behavior on x {
        SmoothedAnimation {
            velocity: NativeStyle.StyleConstants.runningWithLiquidGlass ? 100 : 200
        }
    }

    Loader {
        active: NativeStyle.StyleConstants.runningWithLiquidGlass
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: parent.width + (parent.control.down ? 14 : 0)
        height: parent.height + (parent.control.down ? 14 : 0)
        Behavior on width { NumberAnimation { duration: 200 } }
        Behavior on height { NumberAnimation { duration: 200 } }
        sourceComponent: Rectangle {
            radius: width / 2
            readonly property color fillColor1: indicator.color
            readonly property color fillColor2: "black"
            gradient: RadialGradient {
                GradientStop { position: 0.0; color: control.down ? Qt.alpha(fillColor1, 1.0) : handleColor }
                GradientStop { position: 0.15; color: control.down ? Qt.alpha(fillColor2, 0.2) : handleColor  }
                GradientStop { position: 0.4; color: control.down ? Qt.alpha(fillColor2, 0.1) : handleColor  }
                GradientStop { position: 0.6; color: control.down ? Qt.alpha(fillColor2, 0.1) : handleColor  }
                GradientStop { position: 0.9; color: control.down ? Qt.alpha(fillColor2, 0.2) : handleColor }
                GradientStop { position: 1.0; color: control.down ? Qt.alpha(fillColor1, 1.0) : handleColor }
            }
            border.color: Application.styleHints.accessibility.contrastPreference === Qt.HighContrast
                            ? Application.styleHints.colorScheme === Qt.Light ? "#b3000000" : "#b3ffffff"
                            : Application.styleHints.colorScheme === Qt.Light
                                ? Qt.alpha(handleColor, 1.0) : Qt.alpha(handleColor, 0.3)
            border.width: 1

            Behavior on color { ColorAnimation { duration: 200 } }

            layer.enabled: Application.styleHints.accessibility.contrastPreference !== Qt.HighContrast
            layer.effect: MultiEffect {
                shadowEnabled: true
                blurMax: 10
                shadowBlur: 1
                shadowScale: 1.3
                shadowOpacity: 0.05
            }
        }
    }

    Loader {
        active: !NativeStyle.StyleConstants.runningWithLiquidGlass
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: parent.width - 2
        height: parent.height - 2
        sourceComponent: Rectangle {
            radius: width / 2
            color: {
                const light = Application.styleHints.colorScheme === Qt.Light
                if (!control.enabled)
                    return light ? palette.base : "#64676a";
                if (light)
                    return Qt.darker(palette.base, handle.control.down ? 1.05 : 1)
                return Qt.lighter("#cdcbc9", handle.control.down ? 1.05 : 1)
            }

            border.color: Application.styleHints.accessibility.contrastPreference === Qt.HighContrast
                          ? Application.styleHints.colorScheme === Qt.Light ? "#b3000000" : "#b3ffffff"
            : "transparent"
            border.width: 1

            layer.enabled: Application.styleHints.accessibility.contrastPreference !== Qt.HighContrast
            layer.effect: MultiEffect {
                shadowEnabled: true
                blurMax: 10
                shadowBlur: 0.2
                shadowScale: 0.92
                shadowOpacity: 1
            }
        }
    }
}
