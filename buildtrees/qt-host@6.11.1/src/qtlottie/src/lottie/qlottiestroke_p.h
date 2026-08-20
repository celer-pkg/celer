// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIESTROKE_P_H
#define QLOTTIESTROKE_P_H

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

#include <QPen>
#include <QVector4D>

#include <QtLottie/private/qlottieshape_p.h>
#include <QtLottie/private/qlottieproperty_p.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIE_EXPORT QLottieStroke : public QLottieShape
{
public:
    explicit QLottieStroke(const QLottieStroke &other);
    QLottieStroke(QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;
    int parse(const QJsonObject &definition) override;

    QPen pen() const;
    qreal opacity() const;

    QLottieProperty4D<QVector4D> colorProperty() const { return m_color; }
    QLottieProperty<qreal> opacityProperty() const { return m_opacity; }

protected:
    QColor getColor() const;

protected:
    QLottieProperty<qreal> m_opacity;
    QLottieProperty<qreal> m_width;
    QLottieProperty4D<QVector4D> m_color;
    Qt::PenCapStyle m_capStyle;
    Qt::PenJoinStyle m_joinStyle;
    qreal m_miterLimit = 0;
    QLottieProperty<qreal> m_dashOffset;
    QLottieProperty<qreal> m_dashLength;
    QLottieProperty<qreal> m_dashGap;
    bool m_isDashed = false;
};

QT_END_NAMESPACE

#endif // QLOTTIESTROKE_P_H
