// Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef VECTORCANBACKEND_P_H
#define VECTORCANBACKEND_P_H

#include "vectorcan_symbols_p.h"
#include "vectorcanbackend.h"

#if defined(Q_OS_WIN32)
#  include <qt_windows.h>
#else
#  error "Unsupported platform" // other stuff
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

class QWinEventNotifier;
class QTimer;

class VectorCanBackendPrivate
{
    Q_DECLARE_PUBLIC(VectorCanBackend)
public:
    VectorCanBackendPrivate(VectorCanBackend *q);
    ~VectorCanBackendPrivate();

    bool open();
    void close();
    bool setConfigurationParameter(QCanBusDevice::ConfigurationKey key, const QVariant &value);
    void setupChannel(const QString &interfaceName);
    void setupDefaultConfigurations();
    QString systemErrorString(int errorCode) const;
    void startWrite();
    void startRead();
    static XLstatus loadDriver();
    void startupDriver();
    static void cleanupDriver();
    bool setBitRate(quint32 bitrate);
    bool setDataBitRate(quint32 bitrate);

    VectorCanBackend * const q_ptr;

    bool transmitEcho = false;
    long portHandle = XL_INVALID_PORTHANDLE;
    quint64 channelMask = 0;
    HANDLE readHandle = INVALID_HANDLE_VALUE;
    QTimer *writeNotifier = nullptr;
    QWinEventNotifier *readNotifier = nullptr;
    quint32 dataBitRate = 0;
    quint32 arbBitRate = 0;
    int channelIndex = -1;
    bool usesCanFd = false;
};

QT_END_NAMESPACE

#endif // VECTORCANBACKEND_P_H
