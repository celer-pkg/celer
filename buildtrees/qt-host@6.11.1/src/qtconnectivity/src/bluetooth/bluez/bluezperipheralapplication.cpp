// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "bluezperipheralapplication_p.h"
#include "bluezperipheralobjects_p.h"
#include "objectmanageradaptor_p.h"
#include "gattmanager1_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

using namespace Qt::StringLiterals;
using namespace QtBluetoothPrivate; // for D-Bus wrappers

static constexpr QLatin1String appObjectPathTemplate{"/qt/btle/application/%1%2/%3"};

QtBluezPeripheralApplication::QtBluezPeripheralApplication(const QString& hostAdapterPath,
                                                           QObject* parent)
    : QObject(parent),
      m_objectPath(QString(appObjectPathTemplate).
                      arg(sanitizeNameForDBus(QCoreApplication::applicationName())).
                      arg(QCoreApplication::applicationPid()).
                      arg(QRandomGenerator::global()->generate()))
{
    m_objectManager = new OrgFreedesktopDBusObjectManagerAdaptor(this);
    m_gattManager = new OrgBluezGattManager1Interface("org.bluez"_L1, hostAdapterPath,
                                                      QDBusConnection::systemBus(), this);
}

QtBluezPeripheralApplication::~QtBluezPeripheralApplication()
{
    unregisterApplication();
}

void QtBluezPeripheralApplication::registerApplication()
{
    if (m_applicationRegistered) {
        // Can happen eg. if advertisement is start-stop-started
        qCDebug(QT_BT_BLUEZ) << "Bluez peripheral application already registered";
        return;
    }

    if (m_services.isEmpty()) {
        // Registering the application to bluez without services would fail
        qCDebug(QT_BT_BLUEZ) << "No services, omiting Bluez peripheral application registration";
        return;
    }

    qCDebug(QT_BT_BLUEZ) << "Registering bluez peripheral application:" << m_objectPath;

    // Register this application object on DBus
    if (!QDBusConnection::systemBus().registerObject(m_objectPath, m_objectManager,
                                                     QDBusConnection::ExportAllContents)) {
        qCWarning(QT_BT_BLUEZ) << "Peripheral application object registration failed";
        emit errorOccurred();
        return;
    }

    // Register the service objects on DBus
    registerServices();

    // Register the gatt application to Bluez. After successful registration Bluez
    // is aware of this peripheral application and will inquiry which services this application
    // provides, see GetManagedObjects()
    auto reply = m_gattManager->RegisterApplication(QDBusObjectPath(m_objectPath), {});
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                     [this](QDBusPendingCallWatcher* watcher) {
        QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            qCWarning(QT_BT_BLUEZ) << "Application registration failed" << reply.error();
            QDBusConnection::systemBus().unregisterObject(m_objectPath);
            emit errorOccurred();
        } else {
            qCDebug(QT_BT_BLUEZ) << "Peripheral application registered as" << m_objectPath;
            m_applicationRegistered = true;
            emit registered();
        }
        watcher->deleteLater();
    });
}

void QtBluezPeripheralApplication::unregisterApplication()
{
    if (!m_applicationRegistered)
        return;
    m_applicationRegistered = false;
    auto reply = m_gattManager->UnregisterApplication(QDBusObjectPath(m_objectPath));
    reply.waitForFinished();
    if (reply.isError())
        qCWarning(QT_BT_BLUEZ) << "Error in unregistering peripheral application";
    else
        qCDebug(QT_BT_BLUEZ) << "Peripheral application unregistered successfully";
    QDBusConnection::systemBus().unregisterObject(m_objectPath);
    unregisterServices();

    qCDebug(QT_BT_BLUEZ) << "Unregistered Bluez peripheral application on DBus:" << m_objectPath;
}

void QtBluezPeripheralApplication::registerServices()
{
    // Register the service objects on DBus
    for (const auto service: std::as_const(m_services))
        service->registerObject();
    for (const auto& characteristic : std::as_const(m_characteristics))
        characteristic->registerObject();
    for (const auto& descriptor : std::as_const(m_descriptors))
        descriptor->registerObject();
}

void QtBluezPeripheralApplication::unregisterServices()
{
    // Unregister the service objects from DBus
    for (const auto service: std::as_const(m_services))
        service->unregisterObject();
    for (const auto& characteristic : std::as_const(m_characteristics))
        characteristic->unregisterObject();
    for (const auto& descriptor : std::as_const(m_descriptors))
        descriptor->unregisterObject();
}

void QtBluezPeripheralApplication::reset()
{
    unregisterApplication();

    qDeleteAll(m_services);
    m_services.clear();
    qDeleteAll(m_descriptors);
    m_descriptors.clear();
    qDeleteAll(m_characteristics);
    m_characteristics.clear();
}

