// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLEADVERTISER_BLUEZDBUS_P_H
#define QLEADVERTISER_BLUEZDBUS_P_H

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

#include "qlowenergyadvertisingdata.h"
#include "qlowenergyadvertisingparameters.h"

QT_REQUIRE_CONFIG(bluez);

#include <QtCore/QObject>

namespace QtBluetoothPrivate {

class OrgBluezLEAdvertisement1Adaptor;
class OrgBluezLEAdvertisingManager1Interface;

} // namespace QtBluetoothPrivate

QT_BEGIN_NAMESPACE

class QLeDBusAdvertiser : public QObject
{
    Q_OBJECT

public:
    QLeDBusAdvertiser(const QLowEnergyAdvertisingParameters &params,
                      const QLowEnergyAdvertisingData &advertisingData,
                      const QLowEnergyAdvertisingData &scanResponseData,
                      const QString &hostAdapterPath,
                      QObject* parent = nullptr);
    ~QLeDBusAdvertiser() override;

    void startAdvertising();
    void stopAdvertising();

    Q_INVOKABLE void Release();

signals:
    void errorOccurred();

private:
    void setDataForDBus();
    void setAdvertisingParamsForDBus();
    void setAdvertisementDataForDBus();

private:
    const QLowEnergyAdvertisingParameters m_advParams;
    QLowEnergyAdvertisingData m_advData;
    const QString m_advObjectPath;
    QtBluetoothPrivate::OrgBluezLEAdvertisement1Adaptor* const m_advDataDBus;
    QtBluetoothPrivate::OrgBluezLEAdvertisingManager1Interface* const m_advManager;
    bool m_advertising = false;
};

QT_END_NAMESPACE

#endif // QLEADVERTISER_BLUEZDBUS_P_H
