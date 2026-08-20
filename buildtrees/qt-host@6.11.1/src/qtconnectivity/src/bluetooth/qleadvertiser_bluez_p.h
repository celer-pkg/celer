// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLEADVERTISER_BLUEZ_P_H
#define QLEADVERTISER_BLUEZ_P_H

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

#include "bluez/bluez_data_p.h"

#include <QtCore/qlist.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QLeAdvertiser : public QObject
{
    Q_OBJECT
public:
    void startAdvertising() { doStartAdvertising(); }
    void stopAdvertising() { doStopAdvertising(); }

signals:
    void errorOccurred();

public:
    QLeAdvertiser(const QLowEnergyAdvertisingParameters &params,
                  const QLowEnergyAdvertisingData &advData,
                  const QLowEnergyAdvertisingData &responseData, QObject *parent)
        : QObject(parent), m_params(params), m_advData(advData), m_responseData(responseData) {}
    ~QLeAdvertiser() override;

protected:
    const QLowEnergyAdvertisingParameters &parameters() const { return m_params; }
    const QLowEnergyAdvertisingData &advertisingData() const { return m_advData; }
    const QLowEnergyAdvertisingData &scanResponseData() const { return m_responseData; }

private:
    virtual void doStartAdvertising() = 0;
    virtual void doStopAdvertising() = 0;

    const QLowEnergyAdvertisingParameters m_params;
    const QLowEnergyAdvertisingData m_advData;
    const QLowEnergyAdvertisingData m_responseData;
};

struct AdvData;
struct AdvParams;
class HciManager;

class QLeAdvertiserBluez : public QLeAdvertiser
{
    Q_OBJECT
public:
    QLeAdvertiserBluez(const QLowEnergyAdvertisingParameters &params,
                       const QLowEnergyAdvertisingData &advertisingData,
                       const QLowEnergyAdvertisingData &scanResponseData,
                       std::shared_ptr<HciManager> hciManager,
                       QObject *parent = nullptr);
    ~QLeAdvertiserBluez() override;

private:
    void doStartAdvertising() override;
    void doStopAdvertising() override;

    void setPowerLevel(AdvData &advData);
    void setFlags(AdvData &advData);
    void setServicesData(const QLowEnergyAdvertisingData &src, AdvData &dest);
    void setManufacturerData(const QLowEnergyAdvertisingData &src, AdvData &dest);
    void setLocalNameData(const QLowEnergyAdvertisingData &src, AdvData &dest);

    void queueCommand(QBluezConst::OpCodeCommandField ocf, const QByteArray &advertisingData);
    void sendNextCommand();
    void queueAdvertisingCommands();
    void queueReadTxPowerLevelCommand();
    void toggleAdvertising(bool enable);
    void setAdvertisingParams();
    void setAdvertisingInterval(AdvParams &params);
    void setData(bool isScanResponseData);
    void setAdvertisingData();
    void setScanResponseData();
    void setWhiteList();

    void handleCommandCompleted(quint16 opCode, quint8 status, const QByteArray &advertisingData);
    void handleError();

    std::shared_ptr<HciManager> m_hciManager;

    struct Command {
        Command() {}
        Command(QBluezConst::OpCodeCommandField ocf, const QByteArray &data) : ocf(ocf), data(data) { }
        QBluezConst::OpCodeCommandField ocf;
        QByteArray data;
    };
    QList<Command> m_pendingCommands;

    quint8 m_powerLevel;
    bool m_sendPowerLevel;
};

QT_END_NAMESPACE

#endif // Include guard.
