// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEIMAGE_P_H
#define QLOTTIEIMAGE_P_H

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

#include <QImage>
#include <QPointF>
#include <QUrl>

#include <QtLottie/private/qlottieproperty_p.h>
#include <QtLottie/private/qlottiespatialproperty_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class Q_LOTTIE_EXPORT QLottieImage : public QLottieBase
{
public:
    explicit QLottieImage(const QLottieImage &other);
    QLottieImage(QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    int construct(const QJsonObject &definition);

    void render(QLottieRenderer &renderer) const override;

    QUrl url() const { return m_url; }
    QSizeF size() const { return m_size; }
    QImage image() const { return m_image; }

protected:
    QUrl m_url;
    QSizeF m_size;
    QImage m_image;
};

QT_END_NAMESPACE

#endif // QLOTTIEIMAGE_P_H
