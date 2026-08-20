// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 BasysKom GmbH.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QDBusMetaType>
#include "neard_helper_p.h"
#include "objectmanager_interface.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_NEARD)
Q_GLOBAL_STATIC(NeardHelper, neardHelper)

using namespace QtNfcPrivate; // for D-Bus wrappers

NeardHelper::NeardHelper(QObject *parent) :
    QObject(parent)
{
    qDBusRegisterMetaType<InterfaceList>();
    qDBusRegisterMetaType<ManagedObjectList>();

    m_dbusObjectManager = new OrgFreedesktopDBusObjectManagerInterface(QStringLiteral("org.neard"),
                                                                       QStringLiteral("/"),
                                                                       QDBusConnection::systemBus(),
                                                                       this);
    if (!m_dbusObjectManager->isValid()) {
        qCCritical(QT_NFC_NEARD) << "dbus object manager invalid";
        return;
    }

    connect(m_dbusObjectManager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
            this,                &NeardHelper::interfacesAdded);
    connect(m_dbusObjectManager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
            this,                &NeardHelper::interfacesRemoved);
}

NeardHelper *NeardHelper::instance()
{
    return neardHelper();
}

OrgFreedesktopDBusObjectManagerInterface *NeardHelper::dbusObjectManager()
{
    return m_dbusObjectManager;
}

void NeardHelper::interfacesAdded(const QDBusObjectPath &path, InterfaceList interfaceList)
{
    const QList<QString> keys = interfaceList.keys();
    for (const QString &key : keys) {
        if (key == QStringLiteral("org.neard.Tag")) {
            emit tagFound(path);
            break;
        }
        if (key == QStringLiteral("org.neard.Record")) {
            emit recordFound(path);
            break;
        }
    }
}

void NeardHelper::interfacesRemoved(const QDBusObjectPath &path, const QStringList &list)
{
    if (list.contains(QStringLiteral("org.neard.Record"))) {
        qCDebug(QT_NFC_NEARD) << "record removed" << path.path();
        emit recordRemoved(path);
    } else if (list.contains(QStringLiteral("org.neard.Tag"))) {
        qCDebug(QT_NFC_NEARD) << "tag removed" << path.path();
        emit tagRemoved(path);
    }
}

QT_END_NAMESPACE
