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

#ifndef QAREASERIES_P_H
#define QAREASERIES_P_H

#include <QtGraphs/qareaseries.h>
#include <private/qabstractseries_p.h>
#include <QtQuickShapes/private/qquickshape_p.h>

QT_BEGIN_NAMESPACE

class QAreaSeriesPrivate : public QAbstractSeriesPrivate
{
public:
    QAreaSeriesPrivate();

protected:
    QColor m_color = QColor(Qt::transparent);
    QColor m_selectedColor = QColor(Qt::transparent);
    QColor m_borderColor = QColor(Qt::transparent);
    QColor m_selectedBorderColor = QColor(Qt::transparent);
    qreal m_borderWidth = -1.0;
    bool m_selected = false;
    QXYSeries *m_upperSeries = nullptr;
    QXYSeries *m_lowerSeries = nullptr;
    QQuickShapeGradient *m_gradient = nullptr;
    QQuickShapeGradient *m_selectedGradient = nullptr;

private:
    Q_DECLARE_PUBLIC(QAreaSeries)
};

QT_END_NAMESPACE

#endif
