// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QT_NO_IMAGEFORMATPLUGIN

#include <qmacheifhandler.h>

QT_BEGIN_NAMESPACE

class QMacHeifPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "macheif.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

QImageIOPlugin::Capabilities QMacHeifPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    static const Capabilities sysCaps = QIIOFHelpers::systemCapabilities(QStringLiteral("public.heic"));

    Capabilities cap;
    if (!sysCaps)
        return cap;
    if (format == "heic" || format == "heif")
        return sysCaps;
    if (!format.isEmpty())
        return cap;
    if (!device->isOpen())
        return cap;

    if (sysCaps.testFlag(CanRead) && device->isReadable() && QMacHeifHandler::canRead(device))
        cap |= CanRead;
    if (sysCaps.testFlag(CanWrite) && device->isWritable())
        cap |= CanWrite;
    return cap;
}

QImageIOHandler *QMacHeifPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QMacHeifHandler *handler = new QMacHeifHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

QT_END_NAMESPACE

#include "main.moc"

#endif // !QT_NO_IMAGEFORMATPLUGIN
