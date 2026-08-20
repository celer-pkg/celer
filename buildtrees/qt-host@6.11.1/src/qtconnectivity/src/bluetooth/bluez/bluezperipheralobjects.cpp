// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "bluezperipheralobjects_p.h"

#include "propertiesadaptor_p.h"
#include "gattservice1adaptor_p.h"
#include "gattcharacteristic1adaptor_p.h"
#include "gattdescriptor1adaptor_p.h"

#include <QtCore/QLoggingCategory>
#include <QtDBus/QDBusConnection>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

using namespace Qt::StringLiterals;
using namespace QtBluetoothPrivate; // for D-Bus adaptors

static constexpr auto characteristicPathTemplate{"%1/char%2"_L1};
static constexpr auto descriptorPathTemplate{"%1/desc%2"_L1};
static constexpr auto servicePathTemplate{"%1/service%2"_L1};

// The interface names and error values are from BlueZ "gatt-api" documentation
static constexpr auto bluezServiceInterface{"org.bluez.GattService1"_L1};
static constexpr auto bluezCharacteristicInterface{"org.bluez.GattCharacteristic1"_L1};
static constexpr auto bluezDescriptorInterface{"org.bluez.GattDescriptor1"_L1};

static constexpr auto bluezErrorInvalidValueLength{"org.bluez.Error.InvalidValueLength"_L1};
static constexpr auto bluezErrorInvalidOffset{"org.bluez.Error.InvalidOffset"_L1};
static constexpr auto bluezErrorNotAuthorized{"org.bluez.Error.NotAuthorized"_L1};
// Bluetooth Core v5.3, 3.2.9, Vol 3, Part F
static constexpr int maximumAttributeLength{512};


QtBluezPeripheralGattObject::QtBluezPeripheralGattObject(const QString& objectPath,
           const QString& uuid, QLowEnergyHandle handle, QObject* parent)
    : QObject(parent), objectPath(objectPath), uuid(uuid), handle(handle),
      propertiesAdaptor(new OrgFreedesktopDBusPropertiesAdaptor(this))
{}

QtBluezPeripheralGattObject::~QtBluezPeripheralGattObject()
{
    unregisterObject();
}

bool QtBluezPeripheralGattObject::registerObject()
{
    if (m_registered)
        return true;

    if (QDBusConnection::systemBus().registerObject(objectPath, this)) {
        qCDebug(QT_BT_BLUEZ) << "Registered object on DBus:" << objectPath << uuid;
        m_registered = true;
        return true;
    } else {
        qCWarning(QT_BT_BLUEZ) << "Failed to register object on DBus:" << objectPath << uuid;
        return false;
    }
}

void QtBluezPeripheralGattObject::unregisterObject()
{
    if (!m_registered)
        return;
    QDBusConnection::systemBus().unregisterObject(objectPath);
    qCDebug(QT_BT_BLUEZ) << "Unregistered object on DBus:" << objectPath << uuid;
    m_registered = false;
}

void QtBluezPeripheralGattObject::accessEvent(const QVariantMap& options)
{
    // Report this event for connection management purposes
    const auto remoteDevice = options.value("device"_L1).value<QDBusObjectPath>().path();
    if (!remoteDevice.isEmpty())
        emit remoteDeviceAccessEvent(remoteDevice, options.value("mtu"_L1).toUInt());
}

QtBluezPeripheralDescriptor::QtBluezPeripheralDescriptor(
                         const QLowEnergyDescriptorData& descriptorData,
                         const QString& characteristicPath, quint16 ordinal,
                         QLowEnergyHandle handle, QLowEnergyHandle characteristicHandle,
                         QObject* parent)
    : QtBluezPeripheralGattObject(descriptorPathTemplate.arg(characteristicPath).arg(ordinal),
                 descriptorData.uuid().toString(QUuid::WithoutBraces), handle, parent),
      m_adaptor(new OrgBluezGattDescriptor1Adaptor(this)),
      m_characteristicPath(characteristicPath),
      m_characteristicHandle(characteristicHandle)
{
    if (descriptorData.value().size() > maximumAttributeLength) {
        qCWarning(QT_BT_BLUEZ) << "Descriptor value is too large, cropping it to"
                               << maximumAttributeLength;
        m_value = descriptorData.value().sliced(0, maximumAttributeLength);
    } else {
        m_value = descriptorData.value();
    }
    initializeFlags(descriptorData);
}

