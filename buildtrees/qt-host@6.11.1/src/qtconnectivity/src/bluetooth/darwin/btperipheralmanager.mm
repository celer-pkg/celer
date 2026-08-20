// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergyadvertisingparameters.h"
#include "qlowenergycharacteristicdata.h"
#include "qlowenergydescriptordata.h"
#include "btperipheralmanager_p.h"
#include "qlowenergyservicedata.h"
#include "btnotifier_p.h"
#include "qbluetooth.h"

#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>

#include <algorithm>
#include <vector>
#include <limits>
#include <deque>
#include <map>

namespace
{

CBCharacteristicProperties cb_properties(const QLowEnergyCharacteristicData &data)
{
    // Direct 'mapping' is ok.
    return CBCharacteristicProperties(int(data.properties()));
}

CBAttributePermissions cb_permissions(const QLowEnergyCharacteristicData &data)
{
    using QLEC = QLowEnergyCharacteristic;

    const auto props = data.properties();
    CBAttributePermissions cbFlags = {};

    if ((props & QLEC::Write) || (props & QLEC::WriteNoResponse)
        || (props & QLEC::WriteSigned)) {
        cbFlags = CBAttributePermissions(cbFlags | CBAttributePermissionsWriteable);
    }

    if (props & QLEC::Read)
        cbFlags = CBAttributePermissions(cbFlags | CBAttributePermissionsReadable);

    if (data.writeConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired)
        cbFlags = CBAttributePermissions(cbFlags | CBAttributePermissionsWriteEncryptionRequired);

    if (data.readConstraints() & QBluetooth::AttAccessConstraint::AttEncryptionRequired)
        cbFlags = CBAttributePermissions(cbFlags | CBAttributePermissionsReadEncryptionRequired);

    return cbFlags;
}

ObjCStrongReference<CBMutableCharacteristic> create_characteristic(const QLowEnergyCharacteristicData &data)
{
    const ObjCStrongReference<CBMutableCharacteristic> ch([[CBMutableCharacteristic alloc] initWithType:cb_uuid(data.uuid())
                                                           properties:cb_properties(data)
                                                           value:nil
                                                           permissions:cb_permissions(data)],
                                                           RetainPolicy::noInitialRetain);
    return ch;
}

ObjCStrongReference<CBMutableDescriptor> create_descriptor(const QLowEnergyDescriptorData &data)
{
    // CoreBluetooth supports only:
    /*
    "That said, only two of these are currently supported when creating local,
    mutable descriptors: the characteristic user description descriptor and
    the characteristic format descriptor, represented by the CBUUID constants
    CBUUIDCharacteristicUserDescriptionString and CBUUIDCharacteristicFormatString"
    */

    if (data.uuid() != QBluetoothUuid::DescriptorType::CharacteristicUserDescription &&
        data.uuid() != QBluetoothUuid::DescriptorType::CharacteristicPresentationFormat) {
        qCWarning(QT_BT_DARWIN) << "unsupported descriptor" << data.uuid();
        return {};
    }

    QT_BT_MAC_AUTORELEASEPOOL

    // Descriptors are immutable with CoreBluetooth, that's why we
    // have to provide a value here and not able to change it later.
    ObjCStrongReference<NSObject> value;
    if (data.uuid() == QBluetoothUuid::DescriptorType::CharacteristicUserDescription) {
        const QString asQString(QString::fromUtf8(data.value()));
        value.reset(asQString.toNSString(), RetainPolicy::doInitialRetain); // toNSString is auto-released, we have to retain.
    } else {
        const auto nsData = data_from_bytearray(data.value());
        value.reset(nsData.data(), RetainPolicy::doInitialRetain);
    }

    const ObjCStrongReference<CBMutableDescriptor> d([[CBMutableDescriptor alloc]
                                                      initWithType:cb_uuid(data.uuid())
                                                      value:value], RetainPolicy::noInitialRetain);
    return d;
}

quint32 qt_countGATTEntries(const QLowEnergyServiceData &data)
{
    const auto maxu32 = std::numeric_limits<quint32>::max();
    // + 1 for a service itself.
    quint32 nEntries = 1 + quint32(data.includedServices().size());
    for (const auto &ch : data.characteristics()) {
        if (maxu32 - 2 < nEntries)
            return {};
        nEntries += 2;
        if (maxu32 - ch.descriptors().size() < nEntries)
            return {};
        nEntries += ch.descriptors().size();
    }

    return nEntries;
}

bool qt_validate_value_range(const QLowEnergyCharacteristicData &data)
{
    if (data.minimumValueLength() > data.maximumValueLength()
        || data.minimumValueLength() < 0) {
        return false;
    }

    return data.value().size() <= data.maximumValueLength();
}

}

