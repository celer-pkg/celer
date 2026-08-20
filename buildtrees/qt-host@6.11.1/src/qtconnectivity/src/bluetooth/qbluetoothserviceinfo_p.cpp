// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothserviceinfo.h"
#include "qbluetoothserviceinfo_p.h"
#ifndef QT_IOS_BLUETOOTH
#include "dummy/dummy_helper_p.h"
#endif

QT_BEGIN_NAMESPACE

QBluetoothServiceInfoPrivate::QBluetoothServiceInfoPrivate()
{
#ifndef QT_IOS_BLUETOOTH
    printDummyWarning();
#endif
}

QBluetoothServiceInfoPrivate::~QBluetoothServiceInfoPrivate()
{
}

bool QBluetoothServiceInfoPrivate::isRegistered() const
{
    return false;
}

bool QBluetoothServiceInfoPrivate::registerService(const QBluetoothAddress &localAdapter)
{
    Q_UNUSED(localAdapter);
    return false;
}

bool QBluetoothServiceInfoPrivate::unregisterService()
{
    return false;
}

QT_END_NAMESPACE

#include "moc_qbluetoothserviceinfo_p.cpp"
