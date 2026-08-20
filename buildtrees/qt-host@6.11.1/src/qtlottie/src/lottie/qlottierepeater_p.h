// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEREPEATER_P_H
#define QLOTTIEREPEATER_P_H

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

#include <QtLottie/qtlottieexports.h>
#include <QtLottie/private/qlottieshape_p.h>
#include <QtLottie/private/qlottieproperty_p.h>
#include <QtLottie/private/qlottierepeatertransform_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class Q_LOTTIE_EXPORT QLottieRepeater : public QLottieShape
{
public:
    QLottieRepeater() = default;
    explicit QLottieRepeater(const QLottieRepeater &other) = default;
    QLottieRepeater(const QJsonObject &definition, QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    void construct(const QJsonObject &definition);

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;

    int copies() const;
    qreal offset() const;
    const QLottieRepeaterTransform &transform() const;

protected:
    QLottieProperty<int> m_copies;
    QLottieProperty<qreal> m_offset;
    QLottieRepeaterTransform m_transform;
};

QT_END_NAMESPACE

#endif // QLOTTIEREPEATER_P_H
