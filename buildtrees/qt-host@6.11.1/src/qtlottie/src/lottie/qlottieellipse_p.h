// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEELLIPSE_P_H
#define QLOTTIEELLIPSE_P_H

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

class Q_LOTTIE_EXPORT QLottieEllipse : public QLottieShape
{
public:
    explicit QLottieEllipse(const QLottieEllipse &other);
    QLottieEllipse(QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;
    int parse(const QJsonObject &definition) override;

    bool acceptsTrim() const override;

    QPointF position() const;
    QSizeF size() const;

protected:
    QLottieSpatialProperty m_position;
    QLottieProperty2D<QSizeF> m_size;
};

QT_END_NAMESPACE

#endif // QLOTTIEELLIPSE_P_H
