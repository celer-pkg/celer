// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QREMOTEOBJECTREGISTRY_P_H
#define QREMOTEOBJECTREGISTRY_P_H

#include <QtRemoteObjects/qremoteobjectreplica.h>

QT_BEGIN_NAMESPACE

class QRemoteObjectRegistryPrivate;
class QRemoteObjectNodePrivate;

class Q_REMOTEOBJECTS_EXPORT QRemoteObjectRegistry : public QRemoteObjectReplica
{
    Q_OBJECT
    Q_CLASSINFO(QCLASSINFO_REMOTEOBJECT_TYPE, "Registry")

    Q_PROPERTY(QRemoteObjectSourceLocations sourceLocations READ sourceLocations STORED false
               BINDABLE bindableSourceLocations)

public:
    ~QRemoteObjectRegistry() override;
    static void registerMetatypes();

    QRemoteObjectSourceLocations sourceLocations() const;
    QBindable<QRemoteObjectSourceLocations> bindableSourceLocations() const;

Q_SIGNALS:
    void remoteObjectAdded(const QRemoteObjectSourceLocation &entry);
    void remoteObjectRemoved(const QRemoteObjectSourceLocation &entry);

protected Q_SLOTS:
    void addSource(const QRemoteObjectSourceLocation &entry);
    void removeSource(const QRemoteObjectSourceLocation &entry);
    void pushToRegistryIfNeeded();

private:
    void initialize() override;
    void notifySourceLocationsChanged();

    explicit QRemoteObjectRegistry(QObject *parent = nullptr);
    explicit QRemoteObjectRegistry(QRemoteObjectNode *node, const QString &name, QObject *parent = nullptr);

    Q_DECLARE_PRIVATE(QRemoteObjectRegistry)
    friend class QT_PREPEND_NAMESPACE(QRemoteObjectNode);
    friend class QT_PREPEND_NAMESPACE(QRemoteObjectNodePrivate);
};

QT_END_NAMESPACE

#endif