@interface DarwinBTPeripheralManager (PrivateAPI)

- (void)addConnectedCentral:(CBCentral *)central;
- (CBService *)findIncludedService:(const QBluetoothUuid &)qtUUID;

- (void)addIncludedServices:(const QLowEnergyServiceData &)data
        to:(CBMutableService *)cbService
        qtService:(QLowEnergyServicePrivate *)qtService;

- (void)addCharacteristicsAndDescriptors:(const QLowEnergyServiceData &)data
        to:(CBMutableService *)cbService
        qtService:(QLowEnergyServicePrivate *)qtService;

- (CBATTError)validateWriteRequest:(CBATTRequest *)request;

@end

@implementation DarwinBTPeripheralManager
{
    ObjCScopedPointer<CBPeripheralManager> manager;
    LECBManagerNotifier *notifier;

    QLowEnergyHandle lastHandle;
    // Services in this vector are placed in such order:
    // the one that has included services, must
    // follow its included services to avoid exceptions from CBPeripheralManager.
    std::vector<ObjCStrongReference<CBMutableService>> services;
    decltype(services.size()) nextServiceToAdd;

    // Lookup map for included services:
    std::map<QBluetoothUuid, CBService *> serviceIndex;
    ObjCScopedPointer<NSMutableDictionary> advertisementData;

    GenericLEMap<CBCharacteristic *> charMap;
    GenericLEMap<ObjCStrongReference<NSMutableData>> charValues;

    QMap<QLowEnergyHandle, ValueRange> valueRanges;

    std::deque<UpdateRequest> updateQueue;

    PeripheralState state;
    NSUInteger maxNotificationValueLength;
    decltype(services.size()) nOfFailedAds;
}

- (id)initWith:(LECBManagerNotifier *)aNotifier
{
    if (self = [super init]) {
        Q_ASSERT(aNotifier);
        notifier = aNotifier;
        state = PeripheralState::idle;
        nextServiceToAdd = {};
        maxNotificationValueLength = std::numeric_limits<NSUInteger>::max();
    }

    return self;
}

- (void)dealloc
{
    [self detach];
    [super dealloc];
}

- (QSharedPointer<QLowEnergyServicePrivate>)addService:(const QLowEnergyServiceData &)data
{
    using QLES = QLowEnergyService;
    using namespace DarwinBluetooth;

    const auto nEntries = qt_countGATTEntries(data);
    if (!nEntries || nEntries > std::numeric_limits<QLowEnergyHandle>::max() - lastHandle) {
        qCCritical(QT_BT_DARWIN) << "addService: not enough handles";
        return {};
    }

    QT_BT_MAC_AUTORELEASEPOOL

    const BOOL primary = data.type() == QLowEnergyServiceData::ServiceTypePrimary;
    const auto cbUUID = cb_uuid(data.uuid());

    const ObjCStrongReference<CBMutableService>
        newCBService([[CBMutableService alloc] initWithType:cbUUID primary:primary],
                     RetainPolicy::noInitialRetain);

    if (!newCBService) {
        qCCritical(QT_BT_DARWIN) << "addService: failed to create CBMutableService";
        return {};
    }

    auto newQtService = QSharedPointer<QLowEnergyServicePrivate>::create();
    newQtService->state = QLowEnergyService::LocalService;
    newQtService->uuid = data.uuid();
    newQtService->type = primary ? QLES::PrimaryService : QLES::IncludedService;
    newQtService->startHandle = ++lastHandle;
    // Controller will be set by ... controller :)

    [self addIncludedServices:data to:newCBService qtService:newQtService.data()];
    [self addCharacteristicsAndDescriptors:data to:newCBService qtService:newQtService.data()];

    services.push_back(newCBService);
    serviceIndex[data.uuid()] = newCBService;

    newQtService->endHandle = lastHandle;

    return newQtService;
}

