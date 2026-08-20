// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <qimageiohandler.h>
#include <qstringlist.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#include "qwebphandler_p.h"

#include <qiodevice.h>
#include <qbytearray.h>

QT_BEGIN_NAMESPACE

class QWebpPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "webp.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

QWebpPlugin::Capabilities QWebpPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "webp")
        return Capabilities(CanRead | CanWrite);

    Capabilities cap;
    if (!format.isEmpty())
        return cap;
    if (!device->isOpen())
        return cap;

    if (device->isReadable() && QWebpHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;

    return cap;
}

QImageIOHandler *QWebpPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new QWebpHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

QT_END_NAMESPACE

#include "main.moc"

#endif // !QT_NO_IMAGEFORMATPLUGIN
