// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYCONNECTIONPARAMETERS_H
#define QLOWENERGYCONNECTIONPARAMETERS_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QLowEnergyConnectionParametersPrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyConnectionParameters
{
public:
    QLowEnergyConnectionParameters();
    QLowEnergyConnectionParameters(const QLowEnergyConnectionParameters &other);
    ~QLowEnergyConnectionParameters();

    QLowEnergyConnectionParameters &operator=(const QLowEnergyConnectionParameters &other);
    friend bool operator==(const QLowEnergyConnectionParameters &a,
                           const QLowEnergyConnectionParameters &b)
    {
        return equals(a, b);
    }
    friend bool operator!=(const QLowEnergyConnectionParameters &a,
                           const QLowEnergyConnectionParameters &b)
    {
        return !equals(a, b);
    }

    void setIntervalRange(double minimum, double maximum);
    double minimumInterval() const;
    double maximumInterval() const;

    void setLatency(int latency);
    int latency() const;

    void setSupervisionTimeout(int timeout);
    int supervisionTimeout() const;

    void swap(QLowEnergyConnectionParameters &other) noexcept { d.swap(other.d); }

private:
    static bool equals(const QLowEnergyConnectionParameters &a,
                       const QLowEnergyConnectionParameters &b);
    QSharedDataPointer<QLowEnergyConnectionParametersPrivate> d;
};

Q_DECLARE_SHARED(QLowEnergyConnectionParameters)

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN(QLowEnergyConnectionParameters, Q_BLUETOOTH_EXPORT)

#endif // Include guard
