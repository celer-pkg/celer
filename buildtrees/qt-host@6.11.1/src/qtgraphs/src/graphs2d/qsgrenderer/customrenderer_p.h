// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef CUSTOMRENDERER_H
#define CUSTOMRENDERER_H

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
#include <private/qquickdraghandler_p.h>

QT_BEGIN_NAMESPACE

class QGraphsView;
class QCustomSeries;
class AxisRenderer;
struct QLegendData;

class CustomRenderer : public QQuickItem
{
    Q_PROPERTY(QRectF plotArea READ plotArea NOTIFY plotAreaChanged FINAL)

    Q_OBJECT
public:
    CustomRenderer(QGraphsView *graph, bool clipPlotArea);
    ~CustomRenderer() override;

    void handlePolish(QCustomSeries *series);
    void afterPolish(QList<QAbstractSeries *> &cleanupSeries);
    void updateSeries(QCustomSeries *series);
    void afterUpdate(QList<QAbstractSeries *> &cleanupSeries);

    qreal mapX(AxisRenderer *axisRenderer, QCustomSeries *series, qreal x);
    qreal mapY(AxisRenderer *axisRenderer, QCustomSeries *series, qreal y);

    QRectF plotArea() const;

signals:
    void plotAreaChanged();

private:
    struct DataGroup
    {
        QCustomSeries *series = nullptr;
        QList<QQuickItem *> dataItems;
        QQmlComponent *currentDataItem = nullptr;
        QQmlComponent *previousDataItem = nullptr;
    };

    QGraphsView *m_graph = nullptr;
    QMap<QCustomSeries *, DataGroup *> m_groups;

    // Render area variables
    qreal m_maxVertical = 0;
    qreal m_maxHorizontal = 0;
    qreal m_verticalOffset = 0;
    qreal m_horizontalOffset = 0;
    qreal m_areaWidth = 0;
    qreal m_areaHeight = 0;
    QRectF m_plotArea;

    void hideDelegates(QCustomSeries *series);
};

QT_END_NAMESPACE

#endif // CUSTOMRENDERER_H
