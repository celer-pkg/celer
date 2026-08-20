// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <qimageiohandler.h>
#include <qstringlist.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEFORMAT_MNG
#undef QT_NO_IMAGEFORMAT_MNG
#endif
#include "qmnghandler_p.h"

#include <qiodevice.h>
#include <qbytearray.h>

QT_BEGIN_NAMESPACE

class QMngPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "mng.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

QImageIOPlugin::Capabilities QMngPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "mng")
        return Capabilities(CanRead);
    Capabilities cap;
    if (!format.isEmpty())
        return cap;
    if (!device->isOpen())
        return cap;

    if (device->isReadable() && QMngHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *QMngPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QMngHandler *hand = new QMngHandler();
    hand->setDevice(device);
    hand->setFormat(format);
    return hand;
}

QT_END_NAMESPACE

#include "main.moc"

#endif // !QT_NO_IMAGEFORMATPLUGIN
