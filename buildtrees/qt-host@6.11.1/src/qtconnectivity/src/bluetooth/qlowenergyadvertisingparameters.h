// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYADVERTISINGPARAMETERS_H
#define QLOWENERGYADVERTISINGPARAMETERS_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtCore/qlist.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QLowEnergyAdvertisingParametersPrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyAdvertisingParameters
{
public:
    QLowEnergyAdvertisingParameters();
    QLowEnergyAdvertisingParameters(const QLowEnergyAdvertisingParameters &other);
    ~QLowEnergyAdvertisingParameters();

    QLowEnergyAdvertisingParameters &operator=(const QLowEnergyAdvertisingParameters &other);
    friend bool operator==(const QLowEnergyAdvertisingParameters &a,
                           const QLowEnergyAdvertisingParameters &b)
    {
        return equals(a, b);
    }

    friend bool operator!=(const QLowEnergyAdvertisingParameters &a,
                           const QLowEnergyAdvertisingParameters &b)
    {
        return !equals(a, b);
    }

    enum Mode { AdvInd = 0x0, AdvScanInd = 0x2, AdvNonConnInd = 0x3 };
    void setMode(Mode mode);
    Mode mode() const;

    class Q_BLUETOOTH_EXPORT AddressInfo
    {
    public:
        AddressInfo(const QBluetoothAddress &addr, QLowEnergyController::RemoteAddressType t)
            : address(addr), type(t) {}
        AddressInfo() : type(QLowEnergyController::PublicAddress) {}

        QBluetoothAddress address;
        QLowEnergyController::RemoteAddressType type;
        friend bool operator==(const AddressInfo &a, const AddressInfo &b) { return equals(a, b); }
        friend bool operator!=(const AddressInfo &a, const AddressInfo &b) { return !equals(a, b); }

    private:
        static bool equals(const AddressInfo &a, const AddressInfo &b);
    };

    enum FilterPolicy {
        IgnoreWhiteList = 0x00,
        UseWhiteListForScanning = 0x01,
        UseWhiteListForConnecting = 0x02,
        UseWhiteListForScanningAndConnecting = 0x03,
    };
    void setWhiteList(const QList<AddressInfo> &whiteList, FilterPolicy policy);
    QList<AddressInfo> whiteList() const;
    FilterPolicy filterPolicy() const;

    void setInterval(quint16 minimum, quint16 maximum);
    int minimumInterval() const;
    int maximumInterval() const;

    // TODO: own address type
    // TODO: For ADV_DIRECT_IND: peer address + peer address type

    void swap(QLowEnergyAdvertisingParameters &other) noexcept { d.swap(other.d); }

private:
    static bool equals(const QLowEnergyAdvertisingParameters &a,
                       const QLowEnergyAdvertisingParameters &b);
    QSharedDataPointer<QLowEnergyAdvertisingParametersPrivate> d;
};

Q_DECLARE_SHARED(QLowEnergyAdvertisingParameters)

QT_END_NAMESPACE

#endif // Include guard