- (void) setParameters:(const QLowEnergyAdvertisingParameters &)parameters
         data:(const QLowEnergyAdvertisingData &)data
         scanResponse:(const QLowEnergyAdvertisingData &)scanResponse
{
    Q_UNUSED(parameters);

    // This is the last method we call on the controller's thread
    // before starting advertising on the Qt's LE queue.
    // From Apple's docs:
    /*
        - (void)startAdvertising:(NSDictionary *)advertisementData

        Advertises peripheral manager data.

        * advertisementData

        - An optional dictionary containing the data you want to advertise.
        The possible keys of an advertisementData dictionary are detailed in CBCentralManagerDelegate
        Protocol Reference. That said, only two of the keys are supported for peripheral manager objects:
        CBAdvertisementDataLocalNameKey and CBAdvertisementDataServiceUUIDsKey.
    */

    QT_BT_MAC_AUTORELEASEPOOL

    advertisementData.reset([[NSMutableDictionary alloc] init],
                            DarwinBluetooth::RetainPolicy::noInitialRetain);
    if (!advertisementData) {
        qCWarning(QT_BT_DARWIN) << "setParameters: failed to allocate "
                                   "NSMutableDictonary (advertisementData)";
        return;
    }

    auto localName = scanResponse.localName();
    if (!localName.size())
        localName = data.localName();

    if (localName.size()) {
        [advertisementData setObject:localName.toNSString()
         forKey:CBAdvertisementDataLocalNameKey];
    }

    if (data.services().isEmpty() && scanResponse.services().isEmpty())
        return;

    const ObjCScopedPointer<NSMutableArray> uuids([[NSMutableArray alloc] init],
                                                  DarwinBluetooth::RetainPolicy::noInitialRetain);
    if (!uuids) {
        qCWarning(QT_BT_DARWIN) << "setParameters: failed to allocate "
                                   "NSMutableArray (services uuids)";
        return;
    }


    for (const auto &qtUUID : data.services()) {
        const auto cbUUID = cb_uuid(qtUUID);
        if (cbUUID)
            [uuids addObject:cbUUID];
    }

    for (const auto &qtUUID : scanResponse.services()) {
        const auto cbUUID = cb_uuid(qtUUID);
        if (cbUUID)
            [uuids addObject:cbUUID];
    }

    if ([uuids count]) {
        [advertisementData setObject:uuids
         forKey:CBAdvertisementDataServiceUUIDsKey];
    }
}

- (void)startAdvertising
{
    state = PeripheralState::waitingForPowerOn;
    if (manager)
        [manager setDelegate:nil];
    manager.reset([[CBPeripheralManager alloc] initWithDelegate:self
                   queue:DarwinBluetooth::qt_LE_queue()],
                   DarwinBluetooth::RetainPolicy::noInitialRetain);
}

- (void)stopAdvertising
{
    [manager stopAdvertising];
    state = PeripheralState::idle;
}

- (void)detach
{
    if (notifier) {
        notifier->disconnect();
        notifier->deleteLater();
        notifier = nullptr;
    }

    if (state == PeripheralState::advertising) {
        [manager stopAdvertising];
        [manager setDelegate:nil];
        state = PeripheralState::idle;
    }
}

