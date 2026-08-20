// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qremoteobjectregistry.h"
#include "qremoteobjectreplica_p.h"

#include <private/qobject_p.h>
#include <QtCore/qset.h>
#include <QtCore/qdatastream.h>

QT_BEGIN_NAMESPACE

class QRemoteObjectRegistryPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QRemoteObjectRegistry)

    QRemoteObjectSourceLocations sourceLocationsActualCalculation() const
    {
        return q_func()->propAsVariant(0).value<QRemoteObjectSourceLocations>();
    }
    Q_OBJECT_COMPUTED_PROPERTY(QRemoteObjectRegistryPrivate, QRemoteObjectSourceLocations,
                               sourceLocations,
                               &QRemoteObjectRegistryPrivate::sourceLocationsActualCalculation)
    QRemoteObjectSourceLocations hostedSources;
};

/*!
    \class QRemoteObjectRegistry
    \inmodule QtRemoteObjects
    \brief A class holding information about \l {Source} objects available on the Qt Remote Objects network.

    The Registry is a special Source/Replica pair held by a \l
    {QRemoteObjectNode} {node} itself. It knows about all other \l {Source}s
    available on the network, and simplifies the process of connecting to other
    \l {QRemoteObjectNode} {node}s.
*/
QRemoteObjectRegistry::QRemoteObjectRegistry(QObject *parent)
    : QRemoteObjectReplica(*new QRemoteObjectRegistryPrivate, parent)
{
    connect(this, &QRemoteObjectRegistry::stateChanged, this, &QRemoteObjectRegistry::pushToRegistryIfNeeded);
}

QRemoteObjectRegistry::QRemoteObjectRegistry(QRemoteObjectNode *node, const QString &name, QObject *parent)
    : QRemoteObjectReplica(*new QRemoteObjectRegistryPrivate, parent)
{
    connect(this, &QRemoteObjectRegistry::stateChanged, this, &QRemoteObjectRegistry::pushToRegistryIfNeeded);
    initializeNode(node, name);
}

/*!
    \fn void QRemoteObjectRegistry::remoteObjectAdded(const QRemoteObjectSourceLocation &entry)

    This signal is emitted whenever a new source location is added to the registry.

    \a entry is a QRemoteObjectSourceLocation, a typedef for
    QPair<QString, QRemoteObjectSourceLocationInfo>.

    \sa remoteObjectRemoved()
*/

/*!
    \fn void QRemoteObjectRegistry::remoteObjectRemoved(const QRemoteObjectSourceLocation &entry)

    This signal is emitted whenever a Source location is removed from the Registry.

    \a entry is a QRemoteObjectSourceLocation, a typedef for
    QPair<QString, QRemoteObjectSourceLocationInfo>.

    \sa remoteObjectAdded()
*/

/*!
    \property QRemoteObjectRegistry::sourceLocations
    \brief The set of sources known to the registry.

    This property is a QRemoteObjectSourceLocations, which is a typedef for
    QHash<QString, QRemoteObjectSourceLocationInfo>. Each known \l Source is
    contained as a QString key in the hash, and the corresponding value for
    that key is the QRemoteObjectSourceLocationInfo for the host node.
*/

/*!
    Destructor for QRemoteObjectRegistry.
*/
QRemoteObjectRegistry::~QRemoteObjectRegistry()
{}

void QRemoteObjectRegistry::registerMetatypes()
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;
    qRegisterMetaType<QRemoteObjectSourceLocation>();
    qRegisterMetaType<QRemoteObjectSourceLocations>();
}

void QRemoteObjectRegistry::initialize()
{
    QRemoteObjectRegistry::registerMetatypes();
    QVariantList properties;
    properties.reserve(3);
    properties << QVariant::fromValue(QRemoteObjectSourceLocations());
    properties << QVariant::fromValue(QRemoteObjectSourceLocation());
    properties << QVariant::fromValue(QRemoteObjectSourceLocation());
    setProperties(std::move(properties));
}

void QRemoteObjectRegistry::notifySourceLocationsChanged()
{
    d_func()->sourceLocations.notify();
}

