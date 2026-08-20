// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTCPKEEPALIVECONFIGURATION_P_H
#define QTCPKEEPALIVECONFIGURATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the Network Access API.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/private/qglobal_p.h>
#include <QtNetwork/qtnetworkglobal.h>

#include <chrono>

QT_BEGIN_NAMESPACE

struct QTcpKeepAliveConfiguration
{
    std::chrono::duration<int> idleTimeBeforeProbes;
    std::chrono::duration<int> intervalBetweenProbes;
    int probeCount;

    bool isEqual(const QTcpKeepAliveConfiguration &other) const noexcept
    {
           return idleTimeBeforeProbes == other.idleTimeBeforeProbes
           && intervalBetweenProbes == other.intervalBetweenProbes
           && probeCount == other.probeCount;
    }

    friend bool operator==(const QTcpKeepAliveConfiguration &lhs, const QTcpKeepAliveConfiguration &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend bool operator!=(const QTcpKeepAliveConfiguration &lhs, const QTcpKeepAliveConfiguration &rhs) noexcept
    { return !lhs.isEqual(rhs); }

};

Q_DECLARE_TYPEINFO(QTcpKeepAliveConfiguration, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif // QTCPKEEPALIVECONFIGURATION_P_H
