// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEFLATLAYERS_P_H
#define QLOTTIEFLATLAYERS_P_H

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

#include <QtLottie/private/qlottielayer_p.h>
#include <QtGui/QColor>

QT_BEGIN_NAMESPACE

class QJsonObject;
class QLottieRenderer;

class Q_LOTTIE_EXPORT QLottieNullLayer : public QLottieLayer
{
public:
    QLottieNullLayer() = default;
    explicit QLottieNullLayer(const QLottieNullLayer &other);

    QLottieBase *clone() const override;

    void render(QLottieRenderer &render) const override;
    int parse(const QJsonObject &definition) override;
};

class Q_LOTTIE_EXPORT QLottieSolidLayer : public QLottieLayer
{
public:
    QLottieSolidLayer() = default;
    explicit QLottieSolidLayer(const QLottieSolidLayer &other);

    QLottieBase *clone() const override;

    void render(QLottieRenderer &render) const override;
    int parse(const QJsonObject &definition) override;

    QColor color() const;

private:
    QColor m_color;
};

class Q_LOTTIE_EXPORT QLottieImageLayer : public QLottieLayer
{
public:
    QLottieImageLayer() = default;
    explicit QLottieImageLayer(const QLottieImageLayer &other);

    QLottieBase *clone() const override;

    void render(QLottieRenderer &render) const override;
    int parse(const QJsonObject &definition) override;
};

QT_END_NAMESPACE

#endif // QLOTTIEFLATLAYERS_P_H
