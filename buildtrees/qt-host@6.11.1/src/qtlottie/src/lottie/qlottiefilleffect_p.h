// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTIEFILLEFFECT_P_H
#define QLOTIEFILLEFFECT_P_H

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

#include <QColor>
#include <QVector4D>

#include <QtLottie/private/qlottiebase_p.h>
#include <QtLottie/private/qlottieproperty_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class Q_LOTTIE_EXPORT QLottieFillEffect : public QLottieBase
{
public:
    QLottieFillEffect() = default;
    explicit QLottieFillEffect(const QLottieFillEffect &other);

    QLottieBase *clone() const override;

    void construct(const QJsonObject &definition);

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;

    QColor color() const;
    qreal opacity() const;

protected:
    QLottieProperty4D<QVector4D> m_color;
    QLottieProperty<qreal> m_opacity;
};

QT_END_NAMESPACE

#endif // QLOTIEFILLEFFECT_P_H
