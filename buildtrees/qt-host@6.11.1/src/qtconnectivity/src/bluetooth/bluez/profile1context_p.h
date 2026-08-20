// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PROFILECONTEXT_P_H
#define PROFILECONTEXT_P_H

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

#include <QtCore/qobject.h>
#include <QtDBus/qdbuscontext.h>
#include <QtDBus/qdbusextratypes.h>
#include <QtDBus/qdbusunixfiledescriptor.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class OrgBluezProfile1ContextInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Profile1")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.bluez.Profile1\">\n"
"    <method name=\"NewConnection\">\n"
"      <arg direction=\"in\" type=\"o\"/>\n"
"      <arg direction=\"in\" type=\"h\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In2\"/>\n"
"    </method>\n"
"    <method name=\"RequestDisconnection\">\n"
"      <arg direction=\"in\" type=\"o\"/>\n"
"    </method>\n"
"    <method name=\"Release\">\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    explicit OrgBluezProfile1ContextInterface(QObject *parent = nullptr);

Q_SIGNALS:
    void newConnection(const QDBusUnixFileDescriptor &fd);

public Q_SLOTS:
    void NewConnection(const QDBusObjectPath &, const QDBusUnixFileDescriptor &,
                       const QVariantMap &);
    void RequestDisconnection(const QDBusObjectPath &);
    Q_NOREPLY void Release();
};

QT_END_NAMESPACE

#endif // PROFILECONTEXT_P_H
