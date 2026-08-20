// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QPIESERIES_P_H
#define QPIESERIES_P_H

//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include <private/qabstractseries_p.h>
#include <QtGraphs/qpieseries.h>

QT_BEGIN_NAMESPACE

class QPieSeriesPrivate : public QAbstractSeriesPrivate
{
public:
    QPieSeriesPrivate();
    ~QPieSeriesPrivate() = default;

    void updateData(bool clearHidden = false);
    void updateLabels();
    void setSizes(qreal innerSize, qreal outerSize);

private:
    QList<QPieSlice *> m_slices;
    qreal m_pieRelativeHorPos;
    qreal m_pieRelativeVerPos;
    qreal m_pieRelativeSize;
    qreal m_pieStartAngle;
    qreal m_pieEndAngle;
    qreal m_sum;
    qreal m_holeRelativeSize;
    qreal m_angleSpanVisibleLimit;
    QPieSeries::LabelVisibility m_angleSpanVisibleMode;
    Q_DECLARE_PUBLIC(QPieSeries)
};

QT_END_NAMESPACE

#endif // QPIESERIES_P_H
