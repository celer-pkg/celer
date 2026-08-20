// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Alex Char.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGui/qimageiohandler.h>
#include "qicnshandler_p.h"

#ifndef QT_NO_IMAGEFORMATPLUGIN
#ifndef QT_NO_DATASTREAM

QT_BEGIN_NAMESPACE

class QICNSPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "icns.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

QImageIOPlugin::Capabilities QICNSPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == QByteArrayLiteral("icns"))
        return Capabilities(CanRead | CanWrite);
    Capabilities cap;
    if (!format.isEmpty())
        return cap;
    if (!device || !device->isOpen())
        return cap;

    if (device->isReadable() && QICNSHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}

QImageIOHandler *QICNSPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new QICNSHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

QT_END_NAMESPACE

#include "main.moc"

#endif // QT_NO_DATASTREAM

#endif // QT_NO_IMAGEFORMATPLUGIN
