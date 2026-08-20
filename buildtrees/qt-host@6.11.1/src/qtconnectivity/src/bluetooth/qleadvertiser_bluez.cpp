// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:trusted-data

#include "qleadvertiser_bluez_p.h"

#include "bluez/bluez_data_p.h"
#include "bluez/hcimanager_p.h"
#include "qbluetoothsocketbase_p.h"

#include <QtCore/qloggingcategory.h>

#include <cstring>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

QLeAdvertiser::~QLeAdvertiser()
    = default;

struct AdvParams {
    quint16 minInterval;
    quint16 maxInterval;
    quint8 type;
    quint8 ownAddrType;
    quint8 directAddrType;
    bdaddr_t directAddr;
    quint8 channelMap;
    quint8 filterPolicy;
} __attribute__ ((packed));

struct AdvData {
    quint8 length;
    quint8 data[31];
};

struct WhiteListParams {
    quint8 addrType;
    bdaddr_t addr;
};


template <typename T>
static QByteArray byteArrayFromStruct(const T &data)
{
    return QByteArray(reinterpret_cast<const char *>(&data), sizeof data);
}

QLeAdvertiserBluez::QLeAdvertiserBluez(const QLowEnergyAdvertisingParameters &params,
                                       const QLowEnergyAdvertisingData &advertisingData,
                                       const QLowEnergyAdvertisingData &scanResponseData,
                                       std::shared_ptr<HciManager> hciManager, QObject *parent)
    : QLeAdvertiser(params, advertisingData, scanResponseData, parent), m_hciManager(hciManager)
{
    Q_ASSERT(m_hciManager);
    connect(m_hciManager.get(), &HciManager::commandCompleted, this,
            &QLeAdvertiserBluez::handleCommandCompleted);
}

QLeAdvertiserBluez::~QLeAdvertiserBluez()
{
    disconnect(m_hciManager.get(), &HciManager::commandCompleted, this,
               &QLeAdvertiserBluez::handleCommandCompleted);
    doStopAdvertising();
}

void QLeAdvertiserBluez::doStartAdvertising()
{
    if (!m_hciManager->monitorEvent(HciManager::HciEvent::EVT_CMD_COMPLETE)) {
        handleError();
        return;
    }

    m_sendPowerLevel = advertisingData().includePowerLevel()
            || scanResponseData().includePowerLevel();
    if (m_sendPowerLevel)
        queueReadTxPowerLevelCommand();
    else
        queueAdvertisingCommands();
    sendNextCommand();
}

void QLeAdvertiserBluez::doStopAdvertising()
{
    toggleAdvertising(false);
    sendNextCommand();
}

void QLeAdvertiserBluez::queueCommand(QBluezConst::OpCodeCommandField ocf, const QByteArray &data)
{
    m_pendingCommands << Command(ocf, data);
}

void QLeAdvertiserBluez::sendNextCommand()
{
    if (m_pendingCommands.isEmpty()) {
        // TODO: Unmonitor event.
        return;
    }
    const Command &c = m_pendingCommands.first();
    if (!m_hciManager->sendCommand(QBluezConst::OgfLinkControl, c.ocf, c.data)) {
        handleError();
        return;
    }
}

void QLeAdvertiserBluez::queueAdvertisingCommands()
{
    toggleAdvertising(false); // Stop advertising first, in case it's currently active.
    setWhiteList();
    setAdvertisingParams();
    setAdvertisingData();
    setScanResponseData();
    toggleAdvertising(true);
}

void QLeAdvertiserBluez::queueReadTxPowerLevelCommand()
{
    // Spec v4.2, Vol 2, Part E, 7.8.6
    queueCommand(QBluezConst::OcfLeReadTxPowerLevel, QByteArray());
}

void QLeAdvertiserBluez::toggleAdvertising(bool enable)
{
    // Spec v4.2, Vol 2, Part E, 7.8.9
    queueCommand(QBluezConst::OcfLeSetAdvEnable, QByteArray(1, enable));
}

