// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <qimageiohandler.h>
#include <qstringlist.h>

#if !defined(QT_NO_SVGRENDERER)

#include "qsvgiohandler.h"

#include <qiodevice.h>
#include <qbytearray.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

class QSvgPlugin : public QImageIOPlugin
{
    Q_OBJECT
#ifndef QT_NO_COMPRESS
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "svg.json")
#else
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "svg-nocompress.json")
#endif

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

QImageIOPlugin::Capabilities QSvgPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
#ifndef QT_NO_COMPRESS
    if (format == "svg" || format == "svgz")
#else
    if (format == "svg")
#endif
        return Capabilities(CanRead);
    Capabilities cap;
    if (!format.isEmpty())
        return cap;
    if (device->isReadable() && QSvgIOHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *QSvgPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QSvgIOHandler *hand = new QSvgIOHandler();
    hand->setDevice(device);
    hand->setFormat(format);
    return hand;
}

QT_END_NAMESPACE

#include "main.moc"

#endif // !QT_NO_IMAGEFORMATPLUGIN
