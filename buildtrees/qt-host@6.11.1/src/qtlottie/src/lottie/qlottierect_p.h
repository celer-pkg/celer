// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIERECT_P_H
#define QLOTTIERECT_P_H

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

#include <QtLottie/private/qlottieshape_p.h>
#include <QtLottie/private/qlottieproperty_p.h>
#include <QtLottie/private/qlottiespatialproperty_p.h>
#include <QtLottie/private/qlottiefill_p.h>
#include <QtLottie/private/qlottiestroke_p.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIE_EXPORT QLottieRect : public QLottieShape
{
public:
    explicit QLottieRect(const QLottieRect &other);
    QLottieRect(QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    void construct(const QJsonObject &definition);

    bool setProperty(QLottieLiteral::PropertyType propertyType, QVariant value) override;

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;
    bool acceptsTrim() const override;
    int parse(const QJsonObject &definition) override;

    QPointF position() const;
    QSizeF size() const;
    qreal roundness() const;

protected:
    QLottieSpatialProperty m_position;
    QLottieProperty2D<QSizeF> m_size;
    QLottieProperty<qreal> m_roundness;
};

QT_END_NAMESPACE

#endif // QLOTTIERECT_P_H
