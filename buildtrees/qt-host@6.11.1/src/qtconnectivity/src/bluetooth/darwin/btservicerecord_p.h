// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTSERVICERECORD_P_H
#define BTSERVICERECORD_P_H

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

#include "btutility_p.h"

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

QT_BEGIN_NAMESPACE

class QBluetoothServiceInfo;

namespace DarwinBluetooth {

ObjCStrongReference<NSMutableDictionary> iobluetooth_service_dictionary(const QBluetoothServiceInfo &serviceInfo);

} // namespace DarwinBluetooth

QT_END_NAMESPACE

#endif
