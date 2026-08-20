// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothserviceinfo.h"
#include "btservicerecord_p.h"

#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>
#include <QtCore/qurl.h>

#include <IOBluetooth/IOBluetooth.h>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

//
// Returns a dictionary containing the Bluetooth RFCOMM service definition
// corresponding to the provided |uuid| and |options|.
namespace {

typedef ObjCStrongReference<NSMutableDictionary> Dictionary;
typedef ObjCStrongReference<IOBluetoothSDPUUID> SDPUUid;
typedef ObjCStrongReference<NSNumber> Number;
typedef QBluetoothServiceInfo QSInfo;
typedef QSInfo::Sequence Sequence;
typedef QSInfo::AttributeId AttributeId;

}

#if 0
QBluetoothUuid profile_uuid(const QBluetoothServiceInfo &serviceInfo)
{
    // Strategy to pick service uuid:
    // 1.) use serviceUuid()
    // 2.) use first custom uuid if available
    // 3.) use first service class uuid
    QBluetoothUuid serviceUuid(serviceInfo.serviceUuid());

    if (serviceUuid.isNull()) {
        const QVariant var(serviceInfo.attribute(QBluetoothServiceInfo::ServiceClassIds));
        if (var.isValid()) {
            const Sequence seq(var.value<Sequence>());

            for (qsizetype i = 0; i < seq.size(); ++i) {
                QBluetoothUuid uuid(seq.at(i).value<QBluetoothUuid>());
                if (uuid.isNull())
                    continue;

                const int size = uuid.minimumSize();
                if (size == 2 || size == 4) { // Base UUID derived
                    if (serviceUuid.isNull())
                        serviceUuid = uuid;
                } else {
                    return uuid;
                }
            }
        }
    }

    return serviceUuid;
}
#endif

template<class IntType>
Number variant_to_nsnumber(const QVariant &);

template<>
Number variant_to_nsnumber<unsigned char>(const QVariant &var)
{
    return Number([NSNumber numberWithUnsignedChar:var.value<unsigned char>()], RetainPolicy::doInitialRetain);
}

template<>
Number variant_to_nsnumber<unsigned short>(const QVariant &var)
{
    return Number([NSNumber numberWithUnsignedShort:var.value<unsigned short>()], RetainPolicy::doInitialRetain);
}

template<>
Number variant_to_nsnumber<unsigned>(const QVariant &var)
{
    return Number([NSNumber numberWithUnsignedInt:var.value<unsigned>()], RetainPolicy::doInitialRetain);
}

template<>
Number variant_to_nsnumber<char>(const QVariant &var)
{
    return Number([NSNumber numberWithChar:var.value<char>()], RetainPolicy::doInitialRetain);
}

template<>
Number variant_to_nsnumber<short>(const QVariant &var)
{
    return Number([NSNumber numberWithShort:var.value<short>()], RetainPolicy::doInitialRetain);
}

template<>
Number variant_to_nsnumber<int>(const QVariant &var)
{
    return Number([NSNumber numberWithInt:var.value<int>()], RetainPolicy::doInitialRetain);
}

template<class ValueType>
void add_attribute(const QVariant &var, AttributeId key, Dictionary dict)
{
    Q_ASSERT_X(dict, Q_FUNC_INFO, "invalid dictionary (nil)");

    if (!var.canConvert<ValueType>())
        return;

    const Number num(variant_to_nsnumber<ValueType>(var));
    [dict setObject:num forKey:[NSString stringWithFormat:@"%x", int(key)]];
}

template<>
void add_attribute<QString>(const QVariant &var, AttributeId key, Dictionary dict)
{
    Q_ASSERT_X(dict, Q_FUNC_INFO, "invalid dictionary (nil)");

    if (!var.canConvert<QString>())
        return;

    const QString string(var.value<QString>());
    if (!string.isEmpty()) {
        if (NSString *const nsString = string.toNSString())
            [dict setObject:nsString forKey:[NSString stringWithFormat:@"%x", int(key)]];
    }
}

