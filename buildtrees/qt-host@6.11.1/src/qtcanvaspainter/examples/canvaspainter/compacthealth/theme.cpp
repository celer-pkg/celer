// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "theme.h"

Theme::Theme() {
    initThemes();
    m_theme = m_darkTheme;
}

void Theme::initThemes() {
    m_darkTheme.background1 = QColor(0x111111);
    m_darkTheme.background2 = QColor(0x1d2127);
    m_darkTheme.foreground1 = QColor(0xf0f0f0);
    m_darkTheme.foreground2 = QColor(0xa0a0a0);
    m_darkTheme.foreground3 = QColor(0x404040);
    m_darkTheme.highlight = QColor(0xf0d040);
    m_darkTheme.warning = QColor(0xf04040);
    m_darkTheme.graph1 = QColor(0x20f020);
    m_darkTheme.graph2 = QColor(0xf0f020);
    m_darkTheme.gradient1.setColorAt(0.0f, 0xff3030);
    m_darkTheme.gradient1.setColorAt(0.7f, 0xD0D020);
    m_darkTheme.gradient1.setColorAt(0.8f, 0x20D020);
    m_darkTheme.gradient1.setColorAt(1.0f, 0x80f0f0);
    m_darkTheme.gradient2.setColorAt(0.0f, 0x80b0f0);
    m_darkTheme.gradient2.setColorAt(1.0f, QColor::fromRgba(0x80504030));

    m_lightTheme.background1 = QColor(0xeeeeee);
    m_lightTheme.background2 = QColor(0xd0d2d8);
    m_lightTheme.foreground1 = QColor(0x101010);
    m_lightTheme.foreground2 = QColor(0x505050);
    m_lightTheme.foreground3 = QColor(0xc0c0c0);
    m_lightTheme.highlight = QColor(0xd09000);
    m_lightTheme.warning = QColor(0xf00000);
    m_lightTheme.graph1 = QColor(0x10a010);
    m_lightTheme.graph2 = QColor(0xa0a010);
    m_lightTheme.gradient1.setColorAt(0.0f, 0xe01010);
    m_lightTheme.gradient1.setColorAt(0.7f, 0xd0d010);
    m_lightTheme.gradient1.setColorAt(0.8f, 0x108010);
    m_lightTheme.gradient1.setColorAt(1.0f, 0x108080);
    m_lightTheme.gradient2.setColorAt(0.0f, 0x60a0e0);
    m_lightTheme.gradient2.setColorAt(1.0f, QColor::fromRgba(0x80504030));
}

void Theme::switchTheme() {
    m_isDarkTheme = !m_isDarkTheme;
    if (m_isDarkTheme)
        m_theme = m_darkTheme;
    else
        m_theme = m_lightTheme;
}

bool Theme::isDark() const
{
    return m_isDarkTheme;
}

QColor Theme::background1() const
{
    return m_theme.background1;
}

QColor Theme::background2() const
{
    return m_theme.background2;
}

QColor Theme::foreground1() const
{
    return m_theme.foreground1;
}

QColor Theme::foreground2() const
{
    return m_theme.foreground2;
}

QColor Theme::foreground3() const
{
    return m_theme.foreground3;
}

QColor Theme::highlight() const
{
    return m_theme.highlight;
}

QColor Theme::warning() const
{
    return m_theme.warning;
}

QColor Theme::graph1() const
{
    return m_theme.graph1;
}

QColor Theme::graph2() const
{
    return m_theme.graph2;
}

QCanvasLinearGradient Theme::gradient1() const
{
    return m_theme.gradient1;
}

QCanvasLinearGradient Theme::gradient2() const
{
    return m_theme.gradient2;
}
