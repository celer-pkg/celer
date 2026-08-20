// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef THEMEMANAGER_P_H
#define THEMEMANAGER_P_H

#include "datavisualizationglobal_p.h"
#include "abstract3dcontroller_p.h"
#include "q3dtheme.h"

QT_BEGIN_NAMESPACE

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    ThemeManager(Abstract3DController *controller);
    ~ThemeManager();

    void addTheme(Q3DTheme *theme);
    void releaseTheme(Q3DTheme *theme);
    void setActiveTheme(Q3DTheme *theme);
    Q3DTheme *activeTheme() const;
    QList<Q3DTheme *> themes() const;

    static void setPredefinedPropertiesToTheme(Q3DTheme *theme, Q3DTheme::Theme type);

protected:
    void connectThemeSignals();
    static QLinearGradient createGradient(const QColor &color, float colorLevel);
    static void setBaseColors(Q3DTheme *theme, const QList<QColor> &colors);
    static void setBackgroundColor(Q3DTheme *theme, const QColor &color);
    static void setWindowColor(Q3DTheme *theme, const QColor &color);
    static void setTextColor(Q3DTheme *theme, const QColor &color);
    static void setTextBackgroundColor(Q3DTheme *theme, const QColor &color);
    static void setGridLineColor(Q3DTheme *theme, const QColor &color);
    static void setSingleHighlightColor(Q3DTheme *theme, const QColor &color);
    static void setMultiHighlightColor(Q3DTheme *theme, const QColor &color);
    static void setLightColor(Q3DTheme *theme, const QColor &color);
    static void setBaseGradients(Q3DTheme *theme, const QList<QLinearGradient> &gradients);
    static void setSingleHighlightGradient(Q3DTheme *theme, const QLinearGradient &gradient);
    static void setMultiHighlightGradient(Q3DTheme *theme, const QLinearGradient &gradient);
    static void setLightStrength(Q3DTheme *theme, float strength);
    static void setAmbientLightStrength(Q3DTheme *theme, float strength);
    static void setHighlightLightStrength(Q3DTheme *theme, float strength);
    static void setLabelBorderEnabled(Q3DTheme *theme, bool enabled);
    static void setFont(Q3DTheme *theme, const QFont &font);
    static void setBackgroundEnabled(Q3DTheme *theme, bool enabled);
    static void setGridEnabled(Q3DTheme *theme, bool enabled);
    static void setLabelBackgroundEnabled(Q3DTheme *theme, bool enabled);
    static void setColorStyle(Q3DTheme *theme, Q3DTheme::ColorStyle style);

private:
    Q3DTheme *m_activeTheme;
    QList<Q3DTheme *> m_themes; // List of all added themes
    Abstract3DController *m_controller;
};

QT_END_NAMESPACE

#endif
