// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEGFILL_P_H
#define QLOTTIEGFILL_P_H

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

#include <QVector4D>
#include <QGradient>

#include <QtLottie/private/qlottiegroup_p.h>
#include <QtLottie/private/qlottieproperty_p.h>
#include <QtLottie/private/qlottieproperty_p.h>
#include <QtLottie/private/qlottiespatialproperty_p.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIE_EXPORT QLottieGFill : public QLottieShape
{
public:
    explicit QLottieGFill(const QLottieGFill &other);
    QLottieGFill(QLottieBase *parent = nullptr);
    ~QLottieGFill() override;

    QLottieBase *clone() const override;

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;
    int parse(const QJsonObject &definition) override;

    QGradient *value() const;
    QGradient::Type gradientType() const;
    QPointF startPoint() const;
    QPointF endPoint() const;
    qreal highlightLength() const;
    qreal highlightAngle() const;
    qreal opacity() const;
    Qt::FillRule fillRule() const;

private:
    void setGradient();

protected:
    QLottieProperty<qreal> m_opacity;
    QLottieSpatialProperty m_startPoint;
    QLottieSpatialProperty m_endPoint;
    QLottieProperty<qreal> m_highlightLength;
    QLottieProperty<qreal> m_highlightAngle;
    QHash<qreal, QLottieProperty4D<QVector4D>> m_colors;
    QGradient *m_gradient = nullptr;
    Qt::FillRule m_fillRule = Qt::WindingFill;
};

QT_END_NAMESPACE

#endif // QLOTTIEGFILL_P_H
