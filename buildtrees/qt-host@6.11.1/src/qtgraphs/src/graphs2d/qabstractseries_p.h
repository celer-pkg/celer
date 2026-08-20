// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QABSTRACTSERIES_P_H
#define QABSTRACTSERIES_P_H

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
#include <QtGraphs/qabstractseries.h>
#include <private/qobject_p.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcSeries2D)
Q_DECLARE_LOGGING_CATEGORY(lcProperties2D)

class QAbstractAxis;
class QGraphsView;

class QAbstractSeriesPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractSeries)
public:
    static QAbstractSeriesPrivate *get(QAbstractSeries *item) { return item->d_func(); }
    static const QAbstractSeriesPrivate *get(const QAbstractSeries *item) { return item->d_func(); }

    explicit QAbstractSeriesPrivate(QAbstractSeries::SeriesType type);
    ~QAbstractSeriesPrivate() override;

    void removeAxis(QAbstractAxis *axis);

    void setLegendData(const QList<QLegendData> &legendData);
    void clearLegendData();
    QAbstractSeries::SeriesType type() { return m_type; }

    static void appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element);

protected:
    QGraphsView *m_graph = nullptr;

private:
    QString m_name;
    bool m_visible = true;
    bool m_loaded = false;
    bool m_selectable = false;
    bool m_hoverable = false;
    bool m_hovered = false;
    qreal m_opacity = 1.0;
    qreal m_valuesMultiplier = 1.0;
    QList<QLegendData> m_legendData;
    int m_drawOrder = 0;

    QAbstractAxis *m_axisX = nullptr;
    QAbstractAxis *m_axisY = nullptr;

    QAbstractSeries::SeriesType m_type;
};

QT_END_NAMESPACE

#endif
