// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qtgahandler.h"
#include "qtgafile.h"

#include <QtCore/QVariant>
#include <QtCore/QDebug>
#include <QtGui/QImage>

QT_BEGIN_NAMESPACE

QTgaHandler::QTgaHandler()
    : QImageIOHandler()
    , tga(0)
{
}

QTgaHandler::~QTgaHandler()
{
    delete tga;
}

bool QTgaHandler::canRead() const
{
    if (!tga)
        tga = new QTgaFile(device());
    if (tga->isValid())
    {
        setFormat("tga");
        return true;
    }
    qWarning("QTgaHandler::canRead(): %s", qPrintable(tga->errorMessage()));
    return false;
}

bool QTgaHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("QTgaHandler::canRead() called with no device");
        return false;
    }

    // TGA reader implementation needs a seekable QIODevice, so
    // sequential devices are not supported
    if (device->isSequential())
        return false;
    qint64 pos = device->pos();
    bool isValid;
    {
        QTgaFile tga(device);
        isValid = tga.isValid();
    }
    device->seek(pos);
    return isValid;
}

bool QTgaHandler::read(QImage *image)
{
    if (!canRead())
        return false;
    *image = tga->readImage();
    return !image->isNull();
}

QVariant QTgaHandler::option(ImageOption option) const
{
    if (option == Size && canRead()) {
        return tga->size();
    } else if (option == CompressionRatio) {
        return tga->compression();
    } else if (option == ImageFormat) {
        return QImage::Format_ARGB32;
    }
    return QVariant();
}

void QTgaHandler::setOption(ImageOption option, const QVariant &value)
{
    Q_UNUSED(option);
    Q_UNUSED(value);
}

bool QTgaHandler::supportsOption(ImageOption option) const
{
    return option == CompressionRatio
            || option == Size
            || option == ImageFormat;
}

QT_END_NAMESPACE
