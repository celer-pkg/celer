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

#ifndef Q3DTHEME_P_H
#define Q3DTHEME_P_H

#include <private/datavisualizationglobal_p.h>
#include "q3dtheme.h"

QT_BEGIN_NAMESPACE

struct Q3DThemeDirtyBitField {
    bool baseColorDirty                : 1;
    bool backgroundColorDirty          : 1;
    bool windowColorDirty              : 1;
    bool labelTextColorDirty           : 1;
    bool labelBackgroundColorDirty     : 1;
    bool gridLineColorDirty            : 1;
    bool singleHighlightColorDirty     : 1;
    bool multiHighlightColorDirty      : 1;
    bool lightColorDirty               : 1;
    bool baseGradientDirty             : 1;
    bool singleHighlightGradientDirty  : 1;
    bool multiHighlightGradientDirty   : 1;
    bool lightStrengthDirty            : 1;
    bool ambientLightStrengthDirty     : 1;
    bool highlightLightStrengthDirty   : 1;
    bool labelBorderEnabledDirty       : 1;
    bool colorStyleDirty               : 1;
    bool fontDirty                     : 1;
    bool backgroundEnabledDirty        : 1;
    bool gridEnabledDirty              : 1;
    bool labelBackgroundEnabledDirty   : 1;
    bool themeIdDirty                  : 1;

    Q3DThemeDirtyBitField()
        : baseColorDirty(false),
          backgroundColorDirty(false),
          windowColorDirty(false),
          labelTextColorDirty(false),
          labelBackgroundColorDirty(false),
          gridLineColorDirty(false),
          singleHighlightColorDirty(false),
          multiHighlightColorDirty(false),
          lightColorDirty(false),
          baseGradientDirty(false),
          singleHighlightGradientDirty(false),
          multiHighlightGradientDirty(false),
          lightStrengthDirty(false),
          ambientLightStrengthDirty(false),
          highlightLightStrengthDirty(false),
          labelBorderEnabledDirty(false),
          colorStyleDirty(false),
          fontDirty(false),
          backgroundEnabledDirty(false),
          gridEnabledDirty(false),
          labelBackgroundEnabledDirty(false),
          themeIdDirty(false)
    {
    }
};

class Q_DATAVISUALIZATION_EXPORT Q3DThemePrivate : public QObject
{
    Q_OBJECT
public:
    Q3DThemePrivate(Q3DTheme *q);
    virtual ~Q3DThemePrivate();

    void resetDirtyBits();

    bool sync(Q3DThemePrivate &other);

    inline bool isDefaultTheme() { return m_isDefaultTheme; }
    inline void setDefaultTheme(bool isDefault) { m_isDefaultTheme = isDefault; }

    // If m_forcePredefinedType is true, it means we should forcibly update all properties
    // of the theme to those of the predefined theme, when setting the theme type. Otherwise
    // we only change the properties that haven't been explicitly changed since last render cycle.
    // Defaults to true, and is only ever set to false by DeclarativeTheme3D to enable using
    // predefined themes as base for custom themes, since the order of initial property sets cannot
    // be easily controlled in QML.
    inline bool isForcePredefinedType() { return m_forcePredefinedType; }
    inline void setForcePredefinedType(bool enable) { m_forcePredefinedType = enable; }

Q_SIGNALS:
    void needRender();

public:
    Q3DTheme::Theme m_themeId;

    Q3DThemeDirtyBitField m_dirtyBits;

    QList<QColor> m_baseColors;
    QColor m_backgroundColor;
    QColor m_windowColor;
    QColor m_textColor;
    QColor m_textBackgroundColor;
    QColor m_gridLineColor;
    QColor m_singleHighlightColor;
    QColor m_multiHighlightColor;
    QColor m_lightColor;
    QList<QLinearGradient> m_baseGradients;
    QLinearGradient m_singleHighlightGradient;
    QLinearGradient m_multiHighlightGradient;
    float m_lightStrength;
    float m_ambientLightStrength;
    float m_highlightLightStrength;
    bool m_labelBorders;
    Q3DTheme::ColorStyle m_colorStyle;
    QFont m_font;
    bool m_backgoundEnabled;
    bool m_gridEnabled;
    bool m_labelBackground;
    bool m_isDefaultTheme;
    bool m_forcePredefinedType;

protected:
    Q3DTheme *q_ptr;
};

QT_END_NAMESPACE

#endif
