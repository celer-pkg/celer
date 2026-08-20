// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEPOLYSTAR_P_H
#define QLOTTIEPOLYSTAR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QRect>
#include <QPointF>
#include <QBrush>
#include <QPen>
#include <QPainterPath>

#include <QtLottie/private/qlottieshape_p.h>
#include <QtLottie/private/qlottieproperty_p.h>
#include <QtLottie/private/qlottiespatialproperty_p.h>
#include <QtLottie/private/qlottiefill_p.h>
#include <QtLottie/private/qlottiestroke_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class Q_LOTTIE_EXPORT QLottiePolyStar : public QLottieShape
{
public:
    explicit QLottiePolyStar(const QLottiePolyStar &other);
    QLottiePolyStar(QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    int parse(const QJsonObject &definition) override;

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;

    bool acceptsTrim() const override;

    QPointF position() const;
    qreal outerRadius() const;
    qreal innerRadius() const;
    qreal startAngle() const;
    int pointCount() const;
    bool isPolygonModeEnabled() const;

protected:
    QLottieSpatialProperty m_position;
    QLottieProperty<int> m_pointCount;
    QLottieProperty<qreal> m_outerRadius;
    QLottieProperty<qreal> m_innerRadius;
    QLottieProperty<qreal> m_startAngle;
    bool m_polygonMode = false;
};

QT_END_NAMESPACE

#endif // QLOTTIEPOLYSTAR_P_H
