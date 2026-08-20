// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 BasysKom GmbH.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef NEARD_HELPER_P_H
#define NEARD_HELPER_P_H

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

#include "neard_dbus_types_p.h"

namespace QtNfcPrivate {

class OrgFreedesktopDBusObjectManagerInterface;

} // namespace QtNfcPrivate

QT_BEGIN_NAMESPACE

class NeardHelper : public QObject
{
    Q_OBJECT
public:
    NeardHelper(QObject* parent = 0);
    static NeardHelper *instance();

    QtNfcPrivate::OrgFreedesktopDBusObjectManagerInterface *dbusObjectManager();

signals:
    void tagFound(const QDBusObjectPath&);
    void tagRemoved(const QDBusObjectPath&);
    void recordFound(const QDBusObjectPath&);
    void recordRemoved(const QDBusObjectPath&);

private slots:
    void interfacesAdded(const QDBusObjectPath&, InterfaceList);
    void interfacesRemoved(const QDBusObjectPath&, const QStringList&);

private:
    QtNfcPrivate::OrgFreedesktopDBusObjectManagerInterface *m_dbusObjectManager;
};

QT_END_NAMESPACE

#endif // NEARD_HELPER_P_H
