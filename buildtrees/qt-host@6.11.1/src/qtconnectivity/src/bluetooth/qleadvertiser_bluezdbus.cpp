// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qleadvertiser_bluezdbus_p.h"
#include "bluez/leadvertisement1adaptor_p.h"
#include "bluez/leadvertisingmanager1_p.h"
#include "bluez/bluez5_helper_p.h"

#include <QtCore/QtMinMax>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

using namespace Qt::StringLiterals;
using namespace QtBluetoothPrivate; // for D-Bus wrappers

// The advertisement dbus object path is freely definable, use as prefix
static constexpr auto advObjectPathTemplate{"/qt/btle/advertisement/%1%2/%3"_L1};
static constexpr auto bluezService{"org.bluez"_L1};
static constexpr auto bluezErrorFailed{"org.bluez.Error.Failed"_L1};

// From bluez API documentation
static constexpr auto advDataTXPower{"tx-power"_L1};
static constexpr auto advDataTypePeripheral{"peripheral"_L1};
static constexpr auto advDataTypeBroadcast{"broadcast"_L1};
static constexpr quint16 advDataMinIntervalMs{20};
static constexpr quint16 advDataMaxIntervalMs{10485};


QLeDBusAdvertiser::QLeDBusAdvertiser(const QLowEnergyAdvertisingParameters &params,
                                     const QLowEnergyAdvertisingData &advertisingData,
                                     const QLowEnergyAdvertisingData &scanResponseData,
                                     const QString &hostAdapterPath,
                                     QObject* parent)
    : QObject(parent),
      m_advParams(params),
      m_advData(advertisingData),
      m_advObjectPath(QString(advObjectPathTemplate).
                      arg(sanitizeNameForDBus(QCoreApplication::applicationName())).
                      arg(QCoreApplication::applicationPid()).
                      arg(QRandomGenerator::global()->generate())),
      m_advDataDBus(new OrgBluezLEAdvertisement1Adaptor(this)),
      m_advManager(new OrgBluezLEAdvertisingManager1Interface(bluezService, hostAdapterPath,
                                                              QDBusConnection::systemBus(), this))
{
    // Bluez DBus API doesn't allow distinguishing between advertisement and scan response data;
    // consolidate the two if they differ.
    // Union of service UUIDs:
    if (scanResponseData.services() != advertisingData.services()) {
        QList<QBluetoothUuid> services = advertisingData.services();
        const auto scanResponseServices = scanResponseData.services();
        for (const auto &service : scanResponseServices) {
            if (!services.contains(service))
                services.append(service);
        }
        m_advData.setServices(services);
    }
    // Scan response is given precedence with rest of the data
    if (!scanResponseData.localName().isEmpty())
        m_advData.setLocalName(scanResponseData.localName());
    if (scanResponseData.manufacturerId() != QLowEnergyAdvertisingData::invalidManufacturerId()) {
        m_advData.setManufacturerData(scanResponseData.manufacturerId(),
                                      scanResponseData.manufacturerData());
    }
    if (scanResponseData.includePowerLevel())
        m_advData.setIncludePowerLevel(true);

    setDataForDBus();
}

QLeDBusAdvertiser::~QLeDBusAdvertiser()
{
    stopAdvertising();
}

// This function parses the advertising data provided by the application and
// populates the dbus adaptor with it. DBus will ask the data from the adaptor when
// the advertisement is later registered (started)
void QLeDBusAdvertiser::setDataForDBus()
{
    setAdvertisingParamsForDBus();
    setAdvertisementDataForDBus();
}

void QLeDBusAdvertiser::setAdvertisingParamsForDBus()
{
    // Whitelist and filter policy
    if (!m_advParams.whiteList().isEmpty())
        qCWarning(QT_BT_BLUEZ) << "White lists and filter policies not supported, ignoring";

    // Legacy advertising mode mapped to GAP role (peripheral vs broadcast)
    switch (m_advParams.mode())
    {
    case QLowEnergyAdvertisingParameters::AdvScanInd:
    case QLowEnergyAdvertisingParameters::AdvNonConnInd:
        m_advDataDBus->setType(advDataTypeBroadcast);
        break;
    case QLowEnergyAdvertisingParameters::AdvInd:
    default:
        m_advDataDBus->setType(advDataTypePeripheral);
    }

    // Advertisement interval (min max in milliseconds). Ensure the values fit the range bluez
    // allows. The max >= min is guaranteed by QLowEnergyAdvertisingParameters::setInterval().
    // Note: Bluez reads these values but at the time of this writing it marks this feature
    // as 'experimental'
    m_advDataDBus->setMinInterval(qBound(advDataMinIntervalMs,
                                         quint16(m_advParams.minimumInterval()),
                                         advDataMaxIntervalMs));
    m_advDataDBus->setMaxInterval(qBound(advDataMinIntervalMs,
                                         quint16(m_advParams.maximumInterval()),
                                         advDataMaxIntervalMs));
}

