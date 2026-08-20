// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEGROUP_P_H
#define QLOTTIEGROUP_P_H

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

#include <QJsonObject>
#include <QColor>

#include <QtLottie/private/qlottieshape_p.h>
#include <QtLottie/private/qlottieproperty_p.h>

QT_BEGIN_NAMESPACE

class QLottieFill;
class QLottieTrimPath;

class Q_LOTTIE_EXPORT QLottieGroup : public QLottieShape
{
public:
    QLottieGroup(QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;
    int parse(const QJsonObject &definition) override;

    bool acceptsTrim() const override;
    void applyTrim(const QLottieTrimPath  &trimmer) override;
};

QT_END_NAMESPACE

#endif // QLOTTIEGROUP_P_H
