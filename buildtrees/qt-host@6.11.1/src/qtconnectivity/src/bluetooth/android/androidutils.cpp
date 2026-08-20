// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "androidutils_p.h"
#include "jni_android_p.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/private/qandroidextras_p.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

bool ensureAndroidPermission(QBluetoothPermission::CommunicationModes modes)
{
    QBluetoothPermission permission;
    permission.setCommunicationModes(modes);

    if (qApp->checkPermission(permission) == Qt::PermissionStatus::Granted)
        return true;

    qCWarning(QT_BT_ANDROID) << "Permissions not authorized for a specified mode:" << modes;
    return false;
}

QJniObject getDefaultBluetoothAdapter()
{
    QJniObject service = QJniObject::getStaticField<QtJniTypes::AndroidContext, jstring>(
                                   "BLUETOOTH_SERVICE");
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    QJniObject manager =
            context.callMethod<jobject>("getSystemService", service.object<jstring>());
    QJniObject adapter;
    if (manager.isValid())
        adapter = manager.callMethod<QtJniTypes::BluetoothAdapter>("getAdapter");

    // ### Qt 7 check if the below double-get of the adapter can be removed.
    // It is a workaround for QTBUG-57489, fixed in 2016. According to the bug it occurred on
    // a certain device running Android 6.0.1 (Qt 6 supports Android 6.0 as the minimum).
    // For completeness: the original workaround was for the deprecated getDefaultAdapter()
    // method, and it is thus unclear if this is needed even in Qt 6 anymore. In addition the
    // impacted device is updateable to Android 8 which may also have fixed the issue.
    if (!adapter.isValid())
        adapter = manager.callMethod<QtJniTypes::BluetoothAdapter>("getAdapter");

    return adapter;
}

QT_END_NAMESPACE
