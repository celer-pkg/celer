// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qmacheifhandler.h"
#include <QVariant>

QT_BEGIN_NAMESPACE

QMacHeifHandler::QMacHeifHandler()
    : d(new QIIOFHelper(this))
{
}

QMacHeifHandler::~QMacHeifHandler()
{
}

bool QMacHeifHandler::canRead(QIODevice *iod)
{
    bool bCanRead = false;
    char buf[12];
    if (iod && iod->peek(buf, 12) == 12) {
        bCanRead = (!qstrncmp(buf + 4, "ftyp", 4) &&
                    (!qstrncmp(buf + 8, "heic", 4) ||
                     !qstrncmp(buf + 8, "heix", 4) ||
                     !qstrncmp(buf + 8, "msf1", 4) ||
                     !qstrncmp(buf + 8, "mif1", 4)));
    }
    return bCanRead;
}

bool QMacHeifHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("heic");
        return true;
    }
    return false;
}

bool QMacHeifHandler::read(QImage *image)
{
    return d->readImage(image);
}

bool QMacHeifHandler::write(const QImage &image)
{
    return d->writeImage(image, QStringLiteral("public.heic"));
}

QVariant QMacHeifHandler::option(ImageOption option) const
{
    return d->imageProperty(option);
}

void QMacHeifHandler::setOption(ImageOption option, const QVariant &value)
{
    d->setOption(option, value);
}

bool QMacHeifHandler::supportsOption(ImageOption option) const
{
    return option == Quality
        || option == Size
        || option == ImageTransformation;
}

QT_END_NAMESPACE
