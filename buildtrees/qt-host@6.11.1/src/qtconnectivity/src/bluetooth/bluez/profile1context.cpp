// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "profile1context_p.h"

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

OrgBluezProfile1ContextInterface::OrgBluezProfile1ContextInterface(QObject *parent) : QObject(parent)
{
}

void OrgBluezProfile1ContextInterface::NewConnection(const QDBusObjectPath &/*remotePath*/,
                                   const QDBusUnixFileDescriptor &descriptor,
                                   const QVariantMap &/*properties*/)
{
    qCDebug(QT_BT_BLUEZ) << "Profile Context: New Connection";
    emit newConnection(descriptor);
    setDelayedReply(false);
}

void OrgBluezProfile1ContextInterface::RequestDisconnection(const QDBusObjectPath &/*remotePath*/)
{
    qCDebug(QT_BT_BLUEZ) << "Profile Context: Request Disconnection";
    setDelayedReply(false);
}

void OrgBluezProfile1ContextInterface::Release()
{
    qCDebug(QT_BT_BLUEZ) << "Profile Context: Release";
}

QT_END_NAMESPACE

#include "moc_profile1context_p.cpp"
