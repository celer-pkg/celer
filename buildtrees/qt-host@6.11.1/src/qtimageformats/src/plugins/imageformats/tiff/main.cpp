// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <qimageiohandler.h>
#include <qdebug.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#include "qtiffhandler_p.h"

QT_BEGIN_NAMESPACE

class QTiffPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "tiff.json")

public:
    Capabilities capabilities(QIODevice * device, const QByteArray & format) const override;
    QImageIOHandler * create(QIODevice * device, const QByteArray & format = QByteArray()) const override;
};

QImageIOPlugin::Capabilities QTiffPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "tiff" || format == "tif")
        return Capabilities(CanRead | CanWrite);
    Capabilities cap;
    if (!format.isEmpty())
        return cap;
    if (!device->isOpen())
        return cap;

    if (device->isReadable() && QTiffHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}

QImageIOHandler* QTiffPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *tiffHandler = new QTiffHandler();
    tiffHandler->setDevice(device);
    tiffHandler->setFormat(format);
    return tiffHandler;
}

QT_END_NAMESPACE

#include "main.moc"

#endif /* QT_NO_IMAGEFORMATPLUGIN */
