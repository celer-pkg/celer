// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef THEME_H
#define THEME_H

#include <QColor>
#include "qcanvaslineargradient.h"

class Theme
{
public:
    Theme();
    void switchTheme();
    bool isDark() const;
    QColor background1() const;
    QColor background2() const;
    QColor foreground1() const;
    QColor foreground2() const;
    QColor foreground3() const;
    QColor highlight() const;
    QColor warning() const;
    QColor graph1() const;
    QColor graph2() const;
    QCanvasLinearGradient gradient1() const;
    QCanvasLinearGradient gradient2() const;

private:

    void initThemes();

    struct ThemeVars {
        QColor background1;
        QColor background2;
        QColor foreground1;
        QColor foreground2;
        QColor foreground3;
        QColor highlight;
        QColor warning;
        QColor graph1;
        QColor graph2;
        QCanvasLinearGradient gradient1;
        QCanvasLinearGradient gradient2;
    };

    ThemeVars m_theme;
    ThemeVars m_previousTheme;
    ThemeVars m_darkTheme;
    ThemeVars m_lightTheme;
    bool m_isDarkTheme = true;
};

#endif // THEME_H
