// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSERVICEDISCOVERYAGENT_P_H
#define QBLUETOOTHSERVICEDISCOVERYAGENT_P_H

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

#include "qbluetoothaddress.h"
#include "qbluetoothdeviceinfo.h"
#include "qbluetoothserviceinfo.h"
#include "qbluetoothservicediscoveryagent.h"

#include <QStack>
#include <QStringList>

#if QT_CONFIG(bluez)

namespace QtBluetoothPrivate {

class OrgBluezManagerInterface;
class OrgBluezAdapterInterface;
class OrgBluezDeviceInterface;
class OrgFreedesktopDBusObjectManagerInterface;

} // namespace QtBluetoothPrivate

#include <QtCore/qprocess.h>

QT_BEGIN_NAMESPACE
class QDBusPendingCallWatcher;
class QXmlStreamReader;
QT_END_NAMESPACE
#endif

#ifdef QT_WINRT_BLUETOOTH
#include <QtCore/QPointer>
#endif

#ifdef QT_OSX_BLUETOOTH
#include "darwin/btdelegates_p.h"
#include "darwin/btraii_p.h"
#endif

#ifdef QT_ANDROID_BLUETOOTH
#include <QtCore/QJniObject>
#include <QtBluetooth/QBluetoothLocalDevice>
#endif

// intentionally outside of Qt namespace
class tst_QBluetoothServiceDiscoveryAgent;

QT_BEGIN_NAMESPACE

class QBluetoothDeviceDiscoveryAgent;
#ifdef QT_ANDROID_BLUETOOTH
class ServiceDiscoveryBroadcastReceiver;
class LocalDeviceBroadcastReceiver;
#endif

#ifdef QT_WINRT_BLUETOOTH
class QWinRTBluetoothServiceDiscoveryWorker;
#endif

class Q_AUTOTEST_EXPORT QBluetoothServiceDiscoveryAgentPrivate
#if defined(QT_WINRT_BLUETOOTH)
        : public QObject
{
    Q_OBJECT
#elif defined(QT_OSX_BLUETOOTH)
        : public QObject, public DarwinBluetooth::SDPInquiryDelegate
{
#else
{
#endif
    Q_DECLARE_PUBLIC(QBluetoothServiceDiscoveryAgent)
    friend class ::tst_QBluetoothServiceDiscoveryAgent;

public:
    // Best-guess value. Should be enough for all real usecases.
    static constexpr int kMaxSdpRecursionDepth = 64;

    enum DiscoveryState {
        Inactive,
        DeviceDiscovery,
        ServiceDiscovery,
    };

    QBluetoothServiceDiscoveryAgentPrivate(QBluetoothServiceDiscoveryAgent *qp,
                                           const QBluetoothAddress &deviceAdapter);
    ~QBluetoothServiceDiscoveryAgentPrivate();

    void startDeviceDiscovery();
    void stopDeviceDiscovery();
    void startServiceDiscovery();
    void stopServiceDiscovery();

    void setDiscoveryState(DiscoveryState s) { state = s; }
    inline DiscoveryState discoveryState() { return state; }

    void setDiscoveryMode(QBluetoothServiceDiscoveryAgent::DiscoveryMode m) { mode = m; }
    QBluetoothServiceDiscoveryAgent::DiscoveryMode DiscoveryMode() { return mode; }

    void _q_deviceDiscoveryFinished();
    void _q_deviceDiscovered(const QBluetoothDeviceInfo &info);
    void _q_serviceDiscoveryFinished();
    void _q_deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error);
#if QT_CONFIG(bluez)
    void _q_sdpScannerDone(int exitCode, QProcess::ExitStatus status);
    void _q_finishSdpScan(QBluetoothServiceDiscoveryAgent::Error errorCode,
                          const QString &errorDescription,
                          const QStringList &xmlRecords);
#endif
#ifdef QT_ANDROID_BLUETOOTH
    void _q_processFetchedUuids(const QBluetoothAddress &address, const QList<QBluetoothUuid> &uuids);

    void populateDiscoveredServices(const QBluetoothDeviceInfo &remoteDevice,
                                    const QList<QBluetoothUuid> &uuids);
    void _q_fetchUuidsTimeout();
    void _q_hostModeStateChanged(QBluetoothLocalDevice::HostMode state);
#endif

private:
    void start(const QBluetoothAddress &address);
    void stop();
    bool isDuplicatedService(const QBluetoothServiceInfo &serviceInfo) const;

#if QT_CONFIG(bluez)
    void startBluez5(const QBluetoothAddress &address);
    void runExternalSdpScan(const QBluetoothAddress &remoteAddress,
                    const QBluetoothAddress &localAddress);
    void sdpScannerDone(int exitCode, QProcess::ExitStatus exitStatus);
    QVariant readAttributeValue(QXmlStreamReader &xml, int depth = 0);
    QBluetoothServiceInfo parseServiceXml(const QString& xml);
    void performMinimalServiceDiscovery(const QBluetoothAddress &deviceAddress);
#endif

public:
    QBluetoothServiceDiscoveryAgent::Error error;
    QString errorString;
    QBluetoothAddress deviceAddress;
    QList<QBluetoothServiceInfo> discoveredServices;
    QList<QBluetoothDeviceInfo> discoveredDevices;
    QBluetoothAddress m_deviceAdapterAddress;

private:
    DiscoveryState state;
    QList<QBluetoothUuid> uuidFilter;

    QBluetoothDeviceDiscoveryAgent *deviceDiscoveryAgent = nullptr;

    QBluetoothServiceDiscoveryAgent::DiscoveryMode mode;

    bool singleDevice;
#if QT_CONFIG(bluez)
    QString foundHostAdapterPath;
    QtBluetoothPrivate::OrgFreedesktopDBusObjectManagerInterface *manager = nullptr;
    QProcess *sdpScannerProcess = nullptr;
#endif

#ifdef QT_ANDROID_BLUETOOTH
    ServiceDiscoveryBroadcastReceiver *receiver = nullptr;
    LocalDeviceBroadcastReceiver *localDeviceReceiver = nullptr;

    QJniObject btAdapter;
    // The sdpCache caches service discovery results while it is running, and is
    // cleared once finished. The cache is used as we may (or may not) get more accurate
    // results after the first result. This temporary caching allows to send the
    // serviceDiscovered() signal once per service and with the most accurate information.
    // Partial cache clearing may occur already during the scan if the second (more accurate)
    // scan result is received.
    QMap<QBluetoothAddress,QPair<QBluetoothDeviceInfo,QList<QBluetoothUuid> > > sdpCache;
#endif

#ifdef QT_WINRT_BLUETOOTH
private slots:
    void processFoundService(quint64 deviceAddress, const QBluetoothServiceInfo &info);
    void onScanFinished(quint64 deviceAddress);
    void onError();

private:
    void releaseWorker();
    QPointer<QWinRTBluetoothServiceDiscoveryWorker> worker;
#endif

#ifdef QT_OSX_BLUETOOTH
    // SDPInquiryDelegate:
    void SDPInquiryFinished(void *device) override;
    void SDPInquiryError(void *device, IOReturn errorCode) override;

    void performMinimalServiceDiscovery(const QBluetoothAddress &deviceAddress);
    //void serviceDiscoveryFinished();

    bool serviceHasMatchingUuid(const QBluetoothServiceInfo &serviceInfo) const;

    DarwinBluetooth::ScopedPointer serviceInquiry;
#endif // QT_OSX_BLUETOOTH

protected:
    QBluetoothServiceDiscoveryAgent *q_ptr;
};

QT_END_NAMESPACE

#endif
