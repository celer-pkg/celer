// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BTTESTUTIL_P_H
#define BTTESTUTIL_P_H

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

#include <QtCore/qcoreapplication.h>

#ifdef Q_OS_ANDROID
#include <QtCore/QJniObject>
#endif

QT_BEGIN_NAMESPACE

bool androidBluetoothEmulator()
{
#ifdef Q_OS_ANDROID
    // QTBUG-106614, the Android-12+ emulator (API level 31+) emulates bluetooth.
    // We need to skip tests on the CI to avoid timeouts when Android waits for bluetooth
    // permission confirmation from the user. Currently the check below skips generally
    // on emulator though, not only on CI
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 31) {
        const auto property = QJniObject::fromString("ro.kernel.qemu");
        const char sysPropsClass[] = "android/os/SystemProperties";
        const auto isQemu = QJniObject::callStaticObjectMethod<jstring>(
                                        sysPropsClass, "get", property.object<jstring>());
        if (isQemu.toString() == "1")
            return true;
    }
#endif
    return false;
}

QT_END_NAMESPACE

#endif // BTTESTUTIL_P_H
