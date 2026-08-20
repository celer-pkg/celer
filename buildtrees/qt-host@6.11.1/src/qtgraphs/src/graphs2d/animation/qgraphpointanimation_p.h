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

#ifndef QGRAPHPOINTANIMATION_H
#define QGRAPHPOINTANIMATION_H

#include <QtCore/QPointF>
#include "private/qgraphanimation_p.h"
#include "private/qxyseriesanimation_p.h"
#include "qqmlintegration.h"

QT_BEGIN_NAMESPACE

class QXYSeries;

class QGraphPointAnimation : public QXYSeriesAnimation
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GraphPointAnimation)

public:
    explicit QGraphPointAnimation(QObject *parent = nullptr);
    ~QGraphPointAnimation() override;

    GraphAnimationType animationType() override;
    void setAnimatingValue(const QVariant &start, const QVariant &end) override;
    QVariant interpolated(const QVariant &start, const QVariant &end, qreal progress) const override;

    void animate() override;
    void end() override;

public Q_SLOTS:
    void valueUpdated(const QVariant &value) override;
};

QT_END_NAMESPACE

#endif // QGRAPHPOINTANIMATION_H
