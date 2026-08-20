// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSERVICEINFO_P_H
#define QBLUETOOTHSERVICEINFO_P_H

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

#include "qbluetoothuuid.h"
#include "qbluetoothaddress.h"
#include "qbluetoothdeviceinfo.h"
#include "qbluetoothserviceinfo.h"

#include <QMap>
#include <QVariant>

#ifdef Q_OS_MACOS
#include "darwin/btraii_p.h"
#endif

namespace QtBluetoothPrivate {

class OrgBluezServiceInterface;
class OrgBluezProfileManager1Interface;

} // namespace QtBluetoothPrivate

#ifdef QT_WINRT_BLUETOOTH
#include <wrl.h>

namespace ABI {
    namespace Windows {
        namespace Devices {
            namespace Bluetooth {
                namespace Rfcomm {
                    struct IRfcommServiceProvider;
                }
            }
        }
    }
}
#endif

QT_BEGIN_NAMESPACE

class QBluetoothServiceInfo;


class QBluetoothServiceInfoPrivate
    : public QObject
{
    Q_OBJECT
public:
    QBluetoothServiceInfoPrivate();
    ~QBluetoothServiceInfoPrivate();

    bool registerService(const QBluetoothAddress &localAdapter = QBluetoothAddress());

    bool isRegistered() const;

    bool unregisterService();

    QBluetoothDeviceInfo deviceInfo;
    QMap<quint16, QVariant> attributes;

    QBluetoothServiceInfo::Sequence protocolDescriptor(QBluetoothUuid::ProtocolUuid protocol) const;
    int serverChannel() const;
private:
#if QT_CONFIG(bluez)
    QtBluetoothPrivate::OrgBluezProfileManager1Interface *service = nullptr;
    quint32 serviceRecord;
    QBluetoothAddress currentLocalAdapter;
    QString profilePath;
#endif

#ifdef QT_WINRT_BLUETOOTH
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::Rfcomm::IRfcommServiceProvider> serviceProvider;

    bool writeSdpAttributes();
#endif

#if QT_OSX_BLUETOOTH
public:
    bool registerService(const QBluetoothServiceInfo &info);

private:

    using SDPRecord = DarwinBluetooth::ScopedPointer;
    SDPRecord serviceRecord;
    quint32 serviceRecordHandle = 0;
#endif // QT_OSX_BLUETOOTH

    mutable bool registered = false;
};

QT_END_NAMESPACE

#endif // QBLUETOOTHSERVICEINFO_P_H