InterfaceList QtBluezPeripheralDescriptor::properties() const
{
    InterfaceList properties;
    properties.insert(bluezDescriptorInterface,
                      {
                          {"UUID"_L1, uuid},
                          {"Characteristic"_L1, QDBusObjectPath(m_characteristicPath)},
                          {"Flags"_L1, m_flags}
                      });
    return  properties;
}

// org.bluez.GattDescriptor1
// This function is invoked when remote device reads the value
QByteArray QtBluezPeripheralDescriptor::ReadValue(const QVariantMap &options, QString& error)
{
    accessEvent(options);
    // Offset is set by Bluez when the value size is more than MTU size.
    // Bluez deduces the value size from the first ReadValue. If the
    // received data size is larger than MTU, Bluez will take the first MTU bytes and
    // issue more ReadValue calls with the 'offset' set
    const quint16 offset = options.value("offset"_L1).toUInt();
    const quint16 mtu = options.value("mtu"_L1).toUInt();

    if (offset > m_value.length() - 1) {
        qCWarning(QT_BT_BLUEZ) << "Invalid offset" << offset << ", value len:" << m_value.length();
        error = bluezErrorInvalidOffset;
        return {};
    }

    if (offset > 0)
        return m_value.mid(offset, mtu);
    else
        return m_value;
}

// org.bluez.GattDescriptor1
// This function is invoked when remote device writes a value
QString QtBluezPeripheralDescriptor::WriteValue(const QByteArray &value,
                                                const QVariantMap &options)
{
    accessEvent(options);

    if (options.value("prepare-authorize"_L1).toBool()) {
        // Qt API doesn't provide the means for application to authorize
        qCWarning(QT_BT_BLUEZ) << "Descriptor write requires authorization."
                               << "The client device needs to be trusted beforehand";
        return bluezErrorNotAuthorized;
    }

    if (value.size() > maximumAttributeLength) {
        qCWarning(QT_BT_BLUEZ) << "Descriptor value is too large:" << value.size();
        return bluezErrorInvalidValueLength;
    }
    m_value = value;
    emit valueUpdatedByRemote(m_characteristicHandle, handle, value);
    return {};
}

// This function is called when the value has been updated locally (server-side)
bool QtBluezPeripheralDescriptor::localValueUpdate(const QByteArray& value)
{
    if (value.size() > maximumAttributeLength) {
        qCWarning(QT_BT_BLUEZ) << "Descriptor value is too large:" << value.size();
        return false;
    }
    m_value = value;
    return true;
}

void QtBluezPeripheralDescriptor::initializeFlags(const QLowEnergyDescriptorData& data)
{
    // Flag tokens are from org.bluez.GattDescriptor1 documentation
    if (data.isReadable())
        m_flags.append("read"_L1);
    if (data.readConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired)
        m_flags.append("encrypt-read"_L1);
    if (data.readConstraints() & QBluetooth::AttAccessConstraint::AttAuthenticationRequired)
        m_flags.append("encrypt-authenticated-read"_L1);

    if (data.isWritable())
        m_flags.append("write"_L1);
    if (data.writeConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired)
        m_flags.append("encrypt-write"_L1);
    if (data.writeConstraints() & QBluetooth::AttAccessConstraint::AttAuthenticationRequired)
        m_flags.append("encrypt-authenticated-write"_L1);

    if (data.readConstraints() & QBluetooth::AttAccessConstraint::AttAuthorizationRequired
            || data.writeConstraints() & QBluetooth::AttAccessConstraint::AttAuthorizationRequired)
        m_flags.append("authorize"_L1);

    if (m_flags.isEmpty()) {
        qCWarning(QT_BT_BLUEZ) << "Descriptor property flags not set" << uuid
                               << "Peripheral may fail to register";
    }
}

QtBluezPeripheralCharacteristic::QtBluezPeripheralCharacteristic(
        const QLowEnergyCharacteristicData& characteristicData,
        const QString& servicePath, quint16 ordinal,
        QLowEnergyHandle handle, QObject* parent)
    : QtBluezPeripheralGattObject(characteristicPathTemplate.arg(servicePath).arg(ordinal),
                 characteristicData.uuid().toString(QUuid::WithoutBraces), handle, parent),
      m_adaptor(new OrgBluezGattCharacteristic1Adaptor(this)),
      m_servicePath(servicePath),
      m_minimumValueLength(std::min(characteristicData.minimumValueLength(),
                                    maximumAttributeLength)),
      m_maximumValueLength(std::min(characteristicData.maximumValueLength(),
                                    maximumAttributeLength))
{
    initializeFlags(characteristicData);
    initializeValue(characteristicData.value());
}