template<>
void add_attribute<QBluetoothUuid>(const QVariant &var, AttributeId key, Dictionary dict)
{
    Q_ASSERT_X(dict, Q_FUNC_INFO, "invalid dictionary (nil)");

    if (!var.canConvert<QBluetoothUuid>())
        return;

    SDPUUid ioUUID(iobluetooth_uuid(var.value<QBluetoothUuid>()));
    [dict setObject:ioUUID forKey:[NSString stringWithFormat:@"%x", int(key)]];
}

template<>
void add_attribute<QUrl>(const QVariant &var, AttributeId key, Dictionary dict)
{
    Q_ASSERT_X(dict, Q_FUNC_INFO, "invalid dictionary (nil)");

    if (!var.canConvert<QUrl>())
        return;

    Q_UNUSED(var);
    Q_UNUSED(key);
    Q_UNUSED(dict);

    // TODO: not clear how should I pass an url in a dictionary, NSURL does not work.
}

template<class ValueType>
void add_attribute(const QVariant &var, NSMutableArray *list);

template<class ValueType>
void add_attribute(const QVariant &var, NSMutableArray *list)
{
    Q_ASSERT_X(list, Q_FUNC_INFO, "invalid list (nil)");

    if (!var.canConvert<ValueType>())
        return;

    const Number num(variant_to_nsnumber<ValueType>(var));
    [list addObject:num];
}

template<>
void add_attribute<unsigned short>(const QVariant &var, NSMutableArray *list)
{
    Q_ASSERT_X(list, Q_FUNC_INFO, "invalid list (nil)");

    if (!var.canConvert<unsigned short>())
        return;

    const Number num(variant_to_nsnumber<unsigned short>(var));

    NSDictionary* dict = @{
        @"DataElementType"  : [NSNumber numberWithInt:1],
        @"DataElementSize"  : [NSNumber numberWithInt:2],
        @"DataElementValue" : num
    };

    [list addObject: dict];
}

template<>
void add_attribute<QString>(const QVariant &var, NSMutableArray *list)
{
    Q_ASSERT_X(list, Q_FUNC_INFO, "invalid list (nil)");

    if (!var.canConvert<QString>())
        return;

    const QString string(var.value<QString>());
    if (!string.isEmpty()) {
        if (NSString *const nsString = string.toNSString())
            [list addObject:nsString];
    }
}

template<>
void add_attribute<QBluetoothUuid>(const QVariant &var, NSMutableArray *list)
{
    Q_ASSERT_X(list, Q_FUNC_INFO, "invalid list (nil)");

    if (!var.canConvert<QBluetoothUuid>())
        return;

    SDPUUid ioUUID(iobluetooth_uuid(var.value<QBluetoothUuid>()));
    [list addObject:ioUUID];
}

template<>
void add_attribute<QUrl>(const QVariant &var, NSMutableArray *list)
{
    Q_ASSERT_X(list, Q_FUNC_INFO, "invalid list (nil)");

    if (!var.canConvert<QUrl>())
        return;

    Q_UNUSED(var);
    Q_UNUSED(list);
    // TODO: not clear how should I pass an url in a dictionary, NSURL does not work.
}

