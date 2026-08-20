// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef BARSRENDERER_H
#define BARSRENDERER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include <QQuickItem>
#include <QtQuick/private/qquicktext_p.h>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QRectF>
#include <QtGui/QColor>

QT_BEGIN_NAMESPACE

class QGraphsView;
class QBarSeries;
class QBarSet;
class QAbstractSeries;
class QQuickTapHandler;

class BarsRenderer : public QQuickItem
{
    Q_OBJECT
public:
    BarsRenderer(QGraphsView *graph, bool clipPlotArea);
    ~BarsRenderer() override;

    void handlePolish(QBarSeries *series, int barSeriesIndex, int barSeriesCount);
    void updateSeries(QBarSeries *series);
    void afterUpdate(QList<QAbstractSeries *> &cleanupSeries);
    void afterPolish(QList<QAbstractSeries *> &cleanupSeries);
    bool handleHoverMove(QHoverEvent *event);

Q_SIGNALS:

private:
    struct BarSelectionRect {
        QBarSeries *series = nullptr;
        QBarSet *barSet = nullptr;
        QList<QRectF> rects;
    };
    struct BarSeriesData {
        QRectF rect;
        QColor color;
        QColor borderColor;
        QString label;
        QColor labelColor;
        float value;
        float borderWidth;
        bool isSelected;
    };

    void updateVerticalBars(QBarSeries *series, qsizetype setCount, qsizetype valuesPerSet,
                            int barSeriesIndex, int barSeriesCount);
    void updateHorizontalBars(QBarSeries *series, qsizetype setCount, qsizetype valuesPerSet,
                              int barSeriesIndex, int barSeriesCount);
    QColor getSetColor(QBarSeries *series, QBarSet *set, qsizetype barSerieIndex);
    QColor getSetSelectedColor(QBarSeries *series, QBarSet *set);
    QColor getSetBorderColor(QBarSeries *series, QBarSet *set, qsizetype barSerieIndex);
    qreal getSetBorderWidth(QBarSeries *series, QBarSet *set);
    QString generateLabelText(QBarSeries *series, qreal value);
    void positionLabelItem(QBarSeries *series, QQuickText *textItem, const BarSeriesData &d);
    void updateComponents(QBarSeries *series);
    void updateValueLabels(QBarSeries *series);

    void onSingleTapped(QEventPoint eventPoint, Qt::MouseButton button);
    void onDoubleTapped(QEventPoint eventPoint, Qt::MouseButton button);
    void onPressedChanged();

    QGraphsView *m_graph = nullptr;
    QHash<QBarSeries *, QList<BarSelectionRect>> m_rectNodesInputRects;
    QHash<QBarSeries *, QList<QQuickItem *>> m_barItems;
    QHash<QBarSeries *, QList<QQuickText *>> m_labelTextItems;
    QHash<QBarSeries *, QList<BarSeriesData>> m_seriesData;

    QQuickTapHandler *m_tapHandler = nullptr;

    QBarSeries *m_currentHoverSeries = nullptr;
    qsizetype m_colorIndex = -1;
    // Margin between bars.
    float m_barMargin = 2.0;
};

QT_END_NAMESPACE

#endif // BARSRENDERER_H
