// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QGRAPHSTHEME_P_H
#define QGRAPHSTHEME_P_H
#include <QtGraphs/qgraphstheme.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <private/qobject_p.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcGraphsCommonTheme)

class QGraphsThemePrivate : public QObjectPrivate
{
public:
    QGraphsThemePrivate();
    ~QGraphsThemePrivate() override;

private:
    struct QGraphsCustomBitField
    {
        bool plotAreaBackgroundColorCustom : 1;
        bool seriesColorsCustom : 1;
        bool borderColorsCustom : 1;
        bool seriesGradientCustom : 1;
        bool labelBackgroundColorCustom : 1;
        bool labelTextColorCustom : 1;
        bool multiHighlightColorCustom : 1;
        bool multiHighlightGradientCustom : 1;
        bool singleHighlightColorCustom : 1;
        bool singleHighlightGradientCustom : 1;
        bool backgroundColorCustom : 1;
        bool axisXLabelFontCustom : 1;
        bool axisYLabelFontCustom : 1;
        bool axisZLabelFontCustom : 1;

        QGraphsCustomBitField()
            : plotAreaBackgroundColorCustom(false)
            , seriesColorsCustom(false)
            , borderColorsCustom(false)
            , seriesGradientCustom(false)
            , labelBackgroundColorCustom(false)
            , labelTextColorCustom(false)
            , multiHighlightColorCustom(false)
            , multiHighlightGradientCustom(false)
            , singleHighlightColorCustom(false)
            , singleHighlightGradientCustom(false)
            , backgroundColorCustom(false)
            , axisXLabelFontCustom(false)
            , axisYLabelFontCustom(false)
            , axisZLabelFontCustom(false)
        {}
    };

    QGraphsThemeDirtyBitField m_dirtyBits;
    QGraphsCustomBitField m_customBits;

    QMetaObject::Connection m_autoColorConnection;
    bool m_themeDirty = false;
    QGraphsTheme::ColorScheme m_colorScheme = QGraphsTheme::ColorScheme::Automatic;
    QGraphsTheme::Theme m_theme = QGraphsTheme::Theme::QtGreen;
    QGraphsTheme::ColorStyle m_colorStyle = QGraphsTheme::ColorStyle::Uniform;
    QList<QObject *> m_themeChildren;
    QColor m_plotAreaBackgroundColor;
    QColor m_plotAreaBackgroundThemeColor;
    bool m_backgroundVisibility = false;
    bool m_gridVisibility = false;
    QColor m_backgroundColor;
    QColor m_backgroundThemeColor;
    bool m_plotAreaBackgroundVisibility = false;
    bool m_labelsVisibility = false;
    QColor m_labelBackgroundColor;
    QColor m_labelBackgroundThemeColor;
    QColor m_labelTextColor;
    QColor m_labelTextThemeColor;
    bool m_labelBackgroundVisibility = false;
    bool m_labelBorderVisibility = false;
    QColor m_singleHighlightColor;
    QColor m_singleHighlightThemeColor;
    QColor m_multiHighlightColor;
    QColor m_multiHighlightThemeColor;
    QLinearGradient m_multiHighlightGradient;
    QLinearGradient m_multiHighlightThemeGradient;
    QLinearGradient m_singleHighlightGradient;
    QLinearGradient m_singleHighlightThemeGradient;
    QFont m_labelFont;
    QList<QColor> m_seriesColors;
    QList<QColor> m_seriesThemeColors;
    QList<QColor> m_borderColors;
    QList<QColor> m_borderThemeColors;
    qreal m_borderWidth = 1.0;
    QList<QLinearGradient> m_seriesGradients;
    QList<QLinearGradient> m_seriesThemeGradients;

    QList<QQuickGraphsColor *> m_colors;
    QList<QQuickGradient *> m_gradients;
    QQuickGradient *m_singleHLQuickGradient = nullptr;
    QQuickGradient *m_multiHLQuickGradient = nullptr;

    QFont m_axisXLabelFont;
    QFont m_axisYLabelFont;
    QFont m_axisZLabelFont;

    QGraphsLine m_grid;
    QGraphsLine m_axisX;
    QGraphsLine m_axisY;
    QGraphsLine m_axisZ;

    bool m_dummyColors = false;

    bool m_componentComplete = false;

    Q_DECLARE_PUBLIC(QGraphsTheme)
    Q_DISABLE_COPY_MOVE(QGraphsThemePrivate)
};

struct QGraphsLinePrivate : public QSharedData
{
    struct QGraphsLineCustomField
    {
        bool mainColorCustom : 1;
        bool subColorCustom : 1;
        bool labelTextColorCustom : 1;

        QGraphsLineCustomField()
            : mainColorCustom(false)
            , subColorCustom(false)
            , labelTextColorCustom(false)
        {}
    };

    QGraphsLinePrivate();
    ~QGraphsLinePrivate();
    QGraphsLinePrivate(const QGraphsLinePrivate &other);
    void resetCustomBits();

    QColor m_mainColor;
    QColor m_subColor;
    qreal m_mainWidth;
    qreal m_subWidth;
    QColor m_labelTextColor;

    QColor m_mainThemeColor;
    QColor m_subThemeColor;
    QColor m_labelTextThemeColor;
    QGraphsLineCustomField m_bits;

    friend bool comparesEqual(const QGraphsLinePrivate &lhs, const QGraphsLinePrivate &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QGraphsLinePrivate)

    friend QGraphsLine;
};

QT_END_NAMESPACE
#endif // QGRAPHSTHEME_P_H
