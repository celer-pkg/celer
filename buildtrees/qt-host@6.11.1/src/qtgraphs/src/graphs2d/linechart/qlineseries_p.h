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

#ifndef QLINESERIES_P_H
#define QLINESERIES_P_H

#include <QtGraphs/qlineseries.h>
#include <private/qgraphpointanimation_p.h>
#include <private/qxyseries_p.h>

QT_BEGIN_NAMESPACE

class QLineSeriesPrivate : public QXYSeriesPrivate
{
public:
    QLineSeriesPrivate();

protected:
    qreal m_width = 2.0;
    Qt::PenCapStyle m_capStyle = Qt::PenCapStyle::SquareCap;
    Qt::PenJoinStyle m_joinStyle = Qt::PenJoinStyle::BevelJoin;
    QLineSeries::LineStyle m_lineStyle = QLineSeries::LineStyle::Straight;
    QLineSeries::StrokeStyle m_strokeStyle = QLineSeries::StrokeStyle::SolidLine;
    qreal m_dashOffset = 0.0;
    QList<qreal> m_dashPattern = {4, 2};

private:
    Q_DECLARE_PUBLIC(QLineSeries)
};

QT_END_NAMESPACE

#endif