void QLeAdvertiserBluez::setAdvertisingParams()
{
    // Spec v4.2, Vol 2, Part E, 7.8.5
    // or Spec v5.3, Vol 4, Part E, 7.8.5
    AdvParams params;
    static_assert(sizeof params == 15, "unexpected struct size");
    using namespace std;
    memset(&params, 0, sizeof params);
    setAdvertisingInterval(params);
    params.type = parameters().mode();
    params.filterPolicy = parameters().filterPolicy();
    if (params.filterPolicy != QLowEnergyAdvertisingParameters::IgnoreWhiteList
            && advertisingData().discoverability() == QLowEnergyAdvertisingData::DiscoverabilityLimited) {
        qCWarning(QT_BT_BLUEZ) << "limited discoverability is incompatible with "
                                  "using a white list; disabling filtering";
        params.filterPolicy = QLowEnergyAdvertisingParameters::IgnoreWhiteList;
    }
    params.ownAddrType = QLowEnergyController::PublicAddress; // TODO: Make configurable.

    // TODO: For ADV_DIRECT_IND.
    // params.directAddrType = xxx;
    // params.direct_bdaddr = xxx;

    params.channelMap = 0x7; // All channels.

    const QByteArray paramsData = byteArrayFromStruct(params);
    qCDebug(QT_BT_BLUEZ) << "advertising parameters:" << paramsData.toHex();
    queueCommand(QBluezConst::OcfLeSetAdvParams, paramsData);
}

static quint16 forceIntoRange(quint16 val, quint16 min, quint16 max)
{
    return qMin(qMax(val, min), max);
}

void QLeAdvertiserBluez::setAdvertisingInterval(AdvParams &params)
{
    const double multiplier = 0.625;
    const quint16 minVal = parameters().minimumInterval() / multiplier;
    const quint16 maxVal = parameters().maximumInterval() / multiplier;
    Q_ASSERT(minVal <= maxVal);
    const quint16 specMinimum =
            parameters().mode() == QLowEnergyAdvertisingParameters::AdvScanInd
            || parameters().mode() == QLowEnergyAdvertisingParameters::AdvNonConnInd ? 0xa0 : 0x20;
    const quint16 specMaximum = 0x4000;
    params.minInterval = qToLittleEndian(forceIntoRange(minVal, specMinimum, specMaximum));
    params.maxInterval = qToLittleEndian(forceIntoRange(maxVal, specMinimum, specMaximum));
    Q_ASSERT(params.minInterval <= params.maxInterval);
}

void QLeAdvertiserBluez::setPowerLevel(AdvData &advData)
{
    if (m_sendPowerLevel) {
        advData.data[advData.length++] = 2;
        advData.data[advData.length++]= 0xa;
        advData.data[advData.length++] = m_powerLevel;
    }
}

void QLeAdvertiserBluez::setFlags(AdvData &advData)
{
    // TODO: Discoverability flags are incompatible with ADV_DIRECT_IND
    quint8 flags = 0;
    if (advertisingData().discoverability() == QLowEnergyAdvertisingData::DiscoverabilityLimited)
        flags |= 0x1;
    else if (advertisingData().discoverability() == QLowEnergyAdvertisingData::DiscoverabilityGeneral)
        flags |= 0x2;
    flags |= 0x4; // "BR/EDR not supported". Otherwise clients might try to connect over Bluetooth classic.
    if (flags) {
        advData.data[advData.length++] = 2;
        advData.data[advData.length++] = 0x1;
        advData.data[advData.length++] = flags;
    }
}

template<typename T> static quint8 servicesType(bool dataComplete);
template<> quint8 servicesType<quint16>(bool dataComplete)
{
    return dataComplete ? 0x3 : 0x2;
}
template<> quint8 servicesType<quint32>(bool dataComplete)
{
    return dataComplete ? 0x5 : 0x4;
}
template<> quint8 servicesType<QUuid::Id128Bytes>(bool dataComplete)
{
    return dataComplete ? 0x7 : 0x6;
}

