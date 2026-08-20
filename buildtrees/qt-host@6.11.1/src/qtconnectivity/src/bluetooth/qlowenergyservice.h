// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Javier S. Pedro <maemo@javispedro.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QLOWENERGYSERVICE_H
#define QLOWENERGYSERVICE_H

#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyCharacteristic>

QT_BEGIN_NAMESPACE

class QLowEnergyServicePrivate;
class Q_BLUETOOTH_EXPORT QLowEnergyService : public QObject
{
    Q_OBJECT
public:
    enum ServiceType {
        PrimaryService = 0x0001,
        IncludedService = 0x0002
    };
    Q_ENUM(ServiceType)
    Q_DECLARE_FLAGS(ServiceTypes, ServiceType)

    enum ServiceError {
        NoError = 0,
        OperationError,
        CharacteristicWriteError,
        DescriptorWriteError,
        UnknownError,
        CharacteristicReadError,
        DescriptorReadError
    };
    Q_ENUM(ServiceError)

    enum ServiceState {
        InvalidService = 0,
        RemoteService,
        RemoteServiceDiscovering, // discoverDetails() called and running
        RemoteServiceDiscovered,  // all details have been synchronized
        LocalService,

#if QT_DEPRECATED_SINCE(6, 2)
// for source compatibility:
        DiscoveryRequired
            Q_DECL_ENUMERATOR_DEPRECATED_X(
                "DiscoveryRequired was renamed to RemoteService.")
                = RemoteService,
        DiscoveringService
            Q_DECL_ENUMERATOR_DEPRECATED_X(
                "DiscoveringService was renamed to RemoteServiceDiscovering.")
                = RemoteServiceDiscovering,
        ServiceDiscovered
            Q_DECL_ENUMERATOR_DEPRECATED_X(
                "ServiceDiscovered was renamed to RemoteServiceDiscovered.")
                = RemoteServiceDiscovered,
#endif
    };
    Q_ENUM(ServiceState)

    enum DiscoveryMode {
        FullDiscovery,      // standard, reads all attributes
        SkipValueDiscovery  // does not read characteristic values and descriptors
    };
    Q_ENUM(DiscoveryMode)

    enum WriteMode {
        WriteWithResponse = 0,
        WriteWithoutResponse,
        WriteSigned
    };
    Q_ENUM(WriteMode)

    ~QLowEnergyService();

    QList<QBluetoothUuid> includedServices() const;

    QLowEnergyService::ServiceTypes type() const;
    QLowEnergyService::ServiceState state() const;

    QLowEnergyCharacteristic characteristic(const QBluetoothUuid &uuid) const;
    QList<QLowEnergyCharacteristic> characteristics() const;
    QBluetoothUuid serviceUuid() const;
    QString serviceName() const;

    void discoverDetails(DiscoveryMode mode = FullDiscovery);

    ServiceError error() const;

    bool contains(const QLowEnergyCharacteristic &characteristic) const;
    void readCharacteristic(const QLowEnergyCharacteristic &characteristic);
    void writeCharacteristic(const QLowEnergyCharacteristic &characteristic,
                             const QByteArray &newValue,
                             WriteMode mode = WriteWithResponse);

    bool contains(const QLowEnergyDescriptor &descriptor) const;
    void readDescriptor(const QLowEnergyDescriptor &descriptor);
    void writeDescriptor(const QLowEnergyDescriptor &descriptor,
                         const QByteArray &newValue);

Q_SIGNALS:
    void stateChanged(QLowEnergyService::ServiceState newState);
    void characteristicChanged(const QLowEnergyCharacteristic &info,
                               const QByteArray &value);
    void characteristicRead(const QLowEnergyCharacteristic &info,
                            const QByteArray &value);
    void characteristicWritten(const QLowEnergyCharacteristic &info,
                               const QByteArray &value);
    void descriptorRead(const QLowEnergyDescriptor &info,
                        const QByteArray &value);
    void descriptorWritten(const QLowEnergyDescriptor &info,
                           const QByteArray &value);
    void errorOccurred(QLowEnergyService::ServiceError error);

private:
    Q_DECLARE_PRIVATE(QLowEnergyService)
    QSharedPointer<QLowEnergyServicePrivate> d_ptr;

    // QLowEnergyController is the factory for this class
    friend class QLowEnergyController;
    friend class QLowEnergyControllerPrivate;
    friend class QLowEnergyControllerPrivateBluez;
    friend class QLowEnergyControllerPrivateAndroid;
    friend class QLowEnergyControllerPrivateDarwin;
    QLowEnergyService(QSharedPointer<QLowEnergyServicePrivate> p,
                      QObject *parent = nullptr);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QLowEnergyService::ServiceTypes)

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN_TAGGED(QLowEnergyService::ServiceError, QLowEnergyService__ServiceError,
                               Q_BLUETOOTH_EXPORT)
QT_DECL_METATYPE_EXTERN_TAGGED(QLowEnergyService::ServiceState, QLowEnergyService__ServiceState,
                               Q_BLUETOOTH_EXPORT)
QT_DECL_METATYPE_EXTERN_TAGGED(QLowEnergyService::ServiceType, QLowEnergyService__ServiceType,
                               Q_BLUETOOTH_EXPORT)
QT_DECL_METATYPE_EXTERN_TAGGED(QLowEnergyService::WriteMode, QLowEnergyService__WriteMode,
                               Q_BLUETOOTH_EXPORT)

#endif // QLOWENERGYSERVICE_H
