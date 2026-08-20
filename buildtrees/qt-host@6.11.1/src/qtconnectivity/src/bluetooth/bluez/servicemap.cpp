// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "servicemap_p.h"

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(ServiceMap)

const QDBusArgument &operator>>(const QDBusArgument &argument, ServiceMap &serviceMap)
{
    argument.beginMap();

    while (!argument.atEnd()) {
        quint32 uuid;
        QString service;

        argument.beginMapEntry();
        argument >> uuid;
        argument >> service;
        argument.endMapEntry();

        serviceMap.insert(uuid, service);
    }

    argument.endMap();

    return argument;
}

QT_END_NAMESPACE
