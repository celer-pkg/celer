// Copyright (C) 2016 Centria research and development
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "androidjninfc_p.h"

#include <QCoreApplication>
#include <QtCore/qjnitypes.h>

QT_BEGIN_NAMESPACE

namespace QtNfc {

bool startDiscovery()
{
    return QtJniTypes::QtNfc::callStaticMethod<jboolean>("startDiscovery");
}

bool isEnabled()
{
    return QtJniTypes::QtNfc::callStaticMethod<jboolean>("isEnabled");
}

bool isSupported()
{
    return QtJniTypes::QtNfc::callStaticMethod<jboolean>("isSupported");
}

bool stopDiscovery()
{
    return QtJniTypes::QtNfc::callStaticMethod<jboolean>("stopDiscovery");
}

QtJniTypes::Intent getStartIntent()
{
    return QtJniTypes::QtNfc::callStaticMethod<QtJniTypes::Intent>("getStartIntent");
}

QtJniTypes::Parcelable getTag(const QtJniTypes::Intent &intent)
{
    return QtJniTypes::QtNfc::callStaticMethod<QtJniTypes::Parcelable>("getTag", intent);
}

} // namespace QtNfc

QT_END_NAMESPACE

extern "C" Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void * /*reserved*/)
{
    static bool initialized = false;
    if (initialized)
        return JNI_VERSION_1_6;
    initialized = true;

    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    const auto context = QNativeInterface::QAndroidApplication::context();
    QtJniTypes::QtNfc::callStaticMethod<void>("setContext", context);

    return JNI_VERSION_1_6;
}
