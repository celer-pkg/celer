// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qmacjp2handler.h"
#include "qiiofhelpers_p.h"
#include <QVariant>

QT_BEGIN_NAMESPACE

using namespace NS_IIOF_HELPERS;

class QMacJp2HandlerPrivate
{
    Q_DECLARE_PUBLIC(QMacJp2Handler)
    Q_DISABLE_COPY(QMacJp2HandlerPrivate)
public:
    QMacJp2HandlerPrivate(QMacJp2Handler *q_ptr)
        : writeQuality(-1), q_ptr(q_ptr)
    {}

    int writeQuality;
    QMacJp2Handler *q_ptr;
};


QMacJp2Handler::QMacJp2Handler()
    : d_ptr(new QMacJp2HandlerPrivate(this))
{
}

QMacJp2Handler::~QMacJp2Handler()
{
}

bool QMacJp2Handler::canRead(QIODevice *iod)
{
    bool bCanRead = false;
    char buf[12];
    if (iod && iod->peek(buf, 12) == 12)
        bCanRead = !memcmp(buf, "\000\000\000\fjP  \r\n\207\n", 12);
    return bCanRead;
}

bool QMacJp2Handler::canRead() const
{
    if (canRead(device())) {
        setFormat("jp2");
        return true;
    }
    return false;
}

bool QMacJp2Handler::read(QImage *image)
{
    return QIIOFHelpers::readImage(this, image);
}

bool QMacJp2Handler::write(const QImage &image)
{
    return QIIOFHelpers::writeImage(this, image, QStringLiteral("public.jpeg-2000"));
}

QVariant QMacJp2Handler::option(ImageOption option) const
{
    Q_D(const QMacJp2Handler);
    if (option == Quality)
        return QVariant(d->writeQuality);
    return QVariant();
}

void QMacJp2Handler::setOption(ImageOption option, const QVariant &value)
{
    Q_D(QMacJp2Handler);
    if (option == Quality) {
        bool ok;
        const int quality = value.toInt(&ok);
        if (ok)
            d->writeQuality = quality;
    }
}

bool QMacJp2Handler::supportsOption(ImageOption option) const
{
    return (option == Quality);
}

QT_END_NAMESPACE