template<typename T>
static void addServicesData(AdvData &data, const QList<T> &services)
{
    if (services.isEmpty())
        return;
    constexpr auto sizeofT = static_cast<int>(sizeof(T)); // signed is more convenient
    const qsizetype spaceAvailable = sizeof data.data - data.length;
    // Determine how many services will be set, space may limit the number
    const qsizetype maxServices = (std::min)((spaceAvailable - 2) / sizeofT, services.size());
    if (maxServices <= 0) {
        qCWarning(QT_BT_BLUEZ) << "services data does not fit into advertising data packet";
        return;
    }
    const bool dataComplete = maxServices == services.size();
    if (!dataComplete) {
        qCWarning(QT_BT_BLUEZ) << "only" << maxServices << "out of" << services.size()
                               << "services fit into the advertising data";
    }
    data.data[data.length++] = 1 + maxServices * sizeofT;
    data.data[data.length++] = servicesType<T>(dataComplete);
    for (qsizetype i = 0; i < maxServices; ++i) {
        memcpy(data.data + data.length, &services.at(i), sizeofT);
        data.length += sizeofT;
    }
}

void QLeAdvertiserBluez::setServicesData(const QLowEnergyAdvertisingData &src, AdvData &dest)
{
    QList<quint16> services16;
    QList<quint32> services32;
    QList<QUuid::Id128Bytes> services128;
    const QList<QBluetoothUuid> services = src.services();
    for (const QBluetoothUuid &service : services) {
        bool ok;
        const quint16 service16 = service.toUInt16(&ok);
        if (ok) {
            services16 << qToLittleEndian(service16);
            continue;
        }
        const quint32 service32 = service.toUInt32(&ok);
        if (ok) {
            services32 << qToLittleEndian(service32);
            continue;
        }

        // QUuid::toBytes() is defaults to Big-Endian
        services128 << service.toBytes(QSysInfo::LittleEndian);
    }
    addServicesData(dest, services16);
    addServicesData(dest, services32);
    addServicesData(dest, services128);
}

void QLeAdvertiserBluez::setManufacturerData(const QLowEnergyAdvertisingData &src, AdvData &dest)
{
    if (src.manufacturerId() == QLowEnergyAdvertisingData::invalidManufacturerId())
        return;

    const QByteArray manufacturerData = src.manufacturerData();
    if (qsizetype(sizeof dest.data) - dest.length < 1 + 1 + 2 + manufacturerData.size()) {
        qCWarning(QT_BT_BLUEZ) << "manufacturer data does not fit into advertising data packet";
        return;
    }

    dest.data[dest.length++] = manufacturerData.size() + 1 + 2;
    dest.data[dest.length++] = 0xff;
    putBtData(src.manufacturerId(), dest.data + dest.length);
    dest.length += sizeof(quint16);
    std::memcpy(dest.data + dest.length, manufacturerData.data(), manufacturerData.size());
    dest.length += manufacturerData.size();
}

void QLeAdvertiserBluez::setLocalNameData(const QLowEnergyAdvertisingData &src, AdvData &dest)
{
    if (src.localName().isEmpty())
        return;
    if (dest.length >= sizeof dest.data - 3) {
        qCWarning(QT_BT_BLUEZ) << "local name does not fit into advertising data";
        return;
    }

    const QByteArray localNameUtf8 = src.localName().toUtf8();
    const qsizetype fullSize = localNameUtf8.size() + 1 + 1;
    const qsizetype size = (std::min)(fullSize, qsizetype(sizeof dest.data - dest.length));
    const bool isComplete = size == fullSize;
    dest.data[dest.length++] = size - 1;
    const int dataType = isComplete ? 0x9 : 0x8;
    dest.data[dest.length++] = dataType;
    std::memcpy(dest.data + dest.length, localNameUtf8, size - 2);
    dest.length += size - 2;
}

void QLeAdvertiserBluez::setData(bool isScanResponseData)
{
    // Spec v4.2, Vol 3, Part C, 11 and Supplement, Part 1
    AdvData theData;
    static_assert(sizeof theData == 32, "unexpected struct size");
    theData.length = 0;

    const QLowEnergyAdvertisingData &sourceData = isScanResponseData
            ? scanResponseData() : advertisingData();

    if (const QByteArray rawData = sourceData.rawData(); !rawData.isEmpty()) {
        theData.length = (std::min)(qsizetype(sizeof theData.data), rawData.size());
        std::memcpy(theData.data, rawData.data(), theData.length);
    } else {
        if (sourceData.includePowerLevel())
            setPowerLevel(theData);
        if (!isScanResponseData)
            setFlags(theData);

        // Insert new constant-length data here.

        setLocalNameData(sourceData, theData);
        setServicesData(sourceData, theData);
        setManufacturerData(sourceData, theData);
    }

    std::memset(theData.data + theData.length, 0, sizeof theData.data - theData.length);
    const QByteArray dataToSend = byteArrayFromStruct(theData);

    if (!isScanResponseData) {
        qCDebug(QT_BT_BLUEZ) << "advertising data:" << dataToSend.toHex();
        queueCommand(QBluezConst::OcfLeSetAdvData, dataToSend);
    } else if ((parameters().mode() == QLowEnergyAdvertisingParameters::AdvScanInd
               || parameters().mode() == QLowEnergyAdvertisingParameters::AdvInd)
               && theData.length > 0) {
        qCDebug(QT_BT_BLUEZ) << "scan response data:" << dataToSend.toHex();
        queueCommand(QBluezConst::OcfLeSetScanResponseData, dataToSend);
    }
}