InterfaceList QtBluezPeripheralCharacteristic::properties() const
{
    InterfaceList properties;
    properties.insert(bluezCharacteristicInterface,
                      {
                          {"UUID"_L1, uuid},
                          {"Service"_L1, QDBusObjectPath(m_servicePath)},
                          {"Flags"_L1, m_flags}
                      });
    return properties;
}

// org.bluez.GattCharacteristic1
// This function is invoked when remote device reads the value
QByteArray QtBluezPeripheralCharacteristic::ReadValue(const QVariantMap &options, QString& error)
{
    accessEvent(options);
    // Offset is set by Bluez when the value size is more than MTU size.
    // Bluez deduces the value size from the first ReadValue. If the
    // received data size is larger than MTU, Bluez will take the first MTU bytes and
    // issue more ReadValue calls with the 'offset' set
    const quint16 offset = options.value("offset"_L1).toUInt();
    const quint16 mtu = options.value("mtu"_L1).toUInt();

    if (offset > m_value.length() - 1) {
        qCWarning(QT_BT_BLUEZ) << "Invalid offset" << offset << ", value len:" << m_value.length();
        error = bluezErrorInvalidOffset;
        return {};
    }

    if (offset > 0)
        return m_value.mid(offset, mtu);
    else
        return m_value;
}

// org.bluez.GattCharacteristic1
// This function is invoked when remote device writes a value
QString QtBluezPeripheralCharacteristic::WriteValue(const QByteArray &value,
                                                    const QVariantMap &options)
{
    accessEvent(options);

    if (options.value("prepare-authorize"_L1).toBool()) {
        // Qt API doesn't provide the means for application to authorize
        qCWarning(QT_BT_BLUEZ) << "Characteristic write requires authorization."
                               << "The client device needs to be trusted beforehand";
        return bluezErrorNotAuthorized;
    }

    if (value.size() < m_minimumValueLength || value.size() > m_maximumValueLength) {
        qCWarning(QT_BT_BLUEZ) << "Characteristic value has invalid length" << value.size()
                               << "min:" << m_minimumValueLength
                               << "max:" << m_maximumValueLength;
        return bluezErrorInvalidValueLength;
    }
    m_value = value;
    emit valueUpdatedByRemote(handle, value);
    return {};
}

// This function is called when the value has been updated locally (server-side)
bool QtBluezPeripheralCharacteristic::localValueUpdate(const QByteArray& value)
{
    if (value.size() < m_minimumValueLength || value.size() > m_maximumValueLength) {
        qCWarning(QT_BT_BLUEZ) << "Characteristic value has invalid length" << value.size()
                               << "min:" << m_minimumValueLength
                               << "max:" << m_maximumValueLength;
        return false;
    }
    m_value = value;
    if (m_notifying) {
        emit propertiesAdaptor->PropertiesChanged(
                    bluezCharacteristicInterface, {{"Value"_L1, m_value}}, {});
    }
    return true;
}

// org.bluez.GattCharacteristic1
// These are called when remote client enables or disables NTF/IND
void QtBluezPeripheralCharacteristic::StartNotify()
{
    qCDebug(QT_BT_BLUEZ) << "NTF or IND enabled for characteristic" << uuid;
    m_notifying = true;
}

void QtBluezPeripheralCharacteristic::StopNotify()
{
    qCDebug(QT_BT_BLUEZ) << "NTF or IND disabled for characteristic" << uuid;
    m_notifying = false;
}


void QtBluezPeripheralCharacteristic::initializeValue(const QByteArray& value)
{
    const auto valueSize = value.size();
    if (valueSize < m_minimumValueLength || valueSize > m_maximumValueLength) {
        qCWarning(QT_BT_BLUEZ) << "Characteristic value has invalid length" << valueSize
                               << "min:" << m_minimumValueLength
                               << "max:" << m_maximumValueLength;
        m_value = QByteArray(m_minimumValueLength, 0);
    } else {
        m_value = value;
    }
}

