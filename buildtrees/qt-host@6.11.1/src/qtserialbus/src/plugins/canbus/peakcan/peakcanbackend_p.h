// Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef PEAKCANBACKEND_P_H
#define PEAKCANBACKEND_P_H

#include "peakcanbackend.h"
#include "peakcan_symbols_p.h"

#if defined(Q_OS_WIN32)
#  include <qt_windows.h>
#else
    // other stuff
#endif

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

QT_BEGIN_NAMESPACE

class QSocketNotifier;
class QWinEventNotifier;
class QTimer;

class PeakCanBackendPrivate
{
    Q_DECLARE_PUBLIC(PeakCanBackend)
public:
    PeakCanBackendPrivate(PeakCanBackend *q);

    bool open();
    void close();
    bool setConfigurationParameter(QCanBusDevice::ConfigurationKey key, const QVariant &value);
    void setupChannel(const QByteArray &interfaceName);
    void setupDefaultConfigurations();
    QString systemErrorString(TPCANStatus errorCode);
    void startWrite();
    void startRead();
    bool verifyBitRate(int bitrate);

    PeakCanBackend * const q_ptr;

    bool isFlexibleDatarateEnabled = false;
    bool isOpen = false;
    TPCANHandle channelIndex = PCAN_NONEBUS;
    QTimer *writeNotifier = nullptr;

#if defined(Q_OS_WIN32)
    QWinEventNotifier *readNotifier = nullptr;
    HANDLE readHandle  = INVALID_HANDLE_VALUE;
#else
    QSocketNotifier *readNotifier = nullptr;
    int readHandle = -1;
#endif
};

QT_END_NAMESPACE

#endif // PEAKCANBACKEND_P_H
