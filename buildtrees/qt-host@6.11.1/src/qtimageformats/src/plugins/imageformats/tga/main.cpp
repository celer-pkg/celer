// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGui/QImageIOHandler>
#include <QtCore/QDebug>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEFORMAT_TGA
#undef QT_NO_IMAGEFORMAT_TGA
#endif

#include "qtgahandler.h"

QT_BEGIN_NAMESPACE

class QTgaPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "tga.json")

public:
    Capabilities capabilities(QIODevice * device, const QByteArray & format) const override;
    QImageIOHandler * create(QIODevice * device, const QByteArray & format = QByteArray()) const override;
};

QImageIOPlugin::Capabilities QTgaPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "tga")
        return Capabilities(CanRead);
    Capabilities cap;
    if (!format.isEmpty())
        return cap;
    if (!device->isOpen())
        return cap;

    if (device->isReadable() && QTgaHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler* QTgaPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *tgaHandler = new QTgaHandler();
    tgaHandler->setDevice(device);
    tgaHandler->setFormat(format);
    return tgaHandler;
}

QT_END_NAMESPACE

#include "main.moc"

#endif /* QT_NO_IMAGEFORMATPLUGIN */
