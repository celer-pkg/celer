// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <qimageiohandler.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEFORMAT_WBMP
#undef QT_NO_IMAGEFORMAT_WBMP
#endif

#include "qwbmphandler_p.h"

QT_BEGIN_NAMESPACE

class QWbmpPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "wbmp.json")

public:
    QImageIOPlugin::Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

QImageIOPlugin::Capabilities QWbmpPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "wbmp")
        return Capabilities(CanRead | CanWrite);

    Capabilities cap;
    if (!format.isEmpty())
        return cap;

    if (!device->isOpen())
        return cap;

    if (device->isReadable() && QWbmpHandler::canRead(device))
        cap |= CanRead;

    if (device->isWritable())
        cap |= CanWrite;

    return cap;
}

QImageIOHandler * QWbmpPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new QWbmpHandler(device);

    handler->setFormat(format);
    return handler;
}

QT_END_NAMESPACE

#include "main.moc"

#endif /* QT_NO_IMAGEFORMATPLUGIN */
