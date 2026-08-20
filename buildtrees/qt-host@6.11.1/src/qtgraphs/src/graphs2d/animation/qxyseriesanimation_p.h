// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QXYSERIESANIMATION_H
#define QXYSERIESANIMATION_H

#include <QtCore/QObject>
#include <QtCore/QPointF>
#include "private/qgraphtransition_p.h"

QT_BEGIN_NAMESPACE

class QXYSeriesAnimation : public QGraphAnimation
{
    Q_OBJECT
public:
    explicit QXYSeriesAnimation(QObject *parent = nullptr);
    ~QXYSeriesAnimation() override;

    void updateCurrent(QGraphTransition::TransitionType tt, int index, QPointF point);

protected:
    QGraphTransition::TransitionType m_currentTransitionType;
    QGraphTransition::TransitionType m_previousTransitionType;
    int m_activePointIndex;
    int m_newPointIndex;
    QPointF m_newPoint;

    // QGraphAnimation interface
public:
    void setAnimatingValue(const QVariant &start, const QVariant &end) override = 0;
    void animate() override = 0;
    void end() override = 0;

public Q_SLOTS:
    void valueUpdated(const QVariant &value) override = 0;
};

QT_END_NAMESPACE

#endif // QXYSERIESANIMATION_H