void add_rfcomm_protocol_descriptor_list(uint16 channelID, Dictionary dict)
{
    Q_ASSERT_X(dict, Q_FUNC_INFO, "invalid dictionary (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;

    // Objective-C has literals (for arrays and dictionaries), but it will not compile
    // on 10.7 or below, so quite a lot of code here.

    NSMutableArray *const descriptorList = [NSMutableArray array];

    IOBluetoothSDPUUID *const l2capUUID = [IOBluetoothSDPUUID uuid16:kBluetoothSDPUUID16L2CAP];
    NSArray *const l2capList = [NSArray arrayWithObject:l2capUUID];

    [descriptorList addObject:l2capList];
    //
    IOBluetoothSDPUUID *const rfcommUUID = [IOBluetoothSDPUUID uuid16:kBluetoothSDPUUID16RFCOMM];
    NSMutableDictionary *const rfcommDict = [NSMutableDictionary dictionary];
    [rfcommDict setObject:[NSNumber numberWithInt:1] forKey:@"DataElementType"];
    [rfcommDict setObject:[NSNumber numberWithInt:1] forKey:@"DataElementSize"];
    [rfcommDict setObject:[NSNumber numberWithInt:channelID] forKey:@"DataElementValue"];
    //
    NSMutableArray *const rfcommList = [NSMutableArray array];
    [rfcommList addObject:rfcommUUID];
    [rfcommList addObject:rfcommDict];

    [descriptorList addObject:rfcommList];
    [dict setObject:descriptorList forKey:[NSString stringWithFormat:@"%x",
        kBluetoothSDPAttributeIdentifierProtocolDescriptorList]];
}

void add_l2cap_protocol_descriptor_list(uint16 psm, Dictionary dict)
{
    Q_ASSERT_X(dict, Q_FUNC_INFO, "invalid dictionary (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;

    // Objective-C has literals (for arrays and dictionaries), but it will not compile
    // on 10.7 or below, so quite a lot of code here.

    NSMutableArray *const descriptorList = [NSMutableArray array];
    NSMutableArray *const l2capList = [NSMutableArray array];

    IOBluetoothSDPUUID *const l2capUUID = [IOBluetoothSDPUUID uuid16:kBluetoothSDPUUID16L2CAP];
    [l2capList addObject:l2capUUID];

    NSMutableDictionary *const l2capDict = [NSMutableDictionary dictionary];
    [l2capDict setObject:[NSNumber numberWithInt:1] forKey:@"DataElementType"];
    [l2capDict setObject:[NSNumber numberWithInt:2] forKey:@"DataElementSize"];
    [l2capDict setObject:[NSNumber numberWithInt:psm] forKey:@"DataElementValue"];
    [l2capList addObject:l2capDict];

    [descriptorList addObject:l2capList];
    [dict setObject:descriptorList forKey:[NSString stringWithFormat:@"%x",
        kBluetoothSDPAttributeIdentifierProtocolDescriptorList]];
}

bool add_attribute(const QVariant &var, AttributeId key, NSMutableArray *list)
{
    Q_ASSERT_X(list, Q_FUNC_INFO, "invalid list (nil)");

    if (var.canConvert<Sequence>())
        return false;

    if (var.typeId() == QMetaType::QString) {
        //ServiceName, ServiceDescription, ServiceProvider.
        add_attribute<QString>(var, list);
    } else if (var.userType() == qMetaTypeId<QBluetoothUuid>()) {
        add_attribute<QBluetoothUuid>(var, list);
    } else {
        // Here we need 'key' to understand the type.
        // We can have different integer types actually, so I have to check
        // the 'key' to be sure the conversion is reasonable.
        switch (key) {
        case QSInfo::ServiceRecordHandle:
        case QSInfo::ServiceRecordState:
        case QSInfo::ServiceInfoTimeToLive:
            add_attribute<unsigned>(var, list);
            break;
        case QSInfo::BluetoothProfileDescriptorList:
            add_attribute<unsigned short>(var, list);
            break;
        case QSInfo::ServiceAvailability:
            add_attribute<unsigned char>(var, list);
            break;
        case QSInfo::IconUrl:
        case QSInfo::DocumentationUrl:
        case QSInfo::ClientExecutableUrl:
            add_attribute<QUrl>(var, list);
            break;
        default:;
        }
    }

    return true;
}

bool add_attribute(const QBluetoothServiceInfo &serviceInfo, AttributeId key, Dictionary dict)
{
    Q_ASSERT_X(dict, Q_FUNC_INFO, "invalid dict (nil)");

    const QVariant var(serviceInfo.attribute(key));
    if (var.canConvert<Sequence>())
        return false;

    if (var.typeId() == QMetaType::QString) {
        //ServiceName, ServiceDescription, ServiceProvider.
        add_attribute<QString>(var, key, dict);
    } else if (var.userType() == qMetaTypeId<QBluetoothUuid>()) {
        add_attribute<QBluetoothUuid>(serviceInfo.attribute(key), key, dict);
    } else {
        // We can have different integer types actually, so I have to check
        // the 'key' to be sure the conversion is reasonable.
        switch (key) {
        case QSInfo::ServiceRecordHandle:
        case QSInfo::ServiceRecordState:
        case QSInfo::ServiceInfoTimeToLive:
            add_attribute<unsigned>(serviceInfo.attribute(key), key, dict);
            break;
        case QSInfo::ServiceAvailability:
            add_attribute<unsigned char>(serviceInfo.attribute(key), key, dict);
            break;
        case QSInfo::IconUrl:
        case QSInfo::DocumentationUrl:
        case QSInfo::ClientExecutableUrl:
            add_attribute<QUrl>(serviceInfo.attribute(key), key, dict);
            break;
        default:;
        }
    }

    return true;
}

bool add_sequence_attribute(const QVariant &var, AttributeId key, NSMutableArray *list)
{
    // Add a "nested" sequence.
    Q_ASSERT_X(list, Q_FUNC_INFO, "invalid list (nil)");

    if (var.isNull() || !var.canConvert<Sequence>())
        return false;

    NSMutableArray *const nested = [NSMutableArray array];
    [list addObject:nested];

    const Sequence sequence(var.value<Sequence>());
    for (const QVariant &var : sequence) {
        if (var.canConvert<Sequence>()) {
            add_sequence_attribute(var, key, nested);
        } else {
            add_attribute(var, key, nested);
        }
    }

    return true;
}

bool add_sequence_attribute(const QBluetoothServiceInfo &serviceInfo, AttributeId key, Dictionary dict)
{
    Q_ASSERT_X(dict, Q_FUNC_INFO, "invalid dictionary (nil)");

    const QVariant &var(serviceInfo.attribute(key));
    if (var.isNull() || !var.canConvert<Sequence>())
        return false;

    QT_BT_MAC_AUTORELEASEPOOL;

    NSMutableArray *const list = [NSMutableArray array];
    const Sequence sequence(var.value<Sequence>());
    for (const QVariant &element : sequence) {
        if (!add_sequence_attribute(element, key, list))
            add_attribute(element, key, list);
    }
    [dict setObject:list forKey:[NSString stringWithFormat:@"%x", int(key)]];
    return true;
}

Dictionary iobluetooth_service_dictionary(const QBluetoothServiceInfo &serviceInfo)
{
    Dictionary dict;

    if (serviceInfo.socketProtocol() == QBluetoothServiceInfo::UnknownProtocol)
        return dict;

    const QList<quint16> attributeIds(serviceInfo.attributes());
    if (!attributeIds.size())
        return dict;

    dict.reset([[NSMutableDictionary alloc] init], RetainPolicy::noInitialRetain);

    for (quint16 key : attributeIds) {
        if (key == QSInfo::ProtocolDescriptorList) // We handle it in a special way.
            continue;
        // TODO: check if non-sequence QVariant still must be
        // converted into NSArray for some attribute ID.
        if (!add_sequence_attribute(serviceInfo, AttributeId(key), dict))
            add_attribute(serviceInfo, AttributeId(key), dict);
    }

    if (serviceInfo.socketProtocol() == QBluetoothServiceInfo::L2capProtocol) {
        add_l2cap_protocol_descriptor_list(serviceInfo.protocolServiceMultiplexer(),
                                           dict);
    } else {
        add_rfcomm_protocol_descriptor_list(serviceInfo.serverChannel(), dict);
    }

    return dict;
}

}

QT_END_NAMESPACE