void QLeAdvertiserBluez::setAdvertisingData()
{
    // Spec v4.2, Vol 2, Part E, 7.8.7
    setData(false);
}

void QLeAdvertiserBluez::setScanResponseData()
{
    // Spec v4.2, Vol 2, Part E, 7.8.8
    setData(true);
}

void QLeAdvertiserBluez::setWhiteList()
{
    // Spec v4.2, Vol 2, Part E, 7.8.15-16
    if (parameters().filterPolicy() == QLowEnergyAdvertisingParameters::IgnoreWhiteList)
        return;
    queueCommand(QBluezConst::OcfLeClearWhiteList, QByteArray());
    const QList<QLowEnergyAdvertisingParameters::AddressInfo> whiteListInfos
            = parameters().whiteList();
    for (const auto &addressInfo : whiteListInfos) {
        WhiteListParams commandParam;
        static_assert(sizeof commandParam == 7, "unexpected struct size");
        commandParam.addrType = addressInfo.type;
        convertAddress(addressInfo.address.toUInt64(), commandParam.addr.b);
        queueCommand(QBluezConst::OcfLeAddToWhiteList, byteArrayFromStruct(commandParam));
    }
}

void QLeAdvertiserBluez::handleCommandCompleted(quint16 opCode, quint8 status,
                                                const QByteArray &data)
{
    if (m_pendingCommands.isEmpty())
        return;
    const QBluezConst::OpCodeCommandField ocf = QBluezConst::OpCodeCommandField(ocfFromOpCode(opCode));
    const Command currentCmd = m_pendingCommands.first();
    if (currentCmd.ocf != ocf)
        return; // Not one of our commands.
    m_pendingCommands.takeFirst();
    if (status != 0) {
        qCDebug(QT_BT_BLUEZ) << "command" << ocf
                             << "failed with status" << (HciManager::HciError)status
                             << "status code" << status;
        if (ocf == QBluezConst::OcfLeSetAdvEnable && status == 0xc && currentCmd.data == QByteArray(1, '\0')) {
            // we ignore OcfLeSetAdvEnable if it tries to disable an active advertisement
            // it seems the platform often automatically turns off advertisements
            // subsequently the explicit stopAdvertisement call fails when re-issued
            qCDebug(QT_BT_BLUEZ) << "Advertising disable failed, ignoring";
            sendNextCommand();
            return;
        }
        if (ocf == QBluezConst::OcfLeReadTxPowerLevel) {
            qCDebug(QT_BT_BLUEZ) << "reading power level failed, leaving it out of the "
                                    "advertising data";
            m_sendPowerLevel = false;
        } else {
            handleError();
            return;
        }
    } else {
        qCDebug(QT_BT_BLUEZ) << "command" << ocf << "executed successfully";
    }

    switch (ocf) {
    case QBluezConst::OcfLeReadTxPowerLevel:
        if (m_sendPowerLevel) {
            if (!data.isEmpty())
                m_powerLevel = data.at(0);
            else
                m_powerLevel = 0;
            qCDebug(QT_BT_BLUEZ) << "TX power level is" << m_powerLevel;
        }
        queueAdvertisingCommands();
        break;
    default:
        break;
    }

    sendNextCommand();
}

void QLeAdvertiserBluez::handleError()
{
    m_pendingCommands.clear();
    // TODO: Unmonitor event
    emit errorOccurred();
}

QT_END_NAMESPACE

#include "moc_qleadvertiser_bluez_p.cpp"
