// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
pragma ComponentBehavior: Bound
import QtQuick

QtObject {
    id: root

    readonly property real appMinimumWidth: responsiveSize(__design.minimumWidth())
    readonly property real appMinimumHeight: responsiveSize(__design.minimumHeight())
    property real appWidth: responsiveSize(__design.width(__isScreenPortrait()))
    property real appHeight: responsiveSize(__design.height(__isScreenPortrait()))

    function updateApplicationSize(width: real, height: real) {
        if (Math.abs(appWidth - width) >= 1)
            appWidth = width
        if (Math.abs(appHeight - height) >= 1)
            appHeight = height
    }

    function __isScreenPortrait() {
        switch (Screen.primaryOrientation) {
        case Qt.PortraitOrientation:
        case Qt.InvertedPortraitOrientation:
            return true;
        default:
            break
        }
        return false
    }

    readonly property bool isPortrait: {
        if (__isScreenPortrait())
            return true
        return appWidth < responsiveSize(__design.responsiveBreakpoint())
    }

    function fontFamily() {
        return "Winky Sans"
    }

    enum TextStyle {
        Body,
        Body_Bold,
        Body_L,
        Button_S,
        Button_M,
        Button_L,
        Caption,
        H1,
        H2,
        H2_Bold,
        H3_Bold,
        H3_Light,
        H4,
        Price_S,
        Price_M,
        Price_ML,
        Price_L,
        Price_XL,
        Price_XXL
    }

    function responsiveFontSize(style) {
        // The font sizes are based on Dongle font family.
        // For other font families we multiply them by '0.9'.
        const designFontSize = __design.fontSize(style) * 0.9
        return Math.round(responsiveSize(designFontSize))
    }

    function responsiveSize(designSize: real): real {
        const responsiveRatio = 0.28125 // the scale ratio for design sizes
        return designSize * responsiveRatio
    }

    function isBoldText(style: int): bool {
        return (style === ApplicationConfig.TextStyle.Body_Bold) ||
               (style === ApplicationConfig.TextStyle.Button_L) ||
               (style === ApplicationConfig.TextStyle.H1) ||
               (style === ApplicationConfig.TextStyle.H2_Bold) ||
               (style === ApplicationConfig.TextStyle.H3_Bold) ||
               (style === ApplicationConfig.TextStyle.Price_M)
    }

    property QtObject __design: QtObject {
        id: __design

        function minimumWidth(): int { return 2160 }
        function minimumHeight(): int { return 3386 }

        function width(portrait: bool): int { return portrait ? 2160 : 5184 }
        function height(portrait: bool): int { return portrait ? 3840 : 3040 }

        function responsiveBreakpoint(): int { return 3400 }

        function fontSize(style: int): int {
            switch (style) {
            case ApplicationConfig.TextStyle.Body:
            case ApplicationConfig.TextStyle.Body_Bold:
            case ApplicationConfig.TextStyle.Button_S:
            case ApplicationConfig.TextStyle.Caption:
            case ApplicationConfig.TextStyle.Price_S:
                return 56
            case ApplicationConfig.TextStyle.Price_M:
            case ApplicationConfig.TextStyle.Button_M:
            case ApplicationConfig.TextStyle.H4:
                return 72
            case ApplicationConfig.TextStyle.Body_L:
            case ApplicationConfig.TextStyle.Button_L:
            case ApplicationConfig.TextStyle.H3_Bold:
            case ApplicationConfig.TextStyle.H3_Light:
                return 80
            case ApplicationConfig.TextStyle.H2:
                return 92
            case ApplicationConfig.TextStyle.H1:
                return 112
            case ApplicationConfig.TextStyle.H2_Bold:
            case ApplicationConfig.TextStyle.Price_ML:
                return 120
            case ApplicationConfig.TextStyle.Price_L:
            case ApplicationConfig.TextStyle.Price_XL:
                return 160
            case ApplicationConfig.TextStyle.Price_XXL:
                return 240
            }
        }
    }
}