void QtBluezPeripheralCharacteristic::initializeFlags(const QLowEnergyCharacteristicData& data)
{
    // Flag tokens are from org.bluez.GattCharacteristic1 documentation
    if (data.properties() & QLowEnergyCharacteristic::PropertyType::Broadcasting)
        m_flags.append("broadcast"_L1);
    if (data.properties() & QLowEnergyCharacteristic::PropertyType::WriteNoResponse)
        m_flags.append("write-without-response"_L1);
    if (data.properties() & QLowEnergyCharacteristic::PropertyType::Read)
        m_flags.append("read"_L1);
    if (data.properties() & QLowEnergyCharacteristic::PropertyType::Write)
        m_flags.append("write"_L1);
    if (data.properties() & QLowEnergyCharacteristic::PropertyType::Notify)
        m_flags.append("notify"_L1);
    if (data.properties() & QLowEnergyCharacteristic::PropertyType::Indicate)
        m_flags.append("indicate"_L1);
    if (data.properties() & QLowEnergyCharacteristic::PropertyType::WriteSigned)
        m_flags.append("authenticated-signed-writes"_L1);
    if (data.properties() & QLowEnergyCharacteristic::PropertyType::ExtendedProperty) {
        // If extended properties property is set, check if we have the descriptor
        // describing them. Bluez will generate the actual descriptor based on these
        // flags. For clarity: the 'extended-properties' token mentioned in the Bluez
        // API is implied by these flags.
        const auto descriptors = data.descriptors();
        for (const auto &descriptor : descriptors) {
            // Core Bluetooth v5.3 Vol 3, Part G, 3.3.3.1
            if (descriptor.uuid()
                    == QBluetoothUuid::DescriptorType::CharacteristicExtendedProperties
                    && descriptor.value().size() == 2) {
                const auto properties = descriptor.value().at(0);
                if (properties & 0x01)
                    m_flags.append("reliable-write"_L1);
                if (properties & 0x02)
                    m_flags.append("writable-auxiliaries"_L1);
            }
        }
    }

    if (data.readConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired)
        m_flags.append("encrypt-read"_L1);
    if (data.readConstraints() & QBluetooth::AttAccessConstraint::AttAuthenticationRequired)
        m_flags.append("encrypt-authenticated-read"_L1);
    if (data.writeConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired)
        m_flags.append("encrypt-write"_L1);
    if (data.writeConstraints() & QBluetooth::AttAccessConstraint::AttAuthenticationRequired)
        m_flags.append("encrypt-authenticated-write"_L1);

    if (data.readConstraints() & QBluetooth::AttAccessConstraint::AttAuthorizationRequired
            || data.writeConstraints() & QBluetooth::AttAccessConstraint::AttAuthorizationRequired)
        m_flags.append("authorize"_L1);

    if (m_flags.isEmpty()) {
        qCWarning(QT_BT_BLUEZ) << "Characteristic property flags not set" << uuid
                               << "Peripheral may fail to register";
    }
}


QtBluezPeripheralService::QtBluezPeripheralService(const QLowEnergyServiceData &serviceData,
                      const QString& applicationPath, quint16 ordinal,
                      QLowEnergyHandle handle, QObject* parent)
    : QtBluezPeripheralGattObject(servicePathTemplate.arg(applicationPath).arg(ordinal),
                 serviceData.uuid().toString(QUuid::WithoutBraces), handle, parent),
      m_isPrimary(serviceData.type() == QLowEnergyServiceData::ServiceTypePrimary),
      m_adaptor(new OrgBluezGattService1Adaptor(this))
{
}

void QtBluezPeripheralService::addIncludedService(const QString& objectPath) {
    qCDebug(QT_BT_BLUEZ) << "Adding included service" << objectPath << "for" << uuid;
    m_includedServices.append(QDBusObjectPath(objectPath));
}

InterfaceList QtBluezPeripheralService::properties() const {
    InterfaceList interfaces;
    interfaces.insert(bluezServiceInterface,{
                          {"UUID"_L1, uuid},
                          {"Primary"_L1, m_isPrimary},
                          {"Includes"_L1, QVariant::fromValue(m_includedServices)}
                      });
    return interfaces;
};

QT_END_NAMESPACE

#include "moc_bluezperipheralobjects_p.cpp"
