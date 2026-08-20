// Copyright (C) 2016 Centria research and development
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef ANDROIDJNINFC_H
#define ANDROIDJNINFC_H

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

#include "qglobal.h"

#include <QtCore/QJniObject>
#include <QtCore/qjnitypes.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_JNI_CLASS(QtNfc, "org/qtproject/qt/android/nfc/QtNfc")
Q_DECLARE_JNI_CLASS(QtNfcBroadcastReceiver, "org/qtproject/qt/android/nfc/QtNfcBroadcastReceiver")

Q_DECLARE_JNI_CLASS(NdefMessage, "android/nfc/NdefMessage")

namespace QtNfc {
bool startDiscovery();
bool stopDiscovery();
QtJniTypes::Intent getStartIntent();
bool isEnabled();
bool isSupported();
QtJniTypes::Parcelable getTag(const QtJniTypes::Intent &intent);
} // namespace QtNfc

QT_END_NAMESPACE

#endif
