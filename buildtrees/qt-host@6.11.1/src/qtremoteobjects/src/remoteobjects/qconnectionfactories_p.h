// Copyright (C) 2017-2015 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:declarations-only

#ifndef QCONNECTIONFACTORIES_P_H
#define QCONNECTIONFACTORIES_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qdatastream.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qpointer.h>
#include <QtCore/private/qobject_p.h>

#include <QtRemoteObjects/qtremoteobjectglobal.h>
#include <QtRemoteObjects/qconnectionfactories.h>
#include "qremoteobjectpacket_p.h"

#include <memory>

QT_BEGIN_NAMESPACE

namespace QtRemoteObjects {

static const int dataStreamVersion = QDataStream::Qt_6_2;
static const QLatin1String protocolVersion("QtRO 2.0");

}

class QtROExternalIoDevice : public QtROIoDeviceBase
{
    Q_OBJECT

public:
    explicit QtROExternalIoDevice(QIODevice *device, QObject *parent=nullptr);
    QIODevice *connection() const override;
    bool isOpen() const override;

protected:
    void doClose() override;
    QString deviceType() const override;
private:
    Q_DECLARE_PRIVATE(QtROExternalIoDevice)
};

class QtROIoDeviceBasePrivate : public QObjectPrivate
{
public:
    QtROIoDeviceBasePrivate();

    // TODO Remove stream()
    QDataStream &stream() { return m_dataStream; }

    bool m_isClosing = false;
    quint32 m_curReadSize = 0;
    QDataStream m_dataStream;
    QSet<QString> m_remoteObjects;
    std::unique_ptr<QRemoteObjectPackets::CodecBase> m_codec { nullptr };
    Q_DECLARE_PUBLIC(QtROIoDeviceBase)
};

class QtROClientIoDevicePrivate : public QtROIoDeviceBasePrivate
{
public:
    QtROClientIoDevicePrivate() : QtROIoDeviceBasePrivate() { }
    QUrl m_url;
    Q_DECLARE_PUBLIC(QtROClientIoDevice)
};

class QtROExternalIoDevicePrivate : public QtROIoDeviceBasePrivate
{
public:
    QtROExternalIoDevicePrivate(QIODevice *device) : QtROIoDeviceBasePrivate(), m_device(device) { }
    QPointer<QIODevice> m_device;
    Q_DECLARE_PUBLIC(QtROExternalIoDevice)
};

QT_END_NAMESPACE

#endif
