// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ROLES_H
#define ROLES_H

#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace Roles {
    Q_NAMESPACE
    enum SatelliteModelRoles {
        IdRole = Qt::UserRole + 1,
        RssiRole,
        AzimuthRole,
        ElevationRole,
        SystemRole,
        SystemIdRole,
        InUseRole,
        VisibleNameRole
    };
    Q_ENUM_NS(SatelliteModelRoles)
    QML_NAMED_ELEMENT(Roles)
}

#endif // ROLES_H
