// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickbasictheme_p.h"

#include <QtQuickTemplates2/private/qquicktheme_p.h>
#include <QtQuickControls2/private/qquickstyle_p.h>

QT_BEGIN_NAMESPACE

void QQuickBasicTheme::initialize(QQuickTheme *theme)
{
    QPalette systemPalette;

    const bool isDarkSystemTheme = QQuickStylePrivate::isDarkSystemTheme();

    const QRgb base(isDarkSystemTheme ? 0xFF000000 : 0xFFFFFFFF);
    const QRgb disabledBase(isDarkSystemTheme ? 0xFF292929 : 0xFFD6D6D6);
    const QRgb button(isDarkSystemTheme ? 0xFF2F2F2F : 0xFFE0E0E0);
    const QRgb buttonText(isDarkSystemTheme ? 0xFFD4D6D8 : 0xFF26282A);
    const QRgb disabledButtonText(isDarkSystemTheme ? 0x4DD4D6D8 : 0x4D26282A);
    const QRgb brightText(isDarkSystemTheme ? 0xFF000000 : 0xFFFFFFFF);
    const QRgb disabledBrightText(isDarkSystemTheme ? 0x4D000000 : 0x4DFFFFFF);
    const QRgb dark(isDarkSystemTheme ? 0xFFC8C9CB : 0xFF353637);
    const QRgb highlight(isDarkSystemTheme ? 0xFF0D69F2 : 0xFF0066FF);
    const QRgb disabledHighlight(isDarkSystemTheme ? 0xFF01060F : 0xFFF0F6FF);
    const QRgb highlightedText(isDarkSystemTheme ? 0xFFFDFDFD : 0xFF090909);
    const QRgb light(isDarkSystemTheme ? 0xFF1A1A1A : 0xFFF6F6F6);
    const QRgb link(isDarkSystemTheme ? 0xFF2F86B1 : 0xFF45A7D7);
    const QRgb mid(isDarkSystemTheme ? 0xFF626262 : 0xFFBDBDBD);
    const QRgb midlight(isDarkSystemTheme ? 0xFF2C2C2C : 0xFFE4E4E4);
    const QRgb text(isDarkSystemTheme ? 0xFFEFF0F2 : 0xFF353637);
    const QRgb disabledText(isDarkSystemTheme ? 0x7FC8C9CB : 0x7F353637);
    const QRgb shadow(0xFF28282A); // using the same color for dark and light
    const QRgb toolTipBase(isDarkSystemTheme ? 0xFF000000 : 0xFFFFFFFF);
    const QRgb toolTipText(isDarkSystemTheme ? 0xFFFFFFFF : 0xFF000000);
    const QRgb window(isDarkSystemTheme ? 0xFF000000 : 0xFFFFFFFF);
    const QRgb windowText(isDarkSystemTheme ? 0xFFD4D6D8 : 0xFF26282A);
    const QRgb disabledWindowText(isDarkSystemTheme ? 0xFF3F4040 : 0xFFBDBEBF);
    const QRgb placeholderText(isDarkSystemTheme ? 0x88C8C9CB : 0x88353637);

    systemPalette.setColor(QPalette::Base, QColor::fromRgba(base));
    systemPalette.setColor(QPalette::Disabled, QPalette::Base, QColor::fromRgba(disabledBase));

    systemPalette.setColor(QPalette::Button, QColor::fromRgba(button));

    systemPalette.setColor(QPalette::ButtonText, QColor::fromRgba(buttonText));
    systemPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                           QColor::fromRgba(disabledButtonText));

    systemPalette.setColor(QPalette::BrightText, QColor::fromRgba(brightText));
    systemPalette.setColor(QPalette::Disabled, QPalette::BrightText,
                           QColor::fromRgba(disabledBrightText));

    systemPalette.setColor(QPalette::Dark, QColor::fromRgba(dark));

    systemPalette.setColor(QPalette::Highlight, QColor::fromRgba(highlight));
    systemPalette.setColor(QPalette::Disabled, QPalette::Highlight,
                           QColor::fromRgba(disabledHighlight));

    systemPalette.setColor(QPalette::HighlightedText, QColor::fromRgba(highlightedText));

    systemPalette.setColor(QPalette::Light, QColor::fromRgba(light));

    systemPalette.setColor(QPalette::Link, QColor::fromRgba(link));

    systemPalette.setColor(QPalette::Mid, QColor::fromRgba(mid));

    systemPalette.setColor(QPalette::Midlight, QColor::fromRgba(midlight));

    systemPalette.setColor(QPalette::Text, QColor::fromRgba(text));
    systemPalette.setColor(QPalette::Disabled, QPalette::Text, QColor::fromRgba(disabledText));

    systemPalette.setColor(QPalette::Shadow, QColor::fromRgba(shadow));

    systemPalette.setColor(QPalette::ToolTipBase, QColor::fromRgba(toolTipBase));
    systemPalette.setColor(QPalette::ToolTipText, QColor::fromRgba(toolTipText));

    systemPalette.setColor(QPalette::Window, QColor::fromRgba(window));

    systemPalette.setColor(QPalette::WindowText, QColor::fromRgba(windowText));
    systemPalette.setColor(QPalette::Disabled, QPalette::WindowText,
                           QColor::fromRgba(disabledWindowText));

    systemPalette.setColor(QPalette::PlaceholderText, QColor::fromRgba(placeholderText));

    theme->setPalette(QQuickTheme::System, systemPalette);
}

void QQuickBasicTheme::updateTheme()
{
    QQuickTheme *theme = QQuickTheme::instance();
    if (!theme)
        return;
    initialize(theme);
}

QT_END_NAMESPACE
