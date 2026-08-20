// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef PIERENDERER_H
#define PIERENDERER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include "graphs2d/qabstractseries.h"
#include <QQuickItem>
#include <QtGui/qpainterpath.h>

QT_BEGIN_NAMESPACE

class QGraphsView;
class QPieSeries;
class QPieSlice;
class QQuickShape;
class QAbstractSeries;
class QQuickTapHandler;
class QQuickDragHandler;

class PieRenderer : public QQuickItem
{
    Q_OBJECT
public:
    PieRenderer(QGraphsView *graph, bool clipPlotArea);
    ~PieRenderer() override;

    void updateActiveSlices(QPieSeries *series, QList<QPieSlice *> slicelist);
    void handlePolish(QPieSeries *series);
    void handleSlicesPolish(QPieSeries *series,
                            QList<QLegendData> &legendDataList,
                            QList<QPieSlice *> slicelist,
                            qreal startAngle,
                            qreal endAngle,
                            QPointF center,
                            qreal radius,
                            qreal radiusRatio);
    void afterPolish(QList<QAbstractSeries *> &cleanupSeries);
    void handleSlicesAfterPolish(QList<QPieSlice *> slicelist);
    void updateSeries(QPieSeries *series);
    void afterUpdate(QList<QAbstractSeries *> &cleanupSeries);
    void markedDeleted(QList<QPieSlice *> deleted);

    void setSize(QSizeF size);

    bool handleHoverMove(QHoverEvent *event);

private:
    struct SliceData
    {
        bool initialized;
    };

    struct DragState
    {
        bool dragging = false;
        qreal dragSeriesRadius = 0.0;
        QVector2D dragSeriesCenter;
    };

    Qt::Edge m_closerEdge = Qt::LeftEdge;

    void onSingleTapped(QEventPoint eventPoint, Qt::MouseButton button);
    void onDoubleTapped(QEventPoint eventPoint, Qt::MouseButton button);
    void onPressedChanged();

    void onGrabChanged(QPointingDevice::GrabTransition transition, QEventPoint eventPoint);
    void onTranslationChanged(QVector2D delta);

    bool isPointInSlice(QPointF point, QPieSlice *slice, qreal *angle = nullptr);
    bool isPointInSubSlices(QPointF point, QPieSlice *slice);

    qreal distanceToSegment(const QVector2D p, const QVector2D segmentStart,
                            const QVector2D segmentEnd);

    QGraphsView *m_graph = nullptr;
    QQuickShape *m_shape = nullptr;
    QHash<QPieSlice *, SliceData> m_activeSlices;

    QQuickTapHandler *m_tapHandler = nullptr;
    QPieSlice *m_currentHoverSlice = nullptr;

    QQuickDragHandler *m_dragHandler = nullptr;
    QPieSlice *m_dragSlice = nullptr;
    DragState m_dragState;

    QPainterPath m_painterPath;
    qsizetype m_colorIndex = -1;
};

QT_END_NAMESPACE

#endif // QPIERENDERER_H