void QtBluezPeripheralApplication::addService(const QLowEnergyServiceData &serviceData,
                      QSharedPointer<QLowEnergyServicePrivate> servicePrivate,
                      QLowEnergyHandle serviceHandle)
{
    if (m_applicationRegistered) {
        qCWarning(QT_BT_BLUEZ) << "Adding services to a registered application is not supported "
                                  "on Bluez DBus. Add services only before first advertisement or "
                                  "after disconnection";
        return;
    }

    // The ordinal numbers in the below object creation are used to create paths such as:
    // ../service0/char0/desc0
    // ../service0/char1/desc0
    // ../service1/char0/desc0
    // ../service1/char0/desc1
    // For the Service object itself the ordinal number is the size of the service container
    QtBluezPeripheralService* service = new QtBluezPeripheralService(
                serviceData, m_objectPath, m_services.size(), serviceHandle, this);
    m_services.insert(serviceHandle, service);

    // Add included services
    const auto includedServices = serviceData.includedServices();
    for (const auto includedService : includedServices) {
        // As per Qt documentation the included service must have been added earlier
        for (const auto s : std::as_const(m_services)) {
            if (QBluetoothUuid(s->uuid) == includedService->serviceUuid()) {
                service->addIncludedService(s->objectPath);
            }
        }
    }

    // Set characteristics and their descriptors
    quint16 characteristicOrdinal{0};
    const auto characteristics = serviceData.characteristics();
    for (const auto &characteristicData : characteristics) {
        auto characteristicHandle = handleForCharacteristic(
                    characteristicData.uuid(), servicePrivate);
        QtBluezPeripheralCharacteristic* characteristic =
                new QtBluezPeripheralCharacteristic(characteristicData,
                    service->objectPath, characteristicOrdinal++,
                    characteristicHandle, this);
        m_characteristics.insert(characteristicHandle, characteristic);
        QObject::connect(characteristic, &QtBluezPeripheralCharacteristic::valueUpdatedByRemote,
                         this, &QtBluezPeripheralApplication::characteristicValueUpdatedByRemote);
        QObject::connect(characteristic, &QtBluezPeripheralCharacteristic::remoteDeviceAccessEvent,
                         this, &QtBluezPeripheralApplication::remoteDeviceAccessEvent);

        quint16 descriptorOrdinal{0};
        const auto descriptors = characteristicData.descriptors();
        for (const auto &descriptorData : descriptors) {
            // With bluez we don't use the CCCD user has provided, because Bluez
            // generates it if 'notify/indicate' flag is set. Similarly the extended properties
            // descriptor is generated by Bluez if the related flags are set. Using the application
            // provided descriptors would result in duplicate descriptors.
            if (descriptorData.uuid()
                    == QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration
                    || descriptorData.uuid()
                    == QBluetoothUuid::DescriptorType::CharacteristicExtendedProperties) {
                continue;
            }
            auto descriptorHandle = handleForDescriptor(descriptorData.uuid(),
                                                        servicePrivate,
                                                        characteristicHandle);
            QtBluezPeripheralDescriptor* descriptor =
                    new QtBluezPeripheralDescriptor(descriptorData,
                                                 characteristic->objectPath, descriptorOrdinal++,
                                                 descriptorHandle, characteristicHandle, this);
            QObject::connect(descriptor, &QtBluezPeripheralDescriptor::valueUpdatedByRemote,
                             this, &QtBluezPeripheralApplication::descriptorValueUpdatedByRemote);
            QObject::connect(descriptor, &QtBluezPeripheralCharacteristic::remoteDeviceAccessEvent,
                             this, &QtBluezPeripheralApplication::remoteDeviceAccessEvent);
            m_descriptors.insert(descriptorHandle, descriptor);
        }
    }
}

// This function is called when characteristic is written to from Qt API
bool QtBluezPeripheralApplication::localCharacteristicWrite(QLowEnergyHandle handle,
                                                            const QByteArray& value)
{
    auto characteristic = m_characteristics.value(handle);
    if (!characteristic) {
        qCWarning(QT_BT_BLUEZ) << "DBus characteristic not found for write";
        return false;
    }
    return characteristic->localValueUpdate(value);
}

// This function is called when characteristic is written to from Qt API
bool QtBluezPeripheralApplication::localDescriptorWrite(QLowEnergyHandle handle,
                                                        const QByteArray& value)
{
    auto descriptor = m_descriptors.value(handle);
    if (!descriptor) {
        qCWarning(QT_BT_BLUEZ) << "DBus descriptor not found for write";
        return false;
    }
    return descriptor->localValueUpdate(value);
}

bool QtBluezPeripheralApplication::registrationNeeded()
{
    return !m_applicationRegistered && !m_services.isEmpty();
}

// org.freedesktop.DBus.ObjectManager
// This is called by Bluez when we register the application
ManagedObjectList QtBluezPeripheralApplication::GetManagedObjects()
{
    ManagedObjectList managedObjects;
    for (const auto service: std::as_const(m_services))
        managedObjects.insert(QDBusObjectPath(service->objectPath), service->properties());
    for (const auto& charac : std::as_const(m_characteristics))
        managedObjects.insert(QDBusObjectPath(charac->objectPath), charac->properties());
    for (const auto& descriptor : std::as_const(m_descriptors))
        managedObjects.insert(QDBusObjectPath(descriptor->objectPath), descriptor->properties());

    return managedObjects;
}

// Returns the Qt-internal handle for the characteristic
QLowEnergyHandle QtBluezPeripheralApplication::handleForCharacteristic(QBluetoothUuid uuid,
                                         QSharedPointer<QLowEnergyServicePrivate> service)
{
    const auto handles = service->characteristicList.keys();
    for (const auto handle : handles) {
        if (uuid == service->characteristicList[handle].uuid)
            return handle;
    }
    return 0;
}

// Returns the Qt-internal handle for the descriptor
QLowEnergyHandle QtBluezPeripheralApplication::handleForDescriptor(QBluetoothUuid uuid,
                                     QSharedPointer<QLowEnergyServicePrivate> service,
                                     QLowEnergyHandle characteristicHandle)
{
    const auto characteristicData = service->characteristicList[characteristicHandle];
    const auto handles = characteristicData.descriptorList.keys();
    for (const auto handle : handles) {
        if (uuid == characteristicData.descriptorList[handle].uuid)
            return handle;
    }
    return 0;
}

QT_END_NAMESPACE

#include "moc_bluezperipheralapplication_p.cpp"