- (void)write:(const QByteArray &)value
        charHandle:(QLowEnergyHandle)charHandle
{
    using namespace DarwinBluetooth;

    if (!notifier)
        return;

    QT_BT_MAC_AUTORELEASEPOOL

    if (!charMap.contains(charHandle) || !valueRanges.contains(charHandle)) {
        emit notifier->CBManagerError(QLowEnergyController::UnknownError);
        return;
    }

    const auto & range = valueRanges[charHandle];
    if (value.size() < qsizetype(range.first) || value.size() > qsizetype(range.second)
#ifdef Q_OS_IOS
        || value.size() > DarwinBluetooth::maxValueLength) {
#else
       ) {
#endif
        qCWarning(QT_BT_DARWIN) << "ignoring value of invalid length" << value.size();
        return;
    }

    emit notifier->characteristicWritten(charHandle, value);

    const auto nsData = mutable_data_from_bytearray(value);
    charValues[charHandle] = nsData;
    // We copy data here: sending update requests is async (see sendUpdateRequests),
    // by the time we're allowed to actually send them, the data can change again
    // and we'll send an 'out of order' value.
    const ObjCStrongReference<NSData> copy([NSData dataWithData:nsData], RetainPolicy::doInitialRetain);
    updateQueue.push_back(UpdateRequest{charHandle, copy});
    [self sendUpdateRequests];
}

- (void) addServicesToPeripheral
{
    Q_ASSERT(manager);

    if (nextServiceToAdd < services.size())
        [manager addService:services[nextServiceToAdd++]];
}

// CBPeripheralManagerDelegate:

- (void)peripheralManagerDidUpdateState:(CBPeripheralManager *)peripheral
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

    if (peripheral != manager || !notifier)
        return;

    if (peripheral.state == CBManagerStatePoweredOn) {
        // "Bluetooth is currently powered on and is available to use."
        if (state == PeripheralState::waitingForPowerOn) {
            [manager removeAllServices];
            nextServiceToAdd = {};
            state = PeripheralState::advertising;
            nOfFailedAds = 0;
            [self addServicesToPeripheral];
        }
        return;
    }

    /*
    "A state with a value lower than CBPeripheralManagerStatePoweredOn implies that
    advertising has stopped and that any connected centrals have been disconnected."
    */

    maxNotificationValueLength = std::numeric_limits<NSUInteger>::max();

    if (state == PeripheralState::advertising) {
        state = PeripheralState::waitingForPowerOn;
    } else if (state == PeripheralState::connected) {
        state = PeripheralState::idle;
        emit notifier->disconnected();
    }

    // The next four states are _below_ "powered off"; according to the docs:
    /*
     "In addition, the local database is cleared and all services must be
     explicitly added again."
    */

    if (peripheral.state == CBManagerStateUnsupported) {
        state = PeripheralState::idle;
        emit notifier->LEnotSupported();
    } else if (peripheral.state == CBManagerStateUnauthorized) {
        state = PeripheralState::idle;
        emit notifier->CBManagerError(QLowEnergyController::MissingPermissionsError);
    }

#pragma clang diagnostic pop
}

