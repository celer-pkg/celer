// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTSDPINQUIRY_H
#define BTSDPINQUIRY_H

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

#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

#include <QtCore/private/qglobal_p.h>
#include <QtCore/private/qcore_mac_p.h>
#include <QtCore/qlist.h>

#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>

@class QT_MANGLE_NAMESPACE(DarwinBTSDPInquiry);

QT_BEGIN_NAMESPACE

class QBluetoothServiceInfo;
class QVariant;

namespace DarwinBluetooth {

class SDPInquiryDelegate;

void extract_service_record(IOBluetoothSDPServiceRecord *record, QBluetoothServiceInfo &serviceInfo);
QVariant extract_attribute_value(IOBluetoothSDPDataElement *dataElement);
QList<QBluetoothUuid> extract_services_uuids(IOBluetoothDevice *device);

} // namespace DarwinBluetooth

QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(DarwinBTSDPInquiry) : NSObject

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth::SDPInquiryDelegate) *)aDelegate;
- (void)dealloc;

- (IOReturn)performSDPQueryWithDevice:(const QBluetoothAddress &)address;
- (IOReturn)performSDPQueryWithDevice:(const QBluetoothAddress &)address
                              filters:(const QList<QBluetoothUuid> &)filters;

- (void)stopSDPQuery;

- (void)sdpQueryComplete:(IOBluetoothDevice *)aDevice status:(IOReturn)status;

@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(DarwinBTSDPInquiry);

#endif
