// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef POINTRENDERER_H
#define POINTRENDERER_H

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
#include <QtGraphs/qabstractseries.h>
#include <QtQuick/private/qsgdefaultinternalrectanglenode_p.h>
#include <QtQuickShapes/private/qquickshape_p.h>
#include <QPainterPath>

QT_BEGIN_NAMESPACE

class QGraphsView;
class QXYSeries;
class QLineSeries;
class QScatterSeries;
class QSplineSeries;
class AxisRenderer;
class QQuickTapHandler;
class QQuickDragHandler;
struct QLegendData;

class PointRenderer : public QQuickItem
{
    Q_OBJECT
public:
    PointRenderer(QGraphsView *graph, bool clipPlotArea);
    ~PointRenderer() override;

    void resetShapePathCount();

    void handlePolish(QXYSeries *series);
    void afterPolish(QList<QAbstractSeries *> &cleanupSeries);
    void updateSeries(QXYSeries *series);
    void afterUpdate(QList<QAbstractSeries *> &cleanupSeries);
    bool handleHoverMove(QHoverEvent *event);

   QPointF reverseRenderCoordinates(QAbstractSeries *series, qreal x, qreal y);

private:
    struct PointGroup
    {
        QXYSeries *series = nullptr;
        QQuickShapePath *shapePath = nullptr;
        QPainterPath painterPath;
        QList<QQuickItem *> markers;
        QList<QQuickDragHandler *> dragHandlers;
        QQmlComponent *currentMarker = nullptr;
        QQmlComponent *previousMarker = nullptr;
        QList<QRectF> rects;
        qsizetype colorIndex = -1;
        bool hover = false;
    };

    QQmlComponent *m_tempMarker = nullptr;

    QGraphsView *m_graph = nullptr;
    QQuickShape m_shape;
    QMap<QXYSeries *, PointGroup *> m_groups;
    qsizetype m_currentShapePathIndex = 0;

    // Point drag variables
    QPoint m_previousDelta;
    PointGroup *m_pressedGroup = nullptr;
    qsizetype m_pressedPointIndex = 0;

    // Render area variables
    qreal m_maxVertical = 0;
    qreal m_maxHorizontal = 0;
    qreal m_verticalOffset = 0;
    qreal m_horizontalOffset = 0;
    qreal m_areaWidth = 0;
    qreal m_areaHeight = 0;

    QQuickTapHandler *m_tapHandler = nullptr;

    qreal defaultSize(QXYSeries *series = nullptr);

    struct SeriesStyle {
        QColor color;
        QColor selectedColor;
        QColor borderColor;
        qreal borderWidth;
    };

    SeriesStyle getSeriesStyle(PointGroup *group);

    void calculateRenderCoordinates(AxisRenderer *axisRenderer,
                                    QAbstractSeries *series,
                                    qreal origX,
                                    qreal origY,
                                    qreal *renderX,
                                    qreal *renderY);
    void reverseRenderCoordinates(AxisRenderer *axisRenderer,
                                  QAbstractSeries *series,
                                  qreal renderX,
                                  qreal renderY,
                                  qreal *origX,
                                  qreal *origY);
    void updatePointDelegate(
        QXYSeries *series, PointGroup *group, qsizetype pointIndex, qreal x, qreal y);
    void hidePointDelegates(QXYSeries *series);
    void updateLegendData(QXYSeries *series, QLegendData &legendData);

    void onSingleTapped(QEventPoint eventPoint, Qt::MouseButton button);
    void onDoubleTapped(QEventPoint eventPoint, Qt::MouseButton button);
    void onPressedChanged();

#ifdef USE_SCATTERGRAPH
    void updateScatterSeries(QScatterSeries *scatter, QLegendData &legendData);
#endif
#ifdef USE_LINEGRAPH
    void updateLineSeries(QLineSeries *line, QLegendData &legendData);
#endif
#ifdef USE_SPLINEGRAPH
    void updateSplineSeries(QSplineSeries *spline, QLegendData &legendData);
#endif
};

QT_END_NAMESPACE

#endif // POINTRENDERER_H