- (void)peripheralManagerDidStartAdvertising:(CBPeripheralManager *)peripheral
        error:(NSError *)error
{
    if (peripheral != manager || !notifier)
        return;

    if (error) {
        NSLog(@"failed to start advertising, error: %@", error);
        state = PeripheralState::idle;
        emit notifier->CBManagerError(QLowEnergyController::AdvertisingError);
    }
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral
        didAddService:(CBService *)service error:(NSError *)error
{
    Q_UNUSED(service);

    if (peripheral != manager || !notifier)
        return;

    if (error) {
        NSLog(@"failed to add a service, error: %@", error);
        if (++nOfFailedAds == services.size()) {
            emit notifier->CBManagerError(QLowEnergyController::AdvertisingError);
            state = PeripheralState::idle;
            return;
        }
    }

    if (nextServiceToAdd == services.size()) {
        nOfFailedAds = 0; // Discard any failed, some services made it into advertising.
        [manager startAdvertising:[advertisementData count] ? advertisementData.get() : nil];
    } else {
        [self addServicesToPeripheral];
    }
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral central:(CBCentral *)central
        didSubscribeToCharacteristic:(CBCharacteristic *)characteristic
{
    Q_UNUSED(characteristic);

    if (peripheral != manager || !notifier)
        return;

    [self addConnectedCentral:central];

    if (const auto handle = charMap.key(characteristic))
        emit notifier->notificationEnabled(handle, true);
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral central:(CBCentral *)central
        didUnsubscribeFromCharacteristic:(CBCharacteristic *)characteristic
{
    Q_UNUSED(characteristic);

    if (peripheral != manager || !notifier)
        return;

    const auto handle = charMap.key(characteristic);
    if (![static_cast<CBMutableCharacteristic*>(characteristic).subscribedCentrals count]
            && handle)
            emit notifier->notificationEnabled(handle, false);
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral
        didReceiveReadRequest:(CBATTRequest *)request
{
    if (peripheral != manager || !notifier)
        return;

    QT_BT_MAC_AUTORELEASEPOOL

    const auto handle = charMap.key(request.characteristic);
    if (!handle || !charValues.contains(handle)) {
        qCWarning(QT_BT_DARWIN) << "invalid read request, unknown characteristic";
        [manager respondToRequest:request withResult:CBATTErrorInvalidHandle];
        return;
    }

    const auto &value = charValues[handle];
    if (request.offset > [value length]) {
        qCWarning(QT_BT_DARWIN) << "invalid offset in a read request";
        [manager respondToRequest:request withResult:CBATTErrorInvalidOffset];
        return;
    }

    [self addConnectedCentral:request.central];

    NSData *dataToSend = nil;
    if (!request.offset) {
        dataToSend = value;
    } else {
        dataToSend = [value subdataWithRange:
                      NSMakeRange(request.offset, [value length] - request.offset)];
    }

    request.value = dataToSend;
    [manager respondToRequest:request withResult:CBATTErrorSuccess];
}


- (void)writeValueForCharacteristic:(QLowEnergyHandle) charHandle
        withWriteRequest:(CBATTRequest *)request
{
    Q_ASSERT(charHandle);
    Q_ASSERT(request);

    Q_ASSERT(valueRanges.contains(charHandle));
    const auto &range = valueRanges[charHandle];
    Q_ASSERT(request.offset <= range.second
             &&  request.value.length <= range.second - request.offset);

    Q_ASSERT(charValues.contains(charHandle));
    NSMutableData *const value = charValues[charHandle];
    if (request.offset + request.value.length > value.length)
        [value increaseLengthBy:request.offset + request.value.length - value.length];

    [value replaceBytesInRange:NSMakeRange(request.offset, request.value.length)
                     withBytes:request.value.bytes];
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral
        didReceiveWriteRequests:(NSArray *)requests
{
    using namespace DarwinBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL

    if (peripheral != manager || !notifier) {
        // Detached already.
        return;
    }

    // We first test if all requests are valid
    // since CoreBluetooth requires "all or none"
    // and respond only _once_ to the first one.
    for (CBATTRequest *request in requests) {
        const auto status = [self validateWriteRequest:request];
        if (status != CBATTErrorSuccess) {
            [manager respondToRequest:[requests objectAtIndex:0]
                           withResult:status];
            return;
        }
    }

    std::map<QLowEnergyHandle, NSUInteger> updated;

    for (CBATTRequest *request in requests) {
        // Transition to 'connected' if needed.
        [self addConnectedCentral:request.central];
        const auto charHandle = charMap.key(request.characteristic);
        const auto prevLen = updated[charHandle];
        updated[charHandle] = std::max(request.offset + request.value.length,
                                       prevLen);
        [self writeValueForCharacteristic:charHandle withWriteRequest:request];
    }

    for (const auto &pair : updated) {
        const auto handle = pair.first;
        NSMutableData *value = charValues[handle];
        value.length = pair.second;
        emit notifier->characteristicUpdated(handle, qt_bytearray(value));
        const ObjCStrongReference<NSData> copy([NSData dataWithData:value],
                                               RetainPolicy::doInitialRetain);
        updateQueue.push_back(UpdateRequest{handle, copy});
    }

    if (requests.count) {
        [manager respondToRequest:[requests objectAtIndex:0]
                       withResult:CBATTErrorSuccess];
    }

    [self sendUpdateRequests];
}

- (void)peripheralManagerIsReadyToUpdateSubscribers:(CBPeripheralManager *)peripheral
{
    if (peripheral != manager || !notifier) {
        // Detached.
        return;
    }

    [self sendUpdateRequests];
}

- (void)sendUpdateRequests
{
    QT_BT_MAC_AUTORELEASEPOOL

    while (updateQueue.size()) {
        const auto &request = updateQueue.front();
        if (charMap.contains(request.charHandle)) {
            if (maxNotificationValueLength < [request.value length]) {
                qCWarning(QT_BT_DARWIN) << "value of length" << [request.value length]
                                        << "will possibly be truncated to"
                                        << maxNotificationValueLength;
            }
            const BOOL res = [manager updateValue:request.value
                              forCharacteristic:static_cast<CBMutableCharacteristic *>(charMap[request.charHandle])
                              onSubscribedCentrals:nil];
            if (!res) {
                // Have to wait for the 'ManagerIsReadyToUpdate'.
                break;
            }
        }

        updateQueue.pop_front();
    }
}

// Private API:

- (void)addConnectedCentral:(CBCentral *)central
{
    if (!central)
        return;

    if (!notifier) {
        // We were detached.
        return;
    }

    maxNotificationValueLength = std::min(maxNotificationValueLength,
                                          central.maximumUpdateValueLength);

    QT_BT_MAC_AUTORELEASEPOOL

    if (state == PeripheralState::advertising) {
        state = PeripheralState::connected;
        emit notifier->connected();
    }
}

- (CBService *)findIncludedService:(const QBluetoothUuid &)qtUUID
{
    const auto it = serviceIndex.find(qtUUID);
    if (it == serviceIndex.end())
        return nil;

    return it->second;
}

- (void)addIncludedServices:(const QLowEnergyServiceData &)data
        to:(CBMutableService *)cbService
        qtService:(QLowEnergyServicePrivate *)qtService
{
    Q_ASSERT(cbService);
    Q_ASSERT(qtService);

    QT_BT_MAC_AUTORELEASEPOOL

    ObjCScopedPointer<NSMutableArray> included([[NSMutableArray alloc] init],
                                               DarwinBluetooth::RetainPolicy::noInitialRetain);
    if (!included) {
        qCWarning(QT_BT_DARWIN) << "addIncludedSerivces: failed "
                                   "to allocate NSMutableArray";
        return;
    }

    for (auto includedService : data.includedServices()) {
        if (CBService *cbs = [self findIncludedService:includedService->serviceUuid()]) {
            [included addObject:cbs];
            qtService->includedServices << includedService->serviceUuid();
            ++lastHandle;
        } else {
            qCWarning(QT_BT_DARWIN) << "can not use" << includedService->serviceUuid()
                                    << "as included, it has to be added first";
        }
    }

    if ([included count])
        cbService.includedServices = included;
}

- (void)addCharacteristicsAndDescriptors:(const QLowEnergyServiceData &)data
        to:(CBMutableService *)cbService
        qtService:(QLowEnergyServicePrivate *)qtService
{
    Q_ASSERT(cbService);
    Q_ASSERT(qtService);

    QT_BT_MAC_AUTORELEASEPOOL

    ObjCScopedPointer<NSMutableArray> newCBChars([[NSMutableArray alloc] init],
                                                 DarwinBluetooth::RetainPolicy::noInitialRetain);
    if (!newCBChars) {
        qCWarning(QT_BT_DARWIN) << "addCharacteristicsAndDescritptors: "
                                   "failed to allocate NSMutableArray "
                                   "(characteristics)";
        return;
    }

    for (const auto &ch : data.characteristics()) {
        if (!qt_validate_value_range(ch)) {
            qCWarning(QT_BT_DARWIN) << "addCharacteristicsAndDescritptors: "
                                       "invalid value size/min-max length";
            continue;
        }

#ifdef Q_OS_IOS
        if (ch.value().size() > DarwinBluetooth::maxValueLength) {
            qCWarning(QT_BT_DARWIN) << "addCharacteristicsAndDescritptors: "
                                       "value exceeds the maximal permitted "
                                       "value length ("
                                    << DarwinBluetooth::maxValueLength
                                    << "octets) on the platform";
            continue;
        }
#endif

        const auto cbChar(create_characteristic(ch));
        if (!cbChar) {
            qCWarning(QT_BT_DARWIN) << "addCharacteristicsAndDescritptors: "
                                       "failed to allocate a characteristic";
            continue;
        }

        const auto nsData(mutable_data_from_bytearray(ch.value()));
        if (!nsData) {
            qCWarning(QT_BT_DARWIN) << "addCharacteristicsAndDescritptors: "
                                       "addService: failed to allocate NSData (char value)";
            continue;
        }

        [newCBChars addObject:cbChar];

        const auto declHandle = ++lastHandle;
        // CB part:
        charMap[declHandle] = cbChar;
        charValues[declHandle] = nsData;
        valueRanges[declHandle] = ValueRange(ch.minimumValueLength(), ch.maximumValueLength());
        // QT part:
        QLowEnergyServicePrivate::CharData charData;
        charData.valueHandle = declHandle;
        charData.uuid = ch.uuid();
        charData.properties = ch.properties();
        charData.value = ch.value();

        const ObjCScopedPointer<NSMutableArray> newCBDescs([[NSMutableArray alloc] init],
                                                           DarwinBluetooth::RetainPolicy::noInitialRetain);
        if (!newCBDescs) {
            qCWarning(QT_BT_DARWIN) << "addCharacteristicsAndDescritptors: "
                                       "failed to allocate NSMutableArray "
                                       "(descriptors)";
            continue;
        }

        for (const auto &desc : ch.descriptors()) {
            // CB part:
            const auto cbDesc(create_descriptor(desc));
            const auto descHandle = ++lastHandle;
            if (cbDesc) {
                // See comments in create_descriptor on
                // why cbDesc can be nil.
                [newCBDescs addObject:cbDesc];
            }
            // QT part:
            QLowEnergyServicePrivate::DescData descData;
            descData.uuid = desc.uuid();
            descData.value = desc.value();
            charData.descriptorList.insert(descHandle, descData);
        }

        if ([newCBDescs count])
            cbChar.data().descriptors = newCBDescs.get();

        qtService->characteristicList.insert(declHandle, charData);
    }

    if ([newCBChars count])
        cbService.characteristics = newCBChars.get();
}

- (CBATTError)validateWriteRequest:(CBATTRequest *)request
{
    Q_ASSERT(request);

    QT_BT_MAC_AUTORELEASEPOOL

    const auto handle = charMap.key(request.characteristic);
    if (!handle || !charValues.contains(handle))
        return CBATTErrorInvalidHandle;

    Q_ASSERT(valueRanges.contains(handle));

    const auto &range = valueRanges[handle];
    if (request.offset > range.second)
        return CBATTErrorInvalidOffset;

    if (request.value.length > range.second - request.offset)
        return CBATTErrorInvalidAttributeValueLength;

    return CBATTErrorSuccess;
}

@end