void QLeDBusAdvertiser::setAdvertisementDataForDBus()
{
    // We don't calculate the advertisement length to guard for too long advertisements.
    // There isn't adequate control and visibility on the advertisement for that.
    // - We don't know the max length (legacy or extended advertising)
    // - Bluez may truncate some of the fields on its own, making calculus here imprecise
    // - Scan response may or may not be used to offload some of the data

    // Include the power level if requested and dbus supports it
    const auto supportedIncludes = m_advManager->supportedIncludes();
    if (m_advData.includePowerLevel() && supportedIncludes.contains(advDataTXPower))
        m_advDataDBus->setIncludes({advDataTXPower});

    // Set the application provided name (valid to be empty).
    // For clarity: bluez also has "local-name" system include that could be set if no local
    // name is provided. However that would require that the LocalName DBus property would
    // not exist. Existing LocalName property when 'local-name' is included leads to an
    // advertisement error.
    m_advDataDBus->setLocalName(m_advData.localName());

    // Service UUIDs
    if (!m_advData.services().isEmpty()) {
        QStringList serviceUUIDList;
        const auto services = m_advData.services();
        for (const auto &service : services)
            serviceUUIDList << service.toString(QUuid::StringFormat::WithoutBraces);
        m_advDataDBus->setServiceUUIDs(serviceUUIDList);
    }

    // Manufacturer data
    if (m_advData.manufacturerId() != QLowEnergyAdvertisingData::invalidManufacturerId()) {
        m_advDataDBus->setManufacturerData({
                        {m_advData.manufacturerId(), QDBusVariant(m_advData.manufacturerData())}});
    }

    // Discoverability
    if (m_advDataDBus->type() == advDataTypePeripheral) {
        m_advDataDBus->setDiscoverable(m_advData.discoverability()
                                   != QLowEnergyAdvertisingData::DiscoverabilityNone);
    } else {
        qCDebug(QT_BT_BLUEZ) << "Ignoring advertisement discoverability in broadcast mode";
    }

    // Raw data
    if (!m_advData.rawData().isEmpty())
        qCWarning(QT_BT_BLUEZ) <<  "Raw advertisement data not supported, ignoring";
}

void QLeDBusAdvertiser::startAdvertising()
{
    qCDebug(QT_BT_BLUEZ) << "Start advertising" << m_advObjectPath << "on" << m_advManager->path();
    if (m_advertising) {
        qCWarning(QT_BT_BLUEZ) << "Start tried while already advertising";
        return;
    }

    if (!QDBusConnection::systemBus().registerObject(m_advObjectPath, m_advDataDBus,
                                                     QDBusConnection::ExportAllContents)) {
        qCWarning(QT_BT_BLUEZ) << "Advertisement dbus object registration failed";
        emit errorOccurred();
        return;
    }

    // Register the advertisement which starts the actual advertising.
    // We use call watcher here instead of waitForFinished() because DBus will
    // call back our advertisement object (to read data) in this same thread => would block
    auto reply = m_advManager->RegisterAdvertisement(QDBusObjectPath(m_advObjectPath), {});
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                     [this](QDBusPendingCallWatcher* watcher){
        QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            qCWarning(QT_BT_BLUEZ) << "Advertisement registration failed" << reply.error();
            if (reply.error().name() == bluezErrorFailed)
                qCDebug(QT_BT_BLUEZ) << "Advertisement could've been too large";
            QDBusConnection::systemBus().unregisterObject(m_advObjectPath);
            emit errorOccurred();
        } else {
            qCDebug(QT_BT_BLUEZ) << "Advertisement started successfully";
            m_advertising = true;
        }
        watcher->deleteLater();
    });
}

void QLeDBusAdvertiser::stopAdvertising()
{
    if (!m_advertising)
        return;

    m_advertising = false;
    auto reply = m_advManager->UnregisterAdvertisement(QDBusObjectPath(m_advObjectPath));
    reply.waitForFinished();
    if (reply.isError())
        qCWarning(QT_BT_BLUEZ) << "Error in unregistering advertisement" << reply.error();
    else
        qCDebug(QT_BT_BLUEZ) << "Advertisement unregistered successfully";
    QDBusConnection::systemBus().unregisterObject(m_advObjectPath);
}

// Called by Bluez when the advertisement has been removed (org.bluez.LEAdvertisement1.Release)
void QLeDBusAdvertiser::Release()
{
    qCDebug(QT_BT_BLUEZ) << "Advertisement" << m_advObjectPath << "released"
                         << (m_advertising ? "unexpectedly" : "");
    if (m_advertising) {
        // If we are advertising, it means the Release is unsolicited
        // and handled as an advertisement error. No need to call UnregisterAdvertisement
        m_advertising = false;
        QDBusConnection::systemBus().unregisterObject(m_advObjectPath);
        emit errorOccurred();
    }
}

QT_END_NAMESPACE
