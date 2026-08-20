// Copyright (C) 2024 The Qt Company Ltd.
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

#ifndef QSPLINE3DSERIES_P_H
#define QSPLINE3DSERIES_P_H

#include <QtGraphs/qspline3dseries.h>
#include <private/qscatter3dseries_p.h>

QT_BEGIN_NAMESPACE

class QSpline3DSeriesPrivate : public QScatter3DSeriesPrivate
{
    Q_DECLARE_PUBLIC(QSpline3DSeries)

public:
    QSpline3DSeriesPrivate();
    ~QSpline3DSeriesPrivate() override;

private:
    bool m_splineVisible = true;
    qreal m_tension = 0;
    qreal m_knotting = 0.5;
    bool m_looping = false;
    QColor m_splineColor = QColor(255, 0, 0);
    qsizetype m_resolution = 10;
};

QT_END_NAMESPACE

#endif // QSPLINE3DSERIES_P_H