/*!
    Returns a QRemoteObjectSourceLocations object, which includes the name
    and additional information of all sources known to the registry.
*/
QRemoteObjectSourceLocations QRemoteObjectRegistry::sourceLocations() const
{
    return d_func()->sourceLocations.value();
}

QBindable<QRemoteObjectSourceLocations> QRemoteObjectRegistry::bindableSourceLocations() const
{
    return &d_func()->sourceLocations;
}

/*!
    \internal
*/
void QRemoteObjectRegistry::addSource(const QRemoteObjectSourceLocation &entry)
{
    Q_D(QRemoteObjectRegistry);
    if (d->hostedSources.contains(entry.first)) {
        qCWarning(QT_REMOTEOBJECT) << "Node warning: ignoring source" << entry.first
                                   << "as this node already has a source by that name.";
        return;
    }
    d->hostedSources.insert(entry.first, entry.second);
    if (state() != QRemoteObjectReplica::State::Valid)
        return;

    if (sourceLocations().contains(entry.first)) {
        qCWarning(QT_REMOTEOBJECT) << "Node warning: ignoring source" << entry.first
                                   << "as another source (" << sourceLocations().value(entry.first)
                                   << ") has already registered that name.";
        return;
    }
    qCDebug(QT_REMOTEOBJECT) << "An entry was added to the registry - Sending to source" << entry.first << entry.second;
    // This does not set any data to avoid a coherency problem between client and server
    static int index = QRemoteObjectRegistry::staticMetaObject.indexOfMethod("addSource(QRemoteObjectSourceLocation)");
    QVariantList args;
    args << QVariant::fromValue(entry);
    send(QMetaObject::InvokeMetaMethod, index, args);
}

/*!
    \internal
*/
void QRemoteObjectRegistry::removeSource(const QRemoteObjectSourceLocation &entry)
{
    Q_D(QRemoteObjectRegistry);
    if (!d->hostedSources.contains(entry.first))
        return;

    d->hostedSources.remove(entry.first);
    if (state() != QRemoteObjectReplica::State::Valid)
        return;

    qCDebug(QT_REMOTEOBJECT) << "An entry was removed from the registry - Sending to source" << entry.first << entry.second;
    // This does not set any data to avoid a coherency problem between client and server
    static int index = QRemoteObjectRegistry::staticMetaObject.indexOfMethod("removeSource(QRemoteObjectSourceLocation)");
    QVariantList args;
    args << QVariant::fromValue(entry);
    send(QMetaObject::InvokeMetaMethod, index, args);
}

/*!
    \internal
    This internal function supports the edge case where the \l Registry
    is connected after \l Source objects are added to this \l Node, or
    the connection to the \l Registry is lost. When connected/reconnected, this
    function synchronizes local \l Source objects with the \l Registry.
*/
void QRemoteObjectRegistry::pushToRegistryIfNeeded()
{
    Q_D(QRemoteObjectRegistry);
    if (state() != QRemoteObjectReplica::State::Valid)
        return;

    if (d->hostedSources.isEmpty())
        return;

    const auto &sourceLocs = sourceLocations();
    for (auto it = d->hostedSources.begin(); it != d->hostedSources.end(); ) {
        const QString &loc = it.key();
        const auto sourceLocsIt = sourceLocs.constFind(loc);
        if (sourceLocsIt != sourceLocs.cend()) {
            qCWarning(QT_REMOTEOBJECT) << "Node warning: Ignoring Source" << loc << "as another source ("
                                       << sourceLocsIt.value() << ") has already registered that name.";
            it = d->hostedSources.erase(it);
        } else {
            static const int index = QRemoteObjectRegistry::staticMetaObject.indexOfMethod("addSource(QRemoteObjectSourceLocation)");
            QVariantList args{QVariant::fromValue(QRemoteObjectSourceLocation(loc, it.value()))};
            send(QMetaObject::InvokeMetaMethod, index, args);
            ++it;
        }
    }
}

QT_END_NAMESPACE
