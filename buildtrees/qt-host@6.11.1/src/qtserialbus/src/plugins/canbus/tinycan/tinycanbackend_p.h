// Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef TINYCANBACKEND_P_H
#define TINYCANBACKEND_P_H

#include "tinycanbackend.h"
#include "tinycan_symbols_p.h"

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

class TinyCanBackendPrivate
{
    Q_DECLARE_PUBLIC(TinyCanBackend)
public:
    TinyCanBackendPrivate(TinyCanBackend *q);
    ~TinyCanBackendPrivate();

    bool open();
    void close();
    bool setConfigurationParameter(QCanBusDevice::ConfigurationKey key, const QVariant &value);

    QString systemErrorString(int errorCode);
    void setupChannel(const QString &interfaceName);
    void setupDefaultConfigurations();
    void startWrite();
    void startRead();
    void startupDriver();
    void cleanupDriver();
    void resetController();

    bool setBitRate(int bitrate);

    TinyCanBackend * const q_ptr;

    bool isOpen = false;
    int channelIndex = INDEX_INVALID;
    QTimer *writeNotifier = nullptr;
};

QT_END_NAMESPACE

#endif // TINYCANBACKEND_P_H
