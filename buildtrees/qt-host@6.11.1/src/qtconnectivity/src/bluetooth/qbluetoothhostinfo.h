// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHHOSTINFO_H
#define QBLUETOOTHHOSTINFO_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtBluetooth/QBluetoothAddress>

QT_BEGIN_NAMESPACE

class QBluetoothHostInfoPrivate;
class Q_BLUETOOTH_EXPORT QBluetoothHostInfo
{
public:
    QBluetoothHostInfo();
    QBluetoothHostInfo(const QBluetoothHostInfo &other);
    ~QBluetoothHostInfo();

    QBluetoothHostInfo &operator=(const QBluetoothHostInfo &other);
    friend bool operator==(const QBluetoothHostInfo &a, const QBluetoothHostInfo &b)
    {
        return equals(a, b);
    }
    friend bool operator!=(const QBluetoothHostInfo &a, const QBluetoothHostInfo &b)
    {
        return !equals(a, b);
    }

    QBluetoothAddress address() const;
    void setAddress(const QBluetoothAddress &address);

    QString name() const;
    void setName(const QString &name);

private:
    static bool equals(const QBluetoothHostInfo &a, const QBluetoothHostInfo &b);
    Q_DECLARE_PRIVATE(QBluetoothHostInfo)
    QBluetoothHostInfoPrivate *d_ptr;
};

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN(QBluetoothHostInfo, Q_BLUETOOTH_EXPORT)

#endif
