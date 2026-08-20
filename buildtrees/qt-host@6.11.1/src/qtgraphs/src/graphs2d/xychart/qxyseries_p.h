// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Graphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QXYSERIES_P_H
#define QXYSERIES_P_H

#include <QtGraphs/qxyseries.h>
#include <private/qabstractseries_p.h>
#include <private/qgraphtransition_p.h>

QT_BEGIN_NAMESPACE

class QAbstractAxis;

class QXYSeriesPrivate : public QAbstractSeriesPrivate
{
public:
    static QXYSeriesPrivate *get(QXYSeries *item) { return item->d_func(); }
    static const QXYSeriesPrivate *get(const QXYSeries *item) { return item->d_func(); }

    QXYSeriesPrivate(QAbstractSeries::SeriesType type);

    void setPointSelected(qsizetype index, bool selected, bool &callSignal);
    bool isPointSelected(qsizetype index) const;

    void append(const QList<QPointF> &points);

protected:
    QList<QPointF> m_points;
    QSet<qsizetype> m_selectedPoints;
    QColor m_color = QColor(Qt::transparent);
    QColor m_selectedColor = QColor(Qt::transparent);
    QQmlComponent *m_pointDelegate = nullptr;
    QGraphTransition *m_graphTransition = nullptr;
    bool m_draggable = false;

private:
    Q_DECLARE_PUBLIC(QXYSeries)

    friend class QGraphPointAnimation;
    friend class QGraphTransition;
};

QT_END_NAMESPACE

#endif
