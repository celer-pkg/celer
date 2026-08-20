// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:potentially-allows-insecure-connections

#include "private/qmetaobjectbuilder_p.h"

#include "qremoteobjectcontainers_p.h"
#include "qremoteobjectnode.h"
#include "qremoteobjectnode_p.h"

#include "qremoteobjectregistry.h"
#include "qremoteobjectdynamicreplica.h"
#include "qremoteobjectpacket_p.h"
#include "qremoteobjectregistrysource_p.h"
#include "qremoteobjectreplica_p.h"
#include "qremoteobjectsource_p.h"
#include "qremoteobjectabstractitemmodelreplica_p.h"
#include "qremoteobjectabstractitemmodeladapter_p.h"
#include <QtCore/qabstractitemmodel.h>
#include <memory>
#include <algorithm>

QT_BEGIN_NAMESPACE

using namespace QtRemoteObjects;
using namespace QRemoteObjectStringLiterals;
using namespace QRemoteObjectInternalTypes;

using GadgetType = QVariantList;

// When dynamic types consisted only of PODs and QDynamicReplicas, we used two
// mechanisms for managing the memory allocated for these types.
//
// For POD types, we tracked memory using QObject pointers and the generated
// QMetaType.id() - see ManagedTypeEntry and s_trackedReferences.
//
// For QDynamicReplica types we leveraged QRemoteObjectMetaObjectManager members
// on Nodes that received the type information over the wire.
//
// Now that we can generate dynamic types for bindings to other languages, we need
// to track the memory of these types as well.  Here we modify (hack?) the
// ManagedTypeEntry mechanism.
// 1) While dynamic classes have QMetaObjects, they don't have associated
//    QMetaTypes.  So we create a global FakeClassIdManager that gives Qt reserved
//    ids to class type we can use just for tracking.
// 2) ManagedTypeEntry (which has been renamed from ManagedGadgetTypeEntry) now
//    stores an int id instead of a QMetaType, and for class types, the stored
//    QVariantList needed for PODs is unused.
class FakeClassIdManager {
public:
    int addTypeName(const QByteArray &typeName) {
        if (fakeClassIds.contains(typeName))
            return -1;
        int id = classId++;
        if (id >= QMetaType::User) {
            qWarning() << "Too many dynamic types registered, can't create more";
            return -1;
        }
        fakeClassIds[typeName] = id;
        return id;
    }

    int idForTypeName(const QByteArray &typeName) const {
        return fakeClassIds.value(typeName, -1);
    }

    bool removeTypeName(const QByteArray &typeName) {
        return fakeClassIds.remove(typeName) > 0;
    }

    bool removeTypeById(int id) {
        auto it = std::find_if(fakeClassIds.begin(), fakeClassIds.end(), [id](const auto &value) {
            return value == id;
        });
        if (it != fakeClassIds.end()) {
            fakeClassIds.erase(it);
            return true;
        }
        return false;
    }

private:
    QMap<QByteArray, int> fakeClassIds;
    int classId = 0;
};

Q_GLOBAL_STATIC(FakeClassIdManager, fakeClassIdManager)

struct ManagedTypeEntry
{
    GadgetType gadgetType;  // Optional now that this is used for dynamic types
    int id;                 // Real QMetaType.id() or fake id for dynamic classes for bindings
    QList<QMetaType> enumMetaTypes;
    QMetaObject *metaObject;

    void unregisterMetaTypes()
    {
        if (id >= QMetaType::User)
            QMetaType::unregisterMetaType(QMetaType(id));
        else
            fakeClassIdManager()->removeTypeById(id);
        for (auto enumMetaType : enumMetaTypes)
            QMetaType::unregisterMetaType(enumMetaType);
        free(metaObject);
    }
};

static QMutex s_managedTypesMutex;
static QHash<int, ManagedTypeEntry> s_managedTypes;
static QHash<int, QSet<QObject*>> s_trackedReferences;
static QLocalServer::SocketOptions s_localServerOptions = QLocalServer::NoOptions;

static void GadgetsStaticMetacallFunction(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::ReadProperty) {
        GadgetType *_t = reinterpret_cast<GadgetType *>(_o);
        if (_id < _t->size()) {
            const auto &prop = _t->at(_id);
            prop.metaType().destruct(_a[0]);
            prop.metaType().construct(_a[0], prop.constData());
        }
    } else if (_c == QMetaObject::WriteProperty) {
        GadgetType *_t = reinterpret_cast<GadgetType *>(_o);
        if (_id < _t->size()) {
            auto & prop = (*_t)[_id];
            prop = QVariant(prop.metaType(), _a[0]);
        }
    }
}

static void GadgetTypedDestructor(const QtPrivate::QMetaTypeInterface *, void *ptr)
{
    reinterpret_cast<GadgetType*>(ptr)->~GadgetType();
}

static void GadgetTypedConstructor(const QtPrivate::QMetaTypeInterface *interface, void *where)
{
    GadgetType *gadget = new(where) GadgetType;
    QMutexLocker lock(&s_managedTypesMutex);
    auto it = s_managedTypes.find(interface->typeId);
    if (it == s_managedTypes.end()) {
        delete gadget;
        gadget = nullptr;
        return;
    }
    *gadget = it->gadgetType;
}

static void GadgetTypedCopyConstructor(const QtPrivate::QMetaTypeInterface *, void *where, const void *copy)
{
    new(where) GadgetType(*reinterpret_cast<const GadgetType*>(copy));
}

static void GadgetTypedMoveConstructor(const QtPrivate::QMetaTypeInterface *, void *where, void *copy)
{
    new(where) GadgetType(std::move(*reinterpret_cast<GadgetType*>(copy)));
}

static bool GadgetEqualsFn(const QtPrivate::QMetaTypeInterface *, const void *a, const void *b)
{
    return *reinterpret_cast<const GadgetType*>(a) == *reinterpret_cast<const GadgetType*>(b);
}

static void GadgetDebugStreamFn(const QtPrivate::QMetaTypeInterface *, QDebug &dbg, const void *a)
{
    const GadgetType *gadgetProperties = reinterpret_cast<const GadgetType *>(a);
    for (const auto &prop : *gadgetProperties)
        dbg << prop;
}

static void GadgetDataStreamOutFn(const QtPrivate::QMetaTypeInterface *, QDataStream &ds, const void *a)
{
    const GadgetType *gadgetProperties = reinterpret_cast<const GadgetType *>(a);
    for (const auto &prop : *gadgetProperties)
        ds << prop;
}

static void GadgetDataStreamInFn(const QtPrivate::QMetaTypeInterface *, QDataStream &ds, void *a)
{
    GadgetType *gadgetProperties = reinterpret_cast<GadgetType *>(a);
    for (auto &prop : *gadgetProperties)
        ds >> prop;
}

// Like the Q_GADGET static methods above, we need constructor/destructor methods
// in order to use dynamically defined enums with QVariant or as signal/slot
// parameters (i.e., the queued connection mechanism, which QtRO leverages).
//
// We will need the enum methods to support different sizes when typed scope enum
// support is added, so might as well use that now.
template<typename T>
static void EnumDestructor(const QtPrivate::QMetaTypeInterface *, void *ptr)
{
    static_cast<T*>(ptr)->~T();
}

template<typename T>
static void EnumConstructor(const QtPrivate::QMetaTypeInterface *, void *where)
{
    new(where) T;
}

template<typename T>
static void EnumCopyConstructor(const QtPrivate::QMetaTypeInterface *, void *where, const void *copy)
{
    new(where) T(*static_cast<const T*>(copy));
}

template<typename T>
static void EnumMoveConstructor(const QtPrivate::QMetaTypeInterface *, void *where, void *copy)
{
    new(where) T(std::move(*static_cast<T*>(copy)));
}

// Not used, but keeping these in case we end up with a need for save/load.
template<typename T>
static void EnumSaveOperator(QDataStream & out, const void *data)
{
    const T value = *static_cast<const T *>(data);
    out << value;
}

template<typename T>
static void EnumLoadOperator(QDataStream &in, void *data)
{
    T value = *static_cast<T *>(data);
    in >> value;
}

template<typename T>
static bool EnumEqualsFn(const QtPrivate::QMetaTypeInterface *, const void *a, const void *b)
{
    return *static_cast<const T*>(a) == *static_cast<const T*>(b);
}

template<typename T>
static bool EnumLessThanFn(const QtPrivate::QMetaTypeInterface *, const void *a, const void *b)
{
    return *static_cast<const T*>(a) < *static_cast<const T*>(b);
}

template<typename T>
static void EnumDebugStreamFn(const QtPrivate::QMetaTypeInterface *, QDebug &dbg, const void *a)
{
    dbg << *static_cast<const T *>(a);
}

template<typename T>
static void EnumDataStreamOutFn(const QtPrivate::QMetaTypeInterface *, QDataStream &ds, const void *a)
{
    ds << *static_cast<const T *>(a);
}

template<typename T>
static void EnumDataStreamInFn(const QtPrivate::QMetaTypeInterface *, QDataStream &ds, void *a)
{
    ds >> *static_cast<T *>(a);
}

static QString name(const QMetaObject * const mobj)
{
    const int ind = mobj->indexOfClassInfo(QCLASSINFO_REMOTEOBJECT_TYPE);
    return ind >= 0 ? QString::fromLatin1(mobj->classInfo(ind).value()) : QString();
}

ClassData::ClassData(bool _isSource) : isSource(_isSource)
{
    baseMeta = _isSource ? &QObject::staticMetaObject : &QRemoteObjectReplica::staticMetaObject;
}

QString QtRemoteObjects::getTypeNameAndMetaobjectFromClassInfo(const QMetaObject *& meta) {
    QString typeName;
    const int ind = meta->indexOfClassInfo(QCLASSINFO_REMOTEOBJECT_TYPE);
    if (ind != -1) { //We have an object created from repc or at least with QCLASSINFO defined
        typeName = QString::fromLatin1(meta->classInfo(ind).value());
        while (true) {
            Q_ASSERT(meta->superClass());//This recurses to QObject, which doesn't have QCLASSINFO_REMOTEOBJECT_TYPE
            //At the point superclass doesn't have the same QCLASSINFO_REMOTEOBJECT_TYPE,
            //we have the metaobject we should work from
            if (ind != meta->superClass()->indexOfClassInfo(QCLASSINFO_REMOTEOBJECT_TYPE))
                break;
            meta = meta->superClass();
        }
    }
    return typeName;
}

template <typename K, typename V, typename Query>
bool map_contains(const QMap<K,V> &map, const Query &key, typename QMap<K,V>::const_iterator &result)
{
    const typename QMap<K,V>::const_iterator it = map.find(key);
    if (it == map.end())
        return false;
    result = it;
    return true;
}

/*!
    \qmltype Node
    \nativetype QRemoteObjectNode
    \inqmlmodule QtRemoteObjects
    \brief A node on a Qt Remote Objects network.

    The Node type provides an entry point to a Qt Remote Objects network. A network
    can be as simple as two nodes, or an arbitrarily complex set of processes and devices.

    A Node does not have a url that other nodes can connect to, and thus is able to acquire
    replicas only. It is not able to share source objects.

*/
/*!
    \class QRemoteObjectRegistryHostPrivate
    \inmodule QtRemoteObjects
    \internal
*/

QRemoteObjectNodePrivate::QRemoteObjectNodePrivate()
    : QObjectPrivate()
    , registry(nullptr)
    , retryInterval(250)
    , lastError(QRemoteObjectNode::NoError)
    , persistedStore(nullptr)
{ }

QRemoteObjectNodePrivate::~QRemoteObjectNodePrivate()
{
}

QRemoteObjectSourceLocations QRemoteObjectNodePrivate::remoteObjectAddresses() const
{
    if (registry)
        return registry->sourceLocations();
    return QRemoteObjectSourceLocations();
}

QRemoteObjectSourceLocations QRemoteObjectRegistryHostPrivate::remoteObjectAddresses() const
{
    if (registrySource)
        return registrySource->sourceLocations();
    return QRemoteObjectSourceLocations();
}

/*!
    \reimp
*/
void QRemoteObjectNode::timerEvent(QTimerEvent*)
{
    Q_D(QRemoteObjectNode);

    for (auto it = d->pendingReconnect.begin(), end = d->pendingReconnect.end(); it != end; /*erasing*/) {
        const auto &conn = *it;
        if (conn->isOpen()) {
            it = d->pendingReconnect.erase(it);
        } else {
            conn->connectToServer();
            ++it;
        }
    }

    if (d->pendingReconnect.isEmpty())
        d->reconnectTimer.stop();

    qRODebug(this) << "timerEvent" << d->pendingReconnect.size();
}

/*!
    \qmlproperty int Node::heartbeatInterval

    Heartbeat interval in ms.

    The heartbeat (only helpful for socket connections) will periodically send a
    message to connected nodes to detect whether the connection was disrupted.
    Qt Remote Objects will try to reconnect automatically if it detects a dropped
    connection. This function can help with that detection since the client will
    only detect that the server is unavailable when it tries to send data.

    A value of \c 0 (the default) will disable the heartbeat.
*/


/*!
    \property QRemoteObjectNode::heartbeatInterval
    \brief Heartbeat interval in ms.

    The heartbeat (only helpful for socket connections) will periodically send a
    message to connected nodes to detect whether the connection was disrupted.
    Qt Remote Objects will try to reconnect automatically if it detects a dropped
    connection. This function can help with that detection since the client will
    only detect that the server is unavailable when it tries to send data.

    A value of \c 0 (the default) will disable the heartbeat.
*/
int QRemoteObjectNode::heartbeatInterval() const
{
    Q_D(const QRemoteObjectNode);
    return d->m_heartbeatInterval;
}

void QRemoteObjectNode::setHeartbeatInterval(int interval)
{
    Q_D(QRemoteObjectNode);
    if (d->m_heartbeatInterval == interval)
        return;
    d->m_heartbeatInterval = interval;
    emit heartbeatIntervalChanged(interval);
}

/*!
    \since 5.12
    \typedef QRemoteObjectNode::RemoteObjectSchemaHandler

    Typedef for a std::function method that can take a QUrl input and is
    responsible for creating the communications channel between this node and
    the node hosting the desired \l Source. As some types of QIODevices (e.g.,
    QSslSocket) require additional steps before the device is ready for use,
    the method is responsible for calling \l addClientSideConnection once the
    connection is fully established.
*/

/*!
    \since 5.12
    \brief Provide a custom method to handle externally provided schemas

    This method is tied to the \l Registry and \l {External Schemas}. By
    registering a std::function handler for an external schema, the registered
    method will be called when the registry is notified of a \l Source you've
    acquired being available. Without this registration, QtRO would only be
    able to handle the "built-in" schemas.

    The provided method, \a handler, will be called when the registry sees a \l
    Source object on a new (not yet connected) Node with a {QUrl::schema()} of
    \a schema. The \a handler, of type \l
    QRemoteObjectNode::RemoteObjectSchemaHandler will get the \l QUrl of the
    Node providing the \l Source as an input parameter, and is responsible for
    establishing the communications channel (a \l QIODevice of some sort) and
    calling \l addClientSideConnection with it.

    \sa RemoteObjectSchemaHandler
*/
void QRemoteObjectNode::registerExternalSchema(const QString &schema, QRemoteObjectNode::RemoteObjectSchemaHandler handler)
{
    Q_D(QRemoteObjectNode);
    d->schemaHandlers.insert(schema, handler);
}

/*!
    \since 5.11
    \brief Forward Remote Objects from another network

    The proxy functionality is useful when you want to share \l Source objects
    over multiple networks. For instance, if you have an embedded target using
    target-only connections (like local) and you want to make some of those
    same objects available externally.

    As a concrete example, say you have a set of processes talking to each
    other on your target hardware using a registry, with the \l Registry at
    "local:registry" and separate processes using a node at "local:MyHost" that
    holds \l Source objects. If you wanted to access these objects, but over
    tcp, you could create a new proxyNode like so:

\code
    // myInternalHost is a node only visible on the device...
    QRemoteObjectHost myInternalHost("local:MyHost");
    myInternalHost.enableRemoting<SomeObject>(&someObject);

    // Regular host node, listening on port 12123, so visible to other
    // devices
    QRemoteObjectHost proxyNode("tcp://localhost:12123");

    // Enable proxying objects from nodes on the local machine's internal
    // QtRO bus
    proxyNode.proxy("local:registry");
\endcode

    And from another device you create another node:

\code
    // NB: localhost resolves to a different ip address than proxyNode
    QRemoteObjectHost nodeOnRemoteDevice("tcp://localhost:23234");

    // Connect to the target's proxyNode directly, or use a tcp registry...
    nodeOnRemoteDevice.connectToNode("tcp://<target device>:12123");

    // Because of the proxy, we can get the object over tcp/ip port 12123,
    // even though we can't connect directly to "local:MyHost"
    SomeObject *so = nodeOnRemoteDevice.acquire<SomeObject>();
\endcode

    This would (internally) create a node in proxyNode, which (again
    internally/automatically) connects to the provided registry (given by the
    \a registryUrl parameter, "local:registry" in this example). Whenever
    local:registry emits the \l remoteObjectAdded signal, the
    \c QRemoteObjectSourceLocation is passed to the \a filter given to the proxy
    call. If this method returns true (the default filter simply returns true
    without any filtering), the object is acquired() from the internal node and
    enableRemoting() (once the replica is initialized) is called on proxyNode.

    If a \a hostUrl is provided (which is required to enable reverseProxy, but
    not needed otherwise), the internal node will be a \l QRemoteObjectHost node
    configured with the provided address. If no \a hostUrl is provided, the
    internal node will be a QRemoteObjectNode (not HostNode).

    Returns \c true if the object is acquired from the internal node.

    \sa reverseProxy()
*/
bool QRemoteObjectHostBase::proxy(const QUrl &registryUrl, const QUrl &hostUrl, RemoteObjectNameFilter filter)
{
    Q_D(QRemoteObjectHostBase);
    if (!registryUrl.isValid() || !QtROClientFactory::instance()->isValid(registryUrl)) {
        qROWarning(this) << "Can't proxy to registryUrl (invalid url or schema)" << registryUrl;
        return false;
    }

    if (!hostUrl.isEmpty() && !QtROClientFactory::instance()->isValid(hostUrl)) {
        qROWarning(this) << "Can't proxy using hostUrl (invalid schema)" << hostUrl;
        return false;
    }

    if (d->proxyInfo) {
        qROWarning(this) << "Proxying from multiple objects is currently not supported.";
        return false;
    }

    QRemoteObjectNode *node;
    if (hostUrl.isEmpty()) {
        node = new QRemoteObjectNode(registryUrl);
    } else {
        node = new QRemoteObjectHost(hostUrl, registryUrl);
    }
    d->proxyInfo = new ProxyInfo(node, this, filter);
    return true;
}

/*!
    \since 5.11
    \brief Forwards remote objects to another network.

    The reverseProxy() function allows the \l proxy() functionality to be
    extended, in effect mirroring the proxy functionality in the "reverse"
    direction. These are distinct, because node communication is not symmetric,
    one side calls enableRemoting() with a \l Source object, the other side
    calls acquire() to get a \l Replica. Using \l proxy() allows you to
    "observe" objects on a target device remotely via acquire, but it does not
    allow off-target \l Source objects to be acquired from the device's local:*
    network. That is where \l reverseProxy() comes in. If a proxyNode is
    created like so:

\code
    // myInternalHost is a node only visible on the device...
    QRemoteObjectHost myInternalHost("local:MyHost", "local:registry");

    // RegistryHost node, listening on port 12123, so visible to other
    // devices.  The node must be a RegistryHost, so the Sources on
    // the "outside" network can be forwarded to the inner network.
    QRemoteObjectRegistryHost proxyNode("tcp://0.0.0.0:12123");

    // Enable proxying objects from nodes on the local machine's internal
    // QtRO bus.  Note the hostUrl parameter is now needed.
    proxyNode.proxy("local:registry", "local:fromProxy");
    proxyNode.reverseProxy();
\endcode

    And from another device you create another node:

\code
    // Listen on a local port, and connect to "proxyNode" as the registry.
    // NB: localhost resolves to a different ip address than proxyNode
    QRemoteObjectHost nodeOnRemoteDevice("tcp://localhost:23234",
                                         "tcp://<target device>:12123");

    // Because of the reverseProxy, we can expose objects on this device
    // and they will make their way to proxyNode...
    nodeOnRemoteDevice.enableRemoting<OtherObject>(&otherObject);
\endcode

\code
    // Acquire() can now see the objects on other devices through proxyNode,
    // due to the reverseProxy call.
    OtherObject *oo = myInternalHost.acquire<OtherObject>();
\endcode

    While the \l proxy() functionality allows \l Source objects on another
    network to be acquired(), reverseProxy() allows \l Source objects to be
    "pushed" to an otherwise inaccessible network.

    \note proxy() needs to be called before \l reverseProxy(), and a
    hostUrl needs to be provided to \l proxy for \l reverseProxy() to work. The
    \l reverseProxy() method allows a separate \a filter to be applied. This
    reverseProxy specific filter will receive notifications of new \l Source
    objects on \c proxyNode and acquire them on the internal node if they pass the
    \a filter.

    Returns \c true on success, \c false otherwise.

    \note Currently the reverse proxy functionality is supported only for
    QRemoteObjectRegistryHost. Calling this method on a QRemoteObjectHost node
    will always return \c false.

    \sa proxy()
*/
bool QRemoteObjectHostBase::reverseProxy(QRemoteObjectHostBase::RemoteObjectNameFilter filter)
{
    Q_D(QRemoteObjectHostBase);

    if (!d->proxyInfo) {
        qROWarning(this) << "proxy() needs to be called before setting up reverse proxy.";
        return false;
    }

    QRemoteObjectHost *host = qobject_cast<QRemoteObjectHost *>(d->proxyInfo->proxyNode);
    if (!host) {
        qROWarning(this) << "proxy() needs called with host-url to enable reverse proxy.";
        return false;
    }

    return d->proxyInfo->setReverseProxy(filter);
}

/*!
    \internal The replica needs to have a default constructor to be able
    to create a replica from QML.  In order for it to be properly
    constructed, there needs to be a way to associate the replica with a
    node and start the replica initialization.  Thus we need a public
    method on node to facilitate that.  That's initializeReplica.
*/
void QRemoteObjectNode::initializeReplica(QRemoteObjectReplica *instance, const QString &name)
{
    Q_D(QRemoteObjectNode);
    if (instance->inherits("QRemoteObjectDynamicReplica")) {
        d->setReplicaImplementation(nullptr, instance, name);
    } else {
        const QMetaObject *meta = instance->metaObject();
        // This is a templated acquire, so we tell the Source we don't need
        // them to send the class definition.  Thus we need to store the
        // metaObject for this class - if this is a nested class, the QObject
        // could be a nullptr or updated from the source,
        d->dynamicTypeManager.addFromMetaObject(meta);
        d->setReplicaImplementation(meta, instance, name.isEmpty() ? ::name(meta) : name);
    }
}

void QRemoteObjectNodePrivate::setLastError(QRemoteObjectNode::ErrorCode errorCode)
{
    Q_Q(QRemoteObjectNode);
    lastError = errorCode;
    emit q->error(lastError);
}

void QRemoteObjectNodePrivate::setReplicaImplementation(const QMetaObject *meta, QRemoteObjectReplica *instance, const QString &name)
{
    qROPrivDebug() << "Starting setReplicaImplementation for" << name;
    openConnectionIfNeeded(name);
    QMutexLocker locker(&mutex);
    if (hasInstance(name)) {
        qCDebug(QT_REMOTEOBJECT)<<"setReplicaImplementation - using existing instance";
        QSharedPointer<QRemoteObjectReplicaImplementation> rep = qSharedPointerCast<QRemoteObjectReplicaImplementation>(replicas.value(name).toStrongRef());
        Q_ASSERT(rep);
        instance->d_impl = rep;
        rep->configurePrivate(instance);
    } else {
        instance->d_impl.reset(handleNewAcquire(meta, instance, name));
        instance->initialize();
        replicas.insert(name, instance->d_impl.toWeakRef());
        qROPrivDebug() << "setReplicaImplementation - Created new instance" << name<<remoteObjectAddresses();
    }
}

/*!
    Returns a pointer to the Node's \l {QRemoteObjectRegistry}, if the Node
    is using the Registry feature; otherwise it returns \nullptr.
*/
const QRemoteObjectRegistry *QRemoteObjectNode::registry() const
{
    Q_D(const QRemoteObjectNode);
    return d->registry;
}

/*!
    \class QRemoteObjectAbstractPersistedStore
    \inmodule QtRemoteObjects
    \brief A class which provides the methods for setting PROP values of a
    replica to value they had the last time the replica was used.

    This can be used to provide a "reasonable" value to be displayed until the
    connection to the source is established and current values are available.

    This class must be overridden to provide an implementation for saving (\l
    QRemoteObjectAbstractPersistedStore::saveProperties) and restoring (\l
    QRemoteObjectAbstractPersistedStore::restoreProperties) PROP values. The derived
    type can then be set for a node, and any replica acquired from that node
    will then automatically store PERSISTED properties when the replica
    destructor is called, and retrieve the values when the replica is
    instantiated.
*/

/*!
    Constructs a QRemoteObjectAbstractPersistedStore with the given \a parent.
    The default value of \a parent is \nullptr.
*/
QRemoteObjectAbstractPersistedStore::QRemoteObjectAbstractPersistedStore(QObject *parent)
    : QObject(parent)
{
}

QRemoteObjectAbstractPersistedStore::~QRemoteObjectAbstractPersistedStore()
{
}

/*!
    \fn virtual void QRemoteObjectAbstractPersistedStore::saveProperties(const QString &repName, const QByteArray &repSig, const QVariantList &values)

    This method will be provided the replica class's \a repName, \a repSig and the list of
    \a values that PERSISTED properties have when the replica destructor was
    called. It is the responsibility of the inheriting class to store the
    information in a manner consistent for \l
    QRemoteObjectAbstractPersistedStore::restoreProperties to retrieve.

    \sa QRemoteObjectAbstractPersistedStore::restoreProperties
*/

/*!
    \fn virtual QVariantList QRemoteObjectAbstractPersistedStore::restoreProperties(const QString &repName, const QByteArray &repSig)

    This method will be provided the replica class's \a repName and \a repSig when the
    replica is being initialized. It is the responsibility of the inheriting
    class to get the last values persisted by \l
    QRemoteObjectAbstractPersistedStore::saveProperties and return them. An empty
    QVariantList should be returned if no values are available.

    \sa QRemoteObjectAbstractPersistedStore::saveProperties
*/


QRemoteObjectAbstractPersistedStore *QRemoteObjectNode::persistedStore() const
{
    Q_D(const QRemoteObjectNode);
    return d->persistedStore;
}

/*!
    \qmlproperty QRemoteObjectAbstractPersistedStore Node::persistedStore

    Allows setting a \l QRemoteObjectAbstractPersistedStore instance for the node.

    Allows replica \l PROP members with the PERSISTED trait to save their current value when the
    replica is deleted and restore a stored value the next time the replica is started.

    Requires a \l QRemoteObjectAbstractPersistedStore class implementation to control where and how
    persistence is handled. A default QSettings-based implementation is provided by SettingsStore.
*/

/*!
    \since 5.11
    \property QRemoteObjectNode::persistedStore
    \brief Allows setting a \l QRemoteObjectAbstractPersistedStore instance for the node.

    Allows replica \l PROP members with the PERSISTED trait to save their current value when the
    replica is deleted and restore a stored value the next time the replica is started.

    Requires a \l QRemoteObjectAbstractPersistedStore class implementation to control where and how
    persistence is handled.
*/
void QRemoteObjectNode::setPersistedStore(QRemoteObjectAbstractPersistedStore *persistedStore)
{
    Q_D(QRemoteObjectNode);
    d->persistedStore = persistedStore;
}

QRemoteObjectAbstractPersistedStore::QRemoteObjectAbstractPersistedStore(QRemoteObjectAbstractPersistedStorePrivate &dptr, QObject *parent)
    : QObject(dptr, parent)
{
}

QRemoteObjectAbstractPersistedStorePrivate::QRemoteObjectAbstractPersistedStorePrivate()
{
}

QRemoteObjectAbstractPersistedStorePrivate::~QRemoteObjectAbstractPersistedStorePrivate()
{
}

QRemoteObjectMetaObjectManager::~QRemoteObjectMetaObjectManager()
{
    for (QMetaObject *mo : dynamicTypes) {
        for (auto metaType : enumTypes[mo])
            QMetaType::unregisterMetaType(metaType);
        enumTypes.remove(mo);
        free(mo); //QMetaObjectBuilder uses malloc, not new
    }
}

const QMetaObject *QRemoteObjectMetaObjectManager::metaObjectForType(const QString &type)
{
    qCDebug(QT_REMOTEOBJECT) << "metaObjectForType: looking for" << type << "static keys:" << staticTypes.keys() << "dynamic keys:" << dynamicTypes.keys();
    Q_ASSERT(staticTypes.contains(type) || dynamicTypes.contains(type));
    auto it = staticTypes.constFind(type);
    if (it != staticTypes.constEnd())
        return it.value();
    return dynamicTypes.value(type);
}

static void trackReference(int typeId, QObject *reference)
{
    QMutexLocker lock(&s_managedTypesMutex);
    if (s_trackedReferences[typeId].contains(reference))
        return;
    s_trackedReferences[typeId].insert(reference);
    auto unregisterIfNotUsed = [typeId, reference]{
        QMutexLocker lock(&s_managedTypesMutex);
        Q_ASSERT(s_trackedReferences.contains(typeId));
        Q_ASSERT(s_trackedReferences[typeId].contains(reference));
        s_trackedReferences[typeId].remove(reference);
        if (s_trackedReferences[typeId].isEmpty()) {
            s_trackedReferences.remove(typeId);
            s_managedTypes[typeId].unregisterMetaTypes();
            s_managedTypes.remove(typeId); // Destroys the meta types
        }
    };

    // Unregister the type only when the reference is destroyed.  Original
    // behavior was for the reference to be a connection (derived from
    // QtROIoDeviceBase), but we want to allow language bindings so we
    // now allow for any arbitrary QObject pointer to be used.
    // If the reference is a connection, we do not want to unregister types
    // when the connection is disconnected, because if it gets reconnected
    // it will not register the types again.
    QObject::connect(reference, &QObject::destroyed, reference, unregisterIfNotUsed);
}

bool trackAdditionalReference(QObject *reference, const QByteArray &typeName)
{
    // Determine if the type is a POD or Class.  PODs will have a QMetaType
    int id = -1;
    auto metaType = QMetaType::fromName(typeName);
    if (metaType.isValid())
        id = metaType.id();
    else
        id = fakeClassIdManager->idForTypeName(typeName);

    if (id < 0)
        return false;

    trackReference(id, reference);
    return true;
}

static const char *strDup(const QByteArray &s)
{
    auto result = new char[uint(s.size()) + 1];
    auto end = std::copy(s.cbegin(), s.cend(), result);
    *end = 0;
    return result;
}

using Gadgets = QHash<QByteArray, GadgetData>;
// Registering types uses the QMetaTypeInterface struct.  This struct includes a function to let
// QMetaType::metaObject() return the associated QMetaObject for enums and gadgets, but since we
// are creating the enums dynamically with the gadgets/QObject classes, we need to
//   a. Create the TypeInfo structs and register them
//   b. Use QMetaObjectBuilder to add all replica types (which looks up QMetaType(s) by name)
//   c. Create the QMetaObject from the builder
//   d. Go back and assign the QMetaObject to the TypeInfo for future lookup
// All of this requires a new QMetaTypeInterface with a spot to hold a pointer to the created
// QMetaObject.
struct TypeInfo : public QtPrivate::QMetaTypeInterface
{
    const QMetaObject *metaObject;
};
static const QMetaObject *metaObjectFn(const QtPrivate::QMetaTypeInterface *self)
{
    return static_cast<const TypeInfo *>(self)->metaObject;
}

template <class Int>
static TypeInfo *enumMetaType(const QByteArray &name, uint size, const QMetaObject *meta=nullptr)
{
    static const auto flags = QMetaType::IsEnumeration | QMetaType::NeedsConstruction
                              | QMetaType::NeedsDestruction;

    auto typeInfo = new TypeInfo {
        {
            0, alignof(Int), size, uint(flags), 0, metaObjectFn, strDup(name),
            EnumConstructor<Int>,
            EnumCopyConstructor<Int>,
            EnumMoveConstructor<Int>,
            EnumDestructor<Int>,
            EnumEqualsFn<Int>,
            EnumLessThanFn<Int>,
            EnumDebugStreamFn<Int>,
            EnumDataStreamOutFn<Int>,
            EnumDataStreamInFn<Int>,
            nullptr
        }, meta
    };
    return typeInfo;
}

static TypeInfo *registerEnum(const QByteArray &name, uint size=4u)
{
    TypeInfo *result = nullptr;
    if (QMetaType::fromName(name).isValid())
        return result;
    switch (size) {
    case 1:
        result = enumMetaType<qint8>(name, size);
        break;
    case 2:
        result = enumMetaType<qint16>(name, size);
        break;
    case 4:
        result = enumMetaType<qint32>(name, size);
        break;
    // Qt currently only supports enum values of 4 or less bytes (QMetaEnum value(index) returns int)
//    case 8: id = QMetaType::registerType(name.constData(), nullptr, nullptr, &EnumDestructor<qint64>,
//                                                 &EnumConstructor<qint64>, size, flags, meta);
//        break;
    default:
        qWarning() << "Invalid enum detected" << name << "with size" << size << ".  Defaulting to register as int.";
        size = 4;
        result = enumMetaType<qint32>(name, size);
        break;
    }
#ifdef QTRO_VERBOSE_PROTOCOL
    qDebug() << "Registering new enum" << name << "size:" << size;
#endif
    return result;
}

static std::pair<QMap<QByteArray, QMetaType>, QList<TypeInfo *>>
handleEnums(QMetaObjectBuilder &builder, const QList<EnumData> &enumList, const QByteArray &type)
{
    QList<TypeInfo *> enumsToBeAssignedMetaObject;
    QMap<QByteArray, QMetaType> enumLookup;
    enumsToBeAssignedMetaObject.reserve(enumList.size());
    for (const auto &enumData : enumList) {
        const QByteArray registeredName = type + "::" + enumData.name;
        auto typeInfo = registerEnum(registeredName, enumData.size);
        QMetaType metaType;
        if (typeInfo) {
            enumsToBeAssignedMetaObject.append(typeInfo);
            metaType = QMetaType(typeInfo);
            metaType.registerType();
            enumLookup[enumData.name] = metaType;
            qCDebug(QT_REMOTEOBJECT) << "Registering new gadget enum with id" << metaType.id() << typeInfo->name << "size:" << typeInfo->size;
        } else if (QMetaType::fromName(registeredName).isValid()) {
            qWarning() << "Failed to register enum" << registeredName << "(this name is already in use).";
            continue;
        }
        auto enumBuilder = builder.addEnumerator(enumData.name);
        enumBuilder.setIsFlag(enumData.isFlag);
        enumBuilder.setIsScoped(enumData.isScoped);
        enumBuilder.setMetaType(metaType);

        for (quint32 k = 0; k < enumData.keyCount; ++k) {
            const auto pair = enumData.values.at(k);
            enumBuilder.addKey(pair.name, pair.value);
        }
    }
    return {enumLookup, enumsToBeAssignedMetaObject};
}

QMetaObject *registerGadget(QObject *reference, const GadgetData &gadget, QByteArray typeName)
{
    ManagedTypeEntry entry;

    QMetaObjectBuilder gadgetBuilder;
    gadgetBuilder.setClassName(typeName);
    gadgetBuilder.setFlags(PropertyAccessInStaticMetaCall);

    auto [enumLookup, enumsToBeAssignedMetaObject] = handleEnums(gadgetBuilder, gadget.enums, typeName);
    for (auto metaType : enumLookup)
        entry.enumMetaTypes.append(metaType);

    for (const auto &prop : gadget.properties) {
        int propertyType = QMetaType::fromName(prop.type).id();
        if (!propertyType && enumLookup.contains(prop.type))
            propertyType = enumLookup[prop.type].id();
        auto propertyMetaType = QMetaType(propertyType);
        entry.gadgetType.push_back(QVariant(propertyMetaType));
        auto dynamicProperty = gadgetBuilder.addProperty(prop.name, {propertyMetaType.name()});
        dynamicProperty.setWritable(true);
        dynamicProperty.setReadable(true);
    }

    auto meta = gadgetBuilder.toMetaObject();
    entry.metaObject = meta;
    for (auto typeInfo : enumsToBeAssignedMetaObject)
        typeInfo->metaObject = meta;

    QMetaType::TypeFlags flags = QMetaType::IsGadget;
    if (meta->propertyCount()) {
        meta->d.static_metacall = &GadgetsStaticMetacallFunction;
        meta->d.superdata = nullptr;
        flags |= QMetaType::NeedsConstruction | QMetaType::NeedsDestruction;
        auto typeInfo = new TypeInfo {
            {
                0, alignof(GadgetType), sizeof(GadgetType), uint(flags), 0, metaObjectFn,
                strDup(typeName),
                GadgetTypedConstructor,
                GadgetTypedCopyConstructor,
                GadgetTypedMoveConstructor,
                GadgetTypedDestructor,
                GadgetEqualsFn,
                nullptr, /* LessThanFn */
                GadgetDebugStreamFn,
                GadgetDataStreamOutFn,
                GadgetDataStreamInFn,
                nullptr /* LegacyRegisterOp */
            },
            meta
        };
        entry.id = QMetaType(typeInfo).id();
    } else {
        auto typeInfo = new TypeInfo {
            {
                0, alignof(GadgetType), sizeof(GadgetType), uint(flags), 0, metaObjectFn,
                strDup(typeName),
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            },
            meta
        };
        entry.id = QMetaType(typeInfo).id();
    }
    QMetaType(entry.id).registerType();
    if (reference)
        trackReference(entry.id, reference);
    QMutexLocker lock(&s_managedTypesMutex);
    s_managedTypes.insert(entry.id, entry);
    return meta;
}

static void registerGadgets(QObject *reference, Gadgets &gadgets, QByteArray typeName)
{
    // Ideally this would register a single gadget, but gadgets can depend on other gadgets.
    // To account for this, the registerGadgets method can recurse, removing one Gadget at a
    // time from the input list (which is passed in by non-const reference).

    // If the type is already registered, just track the new reference and return
    const auto &gadget = gadgets.take(typeName);
    int typeId = QMetaType::fromName(typeName).id();
    if (typeId != QMetaType::UnknownType) {
        if (reference)
            trackReference(typeId, reference);
        return;
    }

    // Loop through the properties for dependent gadgets to register
    for (const auto &prop : gadget.properties) {
        int propertyType = QMetaType::fromName(prop.type).id();
        if (!propertyType && gadgets.contains(prop.type))
            registerGadgets(reference, gadgets, prop.type);
    }

    // Once there are no dependencies to address, add this type
    registerGadget(reference, gadget, std::move(typeName));
}

static void registerAllGadgets(QObject *reference, Gadgets &gadgets)
{
    while (!gadgets.isEmpty())
        registerGadgets(reference, gadgets, gadgets.constBegin().key());
}

static std::pair<QMetaObject *, QList<QMetaType>>
registerDefinition(const ClassData &data)
{
    // This code was originally written to support dynamic Replica types, with no expectation
    // that eventually dynamic sources for bindings to other languages would be added.
    // Because it was only expected to support Replica types, the code only used the Rep type
    // name for the name of the class, so we have "Simple" instead of "SimpleReplica".  Now
    // that dynamic Sources are being added, we will have registration issues unless we have
    // distinct names for Replicas and Sources.  To not break existing code, we will append
    // "Source" to the type name if it is a Source, but leave Replica class names alone.
    auto type = data.type + (data.isSource ? "Source" : "");
    QMetaObjectBuilder builder;
    builder.setSuperClass(data.baseMeta);

    builder.addClassInfo(QCLASSINFO_REMOTEOBJECT_TYPE, data.type);
    builder.setClassName(type);

    auto [enumLookup, enumsToBeAssignedMetaObject] = handleEnums(builder, data.enums, type);

    for (const auto &signal : data._signals) {
        auto mmb = builder.addSignal(signal.signature);
        mmb.setParameterNames(signal.parameterNames);
    }

    for (const auto &slot : data._slots) {
        const bool isVoid = slot.returnType.isEmpty() || slot.returnType == QByteArrayLiteral("void");
        QMetaMethodBuilder mmb;
        if (isVoid)
            mmb = builder.addMethod(slot.signature);
        else if (data.isSource)
            mmb = builder.addMethod(slot.signature, slot.returnType);
        else
            mmb = builder.addMethod(slot.signature, QByteArrayLiteral("QRemoteObjectPendingCall"));
        mmb.setParameterNames(slot.parameterNames);
    }

    for (const auto &prop : data.properties) {
        QMetaPropertyBuilder mpb;
        auto metaType = QMetaType::fromName(prop.typeName);
        if (!metaType.isValid()) {
            if (enumLookup.contains(prop.typeName))
                metaType = enumLookup[prop.typeName];
            else
                qDebug() << "No metaType for property" << prop.name << "of type" << prop.typeName;
        }
        if (prop.signalName.isEmpty())
            mpb = builder.addProperty(prop.name, prop.typeName, metaType);
        else
            mpb = builder.addProperty(prop.name, prop.typeName, metaType, builder.indexOfSignal(prop.signalName));
        mpb.setWritable(prop.isWritable);
    }

    auto meta = builder.toMetaObject();
    QList<QMetaType> newEnums(enumsToBeAssignedMetaObject.size());
    for (auto typeInfo : enumsToBeAssignedMetaObject) {
        typeInfo->metaObject = meta;
        newEnums.append(QMetaType(typeInfo));
    }
    return {meta, newEnums};
}

QMetaObject *registerAndTrackDefinition(const ClassData &data, QObject *reference)
{
    auto [meta, newEnums] = registerDefinition(data);
    if (reference) {
        ManagedTypeEntry entry;
        auto id = fakeClassIdManager->addTypeName(meta->className());
        if (id < 0) {
            qWarning() << "Failed to register type" << meta->className() << "name already in use.";
            return nullptr;
        }
        entry.id = id;
        entry.metaObject = meta;
        for (auto metaType : newEnums)
            entry.enumMetaTypes.append(metaType);
        trackReference(id, reference);
        QMutexLocker lock(&s_managedTypesMutex);
        s_managedTypes.insert(id, std::move(entry));
    }
    return meta;
}

static EnumData deserializeEnum(QDataStream &ds)
{
    EnumData enumData;
    ds >> enumData.name;
    ds >> enumData.isFlag;
    ds >> enumData.isScoped;
    ds >> enumData.size;
    ds >> enumData.keyCount;
    enumData.values.reserve(enumData.keyCount);
    for (quint32 i = 0; i < enumData.keyCount; i++) {
        EnumPair pair;
        ds >> pair.name;
        ds >> pair.value;
        enumData.values.append(std::move(pair));
    }
    return enumData;
}

static void parseGadgets(QtROIoDeviceBase *connection, QDataStream &in)
{
    quint32 qtEnums, numGadgets;
    in >> qtEnums; // Qt enums - just need registration
    for (quint32 i = 0; i < qtEnums; ++i) {
        QByteArray enumName;
        in >> enumName;
        QMetaType type(enumMetaType<qint32>(enumName, 4, &Qt::staticMetaObject));
        type.id();
    }
    in >> numGadgets;
    if (numGadgets == 0)
        return;
    Gadgets gadgets;
    for (quint32 i = 0; i < numGadgets; ++i) {
        GadgetData gadget;
        QByteArray type;
        in >> type;
        quint32 numProperties, numEnums;
        in >> numProperties;
        gadget.properties.reserve(numProperties);
        for (quint32 p = 0; p < numProperties; ++p) {
            GadgetProperty prop;
            in >> prop.name;
            in >> prop.type;
            gadget.properties.append(std::move(prop));
        }
        in >> numEnums;
        gadget.enums.reserve(numEnums);
        for (quint32 e = 0; e < numEnums; ++e)
            gadget.enums.append(deserializeEnum(in));
        gadgets[type] = std::move(gadget);
    }
    registerAllGadgets(connection, gadgets);
}

QMetaObject *QRemoteObjectMetaObjectManager::addDynamicType(QtROIoDeviceBase *connection, QDataStream &in)
{
    ClassData classData;

    QString typeString;
    in >> typeString;
    classData.type = typeString.toLatin1();
    // TODO Fix appending "Source"/"Replica"
    // auto type = classData.type + (classData.isSource ? "Source" : "");
    auto type = classData.type;

    quint32 numEnums = 0;
    in >> numEnums;
    classData.enums.reserve(numEnums);
    // QtRemoteObjects Dynamic types will get a class name from the .rep definition, which
    // may be different from the name of the sending class if the QtRO API is used.  We use
    // classEnums to ensure we update enum names.
    QHash<QByteArray, QByteArray> classEnums;
    for (quint32 i = 0; i < numEnums; ++i) {
        auto enumData = deserializeEnum(in);
        // Include Source or Replica in the registered name
        classEnums[enumData.name] = QByteArray(type).append("::").append(enumData.name);
        classData.enums.append(std::move(enumData));
    }

    parseGadgets(connection, in);

    quint32 numSignals = 0;
    in >> numSignals;
    classData._signals.reserve(numSignals);
    for (quint32 i = 0; i < numSignals; ++i) {
        ClassSignal classSignal;
        in >> classSignal.signature;
        in >> classSignal.parameterNames;
        classData._signals.append(std::move(classSignal));
    }

    quint32 numMethods = 0;
    in >> numMethods;
    classData._slots.reserve(numMethods);
    for (quint32 i = 0; i < numMethods; ++i) {
        ClassSlot classSlot;
        QByteArray signature, returnType;
        QByteArrayList paramNames;
        in >> classSlot.signature;
        in >> classSlot.returnType;
        in >> classSlot.parameterNames;
        classData._slots.append(std::move(classSlot));
    }

    quint32 numProperties = 0;
    in >> numProperties;
    classData.properties.reserve(numProperties);
    for (quint32 i = 0; i < numProperties; ++i) {
        ClassProperty classProperty;
        in >> classProperty.name;
        in >> classProperty.typeName;
        in >> classProperty.signalName;
        classProperty.isWritable = !classProperty.signalName.isEmpty();

        // Ignore trailing null
        auto choppedName = QByteArrayView(classProperty.typeName.constData(), classProperty.typeName.size()-1);
        // The typeName for class enums is qualified with the class name.
        // Need to remove the class name  before checking if it's a class enum.
        if (auto idx = choppedName.indexOf("::"); idx >= 0) {
            choppedName.slice(idx + 2);
            if (classEnums.contains(choppedName))
                classProperty.typeName = classEnums[choppedName]; // Update to the enum's registered name
        }
        classData.properties.append(std::move(classProperty));
    }

    auto [meta, newEnums] = registerDefinition(classData);
    enumTypes[meta] = newEnums;
    dynamicTypes.insert(typeString, meta);
    return meta;
}

void QRemoteObjectMetaObjectManager::addFromMetaObject(const QMetaObject *metaObject)
{
    QString className = QLatin1String(metaObject->className());
    if (!className.endsWith(QLatin1String("Replica")))
        return;
    if (className == QLatin1String("QRemoteObjectDynamicReplica") || staticTypes.contains(className))
        return;
    className.chop(7); //Remove 'Replica' from name
    staticTypes.insert(className, metaObject);
}

void QRemoteObjectNodePrivate::connectReplica(QObject *object, QRemoteObjectReplica *instance)
{
    int nConnections = 0;
    const QMetaObject *us = instance->metaObject();
    const QMetaObject *them = object->metaObject();

    static const int memberOffset = QRemoteObjectReplica::staticMetaObject.methodCount();
    for (int idx = memberOffset; idx < us->methodCount(); ++idx) {
        const QMetaMethod mm = us->method(idx);

        qROPrivDebug() << idx << mm.name();
        if (mm.methodType() != QMetaMethod::Signal)
            continue;

        // try to connect to a signal on the parent that has the same method signature
        QByteArray sig = QMetaObject::normalizedSignature(mm.methodSignature().constData());
        qROPrivDebug() << sig;
        if (them->indexOfSignal(sig.constData()) == -1)
            continue;

        sig.prepend(QSIGNAL_CODE + '0');
        const char * const csig = sig.constData();
        const bool res = QObject::connect(object, csig, instance, csig);
        Q_UNUSED(res)
        ++nConnections;

        qROPrivDebug() << sig << res;
    }

    qROPrivDebug() << "# connections =" << nConnections;
}

void QRemoteObjectNodePrivate::openConnectionIfNeeded(const QString &name)
{
    qROPrivDebug() << Q_FUNC_INFO << name << this;
    if (!remoteObjectAddresses().contains(name)) {
        qROPrivDebug() << name << "not available - available addresses:" << remoteObjectAddresses();
        return;
    }

    if (!initConnection(remoteObjectAddresses().value(name).hostUrl))
        qROPrivWarning() << "failed to open connection to" << name;
}

bool QRemoteObjectNodePrivate::initConnection(const QUrl &address)
{
    Q_Q(QRemoteObjectNode);
    if (requestedUrls.contains(address)) {
        qROPrivDebug() << "Connection already requested for " << address.toString();
        return true;
    }

    requestedUrls.insert(address);

    if (schemaHandlers.contains(address.scheme())) {
        schemaHandlers[address.scheme()](address);
        return true;
    }

    QtROClientIoDevice *connection = QtROClientFactory::instance()->create(address, q);
    if (!connection) {
        qROPrivWarning() << "Could not create QtROClientIoDevice for client. Invalid url/scheme provided?" << address;
        return false;
    }
    qROPrivDebug() << "Opening connection to" << address.toString();
    qROPrivDebug() << "Replica Connection isValid" << connection->isOpen();
    QObject::connect(connection, &QtROClientIoDevice::shouldReconnect, q, [this, connection]() {
        onShouldReconnect(connection);
    });
    QObject::connect(connection, &QtROIoDeviceBase::readyRead, q, [this, connection]() {
        onClientRead(connection);
    });
    connect(connection, &QtROClientIoDevice::setError, this,
            &QRemoteObjectNodePrivate::setLastError);
    connection->connectToServer();

    return true;
}

bool QRemoteObjectNodePrivate::hasInstance(const QString &name)
{
    if (!replicas.contains(name))
        return false;

    QSharedPointer<QReplicaImplementationInterface> rep = replicas.value(name).toStrongRef();
    if (!rep) { //already deleted
        replicas.remove(name);
        return false;
    }

    return true;
}

static QDebug operator<<(QDebug debug,
                         const QHash<QString, QWeakPointer<QReplicaImplementationInterface>> &hash)
{
    const QDebugStateSaver saver(debug);
    debug.nospace() << "QHash(";
    for (auto it = hash.cbegin(); it != hash.cend(); ++it)
        debug << '(' << it.key() << ", " << it.value().isNull() << ')';
    debug << ')';
    return debug;
}

void QRemoteObjectNodePrivate::onRemoteObjectSourceAdded(const QRemoteObjectSourceLocation &entry)
{
    qROPrivDebug() << "onRemoteObjectSourceAdded" << entry << replicas << replicas.contains(entry.first);
    if (!entry.first.isEmpty()) {
        QRemoteObjectSourceLocations locs = registry->sourceLocations();
        locs[entry.first] = entry.second;
        //TODO Is there a way to extend QRemoteObjectSourceLocations in place?
        registry->d_impl->setProperty(0, QVariant::fromValue(locs));
        registry->notifySourceLocationsChanged();
        qROPrivDebug() << "onRemoteObjectSourceAdded, now locations =" << locs;
    }
    if (replicas.contains(entry.first)) //We have a replica waiting on this remoteObject
    {
        QSharedPointer<QReplicaImplementationInterface> rep = replicas.value(entry.first).toStrongRef();
        if (!rep) { //replica has been deleted, remove from list
            replicas.remove(entry.first);
            return;
        }

        initConnection(entry.second.hostUrl);

        qROPrivDebug() << "Called initConnection due to new RemoteObjectSource added via registry" << entry.first;
    }
}

void QRemoteObjectNodePrivate::onRemoteObjectSourceRemoved(const QRemoteObjectSourceLocation &entry)
{
    if (!entry.first.isEmpty()) {
        QRemoteObjectSourceLocations locs = registry->sourceLocations();
        locs.remove(entry.first);
        registry->d_impl->setProperty(0, QVariant::fromValue(locs));
        registry->notifySourceLocationsChanged();
    }
}

void QRemoteObjectNodePrivate::onRegistryInitialized()
{
    qROPrivDebug() << "Registry Initialized" << remoteObjectAddresses();

    const auto remotes = remoteObjectAddresses();
    for (auto i = remotes.cbegin(), end = remotes.cend(); i != end; ++i) {
        if (replicas.contains(i.key())) //We have a replica waiting on this remoteObject
        {
            QSharedPointer<QReplicaImplementationInterface> rep = replicas.value(i.key()).toStrongRef();
            if (rep && !requestedUrls.contains(i.value().hostUrl))
                initConnection(i.value().hostUrl);
            else if (!rep) //replica has been deleted, remove from list
                replicas.remove(i.key());

            continue;
        }
    }
}

void QRemoteObjectNodePrivate::onShouldReconnect(QtROClientIoDevice *ioDevice)
{
    Q_Q(QRemoteObjectNode);

    const auto remoteObjects = ioDevice->remoteObjects();
    for (const QString &remoteObject : remoteObjects) {
        connectedSources.remove(remoteObject);
        ioDevice->removeSource(remoteObject);
        if (replicas.contains(remoteObject)) { //We have a replica waiting on this remoteObject
            QSharedPointer<QConnectedReplicaImplementation> rep = qSharedPointerCast<QConnectedReplicaImplementation>(replicas.value(remoteObject).toStrongRef());
            if (rep && !rep->connectionToSource.isNull()) {
                rep->setDisconnected();
            } else if (!rep) {
                replicas.remove(remoteObject);
            }
        }
    }
    if (requestedUrls.contains(ioDevice->url())) {
        // Only try to reconnect to URLs requested via connectToNode
        // If we connected via registry, wait for the registry to see the node/source again
        pendingReconnect.insert(ioDevice);
        if (!reconnectTimer.isActive()) {
            reconnectTimer.start(retryInterval, q);
            qROPrivDebug() << "Starting reconnect timer";
        }
    } else {
        qROPrivDebug() << "Url" << ioDevice->url().toDisplayString().toLatin1()
                       << "lost.  We will reconnect Replicas if they reappear on the Registry.";
    }
}

//This version of handleNewAcquire creates a QConnectedReplica. If this is a
//host node, the QRemoteObjectHostBasePrivate overload is called instead.
QReplicaImplementationInterface *QRemoteObjectNodePrivate::handleNewAcquire(const QMetaObject *meta, QRemoteObjectReplica *instance, const QString &name)
{
    Q_Q(QRemoteObjectNode);
    QConnectedReplicaImplementation *rp = new QConnectedReplicaImplementation(name, meta, q);
    rp->configurePrivate(instance);
    if (connectedSources.contains(name)) { //Either we have a peer connections, or existing connection via registry
        handleReplicaConnection(connectedSources[name].objectSignature, rp, connectedSources[name].device);
    } else {
        //No existing connection, but we know we can connect via registry
        const auto &sourceLocations = remoteObjectAddresses();
        const auto it = sourceLocations.constFind(name);
        // This will try the connection, and if successful, the remoteObjects will be sent
        // The link to the replica will be handled then
        if (it != sourceLocations.constEnd())
            initConnection(it.value().hostUrl);
    }
    return rp;
}

void QRemoteObjectNodePrivate::handleReplicaConnection(const QString &name)
{
    QSharedPointer<QRemoteObjectReplicaImplementation> rep = qSharedPointerCast<QRemoteObjectReplicaImplementation>(replicas.value(name).toStrongRef());
    if (!rep) { //replica has been deleted, remove from list
        replicas.remove(name);
        return;
    }

    if (rep->isShortCircuit())
        return;

    QConnectedReplicaImplementation *connectedRep = static_cast<QConnectedReplicaImplementation *>(rep.data());
    if (connectedRep->connectionToSource.isNull()) {
        const auto sourceInfo = connectedSources.value(name);
        handleReplicaConnection(sourceInfo.objectSignature, connectedRep, sourceInfo.device);
    }
}

void QRemoteObjectNodePrivate::handleReplicaConnection(const QByteArray &sourceSignature, QConnectedReplicaImplementation *rep, QtROIoDeviceBase *connection)
{
    if (!checkSignatures(rep->m_objectSignature, sourceSignature)) {
        qROPrivWarning() << "Signature mismatch for" << rep->m_metaObject->className() << (rep->m_objectName.isEmpty() ? QLatin1String("(unnamed)") : rep->m_objectName);
        rep->setState(QRemoteObjectReplica::SignatureMismatch);
        return;
    }
    rep->setConnection(connection);
}

//Host Nodes can use the more efficient InProcess Replica if we (this Node) hold the Source for the
//requested Replica.  If not, fall back to the Connected Replica case.
QReplicaImplementationInterface *QRemoteObjectHostBasePrivate::handleNewAcquire(const QMetaObject *meta, QRemoteObjectReplica *instance, const QString &name)
{
    QMap<QString, QRemoteObjectSourceBase*>::const_iterator mapIt;
    if (remoteObjectIo && map_contains(remoteObjectIo->m_sourceObjects, name, mapIt)) {
        Q_Q(QRemoteObjectHostBase);
        QInProcessReplicaImplementation *rp = new QInProcessReplicaImplementation(name, meta, q);
        rp->configurePrivate(instance);
        connectReplica(mapIt.value()->m_object, instance);
        rp->connectionToSource = mapIt.value();
        return rp;
    }
    return QRemoteObjectNodePrivate::handleNewAcquire(meta, instance, name);
}

void QRemoteObjectNodePrivate::onClientRead(QObject *obj)
{
    using namespace QRemoteObjectPackets;
    QtROIoDeviceBase *connection = qobject_cast<QtROIoDeviceBase*>(obj);
    QRemoteObjectPacketTypeEnum packetType;
    Q_ASSERT(connection);
    auto &codec = connection->d_func()->m_codec;

    do {
        if (!connection->read(packetType, rxName))
            return;

        if (packetType != Handshake && codec == nullptr) {
            qROPrivWarning() << "Expected Handshake, got " << packetType;
            setLastError(QRemoteObjectNode::ProtocolMismatch);
            connection->close();
            break;
        }

        switch (packetType) {
        case QRemoteObjectPacketTypeEnum::Pong:
        {
            QSharedPointer<QRemoteObjectReplicaImplementation> rep = qSharedPointerCast<QRemoteObjectReplicaImplementation>(replicas.value(rxName).toStrongRef());
            if (rep)
                rep->notifyAboutReply(0, {});
            else //replica has been deleted, remove from list
                replicas.remove(rxName);
            break;
        }
        case QRemoteObjectPacketTypeEnum::Handshake:
            if (rxName != QtRemoteObjects::protocolVersion) {
                qWarning() << "*** Protocol Mismatch, closing connection ***. Got" << rxName << "expected" << QtRemoteObjects::protocolVersion;
                setLastError(QRemoteObjectNode::ProtocolMismatch);
                connection->close();
            } else {
                // TODO should have some sort of manager for the codec
                codec.reset(new QRemoteObjectPackets::QDataStreamCodec);
            }
            break;
        case QRemoteObjectPacketTypeEnum::ObjectList:
        {
            codec->deserializeObjectListPacket(connection->d_func()->stream(), rxObjects);
            qROPrivDebug() << "newObjects:" << rxObjects;
            // We need to make sure all of the source objects are in connectedSources before we add connections,
            // otherwise nested QObjects could fail (we want to acquire children before parents, and the object
            // list is unordered)
            for (const auto &remoteObject : std::as_const(rxObjects)) {
                qROPrivDebug() << "  connectedSources.contains(" << remoteObject << ")" << connectedSources.contains(remoteObject.name) << replicas.contains(remoteObject.name);
                if (!connectedSources.contains(remoteObject.name)) {
                    connectedSources[remoteObject.name] = SourceInfo{connection, remoteObject.typeName, remoteObject.signature};
                    connection->addSource(remoteObject.name);
                    // Make sure we handle Registry first if it is available
                    if (remoteObject.name == QLatin1String("Registry") && replicas.contains(remoteObject.name))
                        handleReplicaConnection(remoteObject.name);
                }
            }
            for (const auto &remoteObject : std::as_const(rxObjects)) {
                if (replicas.contains(remoteObject.name)) //We have a replica waiting on this remoteObject
                    handleReplicaConnection(remoteObject.name);
            }
            break;
        }
        case QRemoteObjectPacketTypeEnum::InitPacket:
        {
            qROPrivDebug() << "InitPacket-->" << rxName << this;
            QSharedPointer<QConnectedReplicaImplementation> rep = qSharedPointerCast<QConnectedReplicaImplementation>(replicas.value(rxName).toStrongRef());
            //Use m_rxArgs (a QVariantList to hold the properties QVariantList)
            codec->deserializeInitPacket(connection->d_func()->stream(), rxArgs);
            if (rep)
            {
                handlePointerToQObjectProperties(rep.data(), rxArgs);
                rep->initialize(std::move(rxArgs));
            } else { //replica has been deleted, remove from list
                replicas.remove(rxName);
            }
            break;
        }
        case QRemoteObjectPacketTypeEnum::InitDynamicPacket:
        {
            qROPrivDebug() << "InitDynamicPacket-->" << rxName << this;
            const QMetaObject *meta = dynamicTypeManager.addDynamicType(connection, connection->d_func()->stream());
            codec->deserializeInitPacket(connection->d_func()->stream(), rxArgs);
            QSharedPointer<QConnectedReplicaImplementation> rep = qSharedPointerCast<QConnectedReplicaImplementation>(replicas.value(rxName).toStrongRef());
            if (rep)
            {
                rep->setDynamicMetaObject(meta);
                handlePointerToQObjectProperties(rep.data(), rxArgs);
                rep->setDynamicProperties(std::move(rxArgs));
            } else { //replica has been deleted, remove from list
                replicas.remove(rxName);
            }
            break;
        }
        case QRemoteObjectPacketTypeEnum::RemoveObject:
        {
            qROPrivDebug() << "RemoveObject-->" << rxName << this;
            connectedSources.remove(rxName);
            connection->removeSource(rxName);
            if (replicas.contains(rxName)) { //We have a replica using the removed source
                QSharedPointer<QConnectedReplicaImplementation> rep = qSharedPointerCast<QConnectedReplicaImplementation>(replicas.value(rxName).toStrongRef());
                if (rep && !rep->connectionToSource.isNull()) {
                    rep->connectionToSource.clear();
                    rep->setState(QRemoteObjectReplica::Suspect);
                } else if (!rep) {
                    replicas.remove(rxName);
                }
            }
            break;
        }
        case QRemoteObjectPacketTypeEnum::PropertyChangePacket:
        {
            int propertyIndex;
            codec->deserializePropertyChangePacket(connection->d_func()->stream(), propertyIndex, rxValue);
            QSharedPointer<QRemoteObjectReplicaImplementation> rep = qSharedPointerCast<QRemoteObjectReplicaImplementation>(replicas.value(rxName).toStrongRef());
            if (rep) {
                QConnectedReplicaImplementation *connectedRep = nullptr;
                if (!rep->isShortCircuit()) {
                    connectedRep = static_cast<QConnectedReplicaImplementation *>(rep.data());
                    if (!connectedRep->childIndices().contains(propertyIndex))
                        connectedRep = nullptr; //connectedRep will be a valid pointer only if propertyIndex is a child index
                }
                if (connectedRep)
                    rep->setProperty(propertyIndex, handlePointerToQObjectProperty(connectedRep, propertyIndex, rxValue));
                else {
                    const QMetaProperty property = rep->m_metaObject->property(propertyIndex + rep->m_metaObject->propertyOffset());
                    if (property.userType() == QMetaType::QVariant && rxValue.canConvert<QRO_>()) {
                        // This is a type that requires registration
                        QRO_ typeInfo = rxValue.value<QRO_>();
                        QDataStream in(typeInfo.classDefinition);
                        parseGadgets(connection, in);
                        QDataStream ds(typeInfo.parameters);
                        ds >> rxValue;
                    }
                    rep->setProperty(propertyIndex, decodeVariant(std::move(rxValue), property.metaType()));
                }
            } else { //replica has been deleted, remove from list
                replicas.remove(rxName);
            }
            break;
        }
        case QRemoteObjectPacketTypeEnum::InvokePacket:
        {
            int call, index, serialId, propertyIndex;
            codec->deserializeInvokePacket(connection->d_func()->stream(), call, index, rxArgs, serialId, propertyIndex);
            QSharedPointer<QRemoteObjectReplicaImplementation> rep = qSharedPointerCast<QRemoteObjectReplicaImplementation>(replicas.value(rxName).toStrongRef());
            if (rep) {
                static QVariant null(QMetaType::fromType<QObject *>(), nullptr);
                QVariant paramValue;
                // Qt usually supports 9 arguments, so ten should be usually safe
                QVarLengthArray<void*, 10> param(rxArgs.size() + 1);
                param[0] = null.data(); //Never a return value
                if (rxArgs.size()) {
                    auto signal = rep->m_metaObject->method(index+rep->m_signalOffset);
                    for (int i = 0; i < rxArgs.size(); i++) {
                        if (signal.parameterType(i) == QMetaType::QVariant)
                            param[i + 1] = const_cast<void*>(reinterpret_cast<const void*>(&rxArgs.at(i)));
                        else {
                            rxArgs[i] = decodeVariant(std::move(rxArgs[i]), signal.parameterMetaType(i));
                            param[i + 1] = const_cast<void *>(rxArgs.at(i).data());
                        }
                    }
                } else if (propertyIndex != -1) {
                    param.resize(2);
                    paramValue = rep->getProperty(propertyIndex);
                    param[1] = paramValue.data();
                }
                qROPrivDebug() << "Replica Invoke-->" << rxName << rep->m_metaObject->method(index+rep->m_signalOffset).name() << index << rep->m_signalOffset;
                // We activate on rep->metaobject() so the private metacall is used, not m_metaobject (which
                // is the class thie replica looks like)
                QMetaObject::activate(rep.data(), rep->metaObject(), index+rep->m_signalOffset, param.data());
            } else { //replica has been deleted, remove from list
                replicas.remove(rxName);
            }
            break;
        }
        case QRemoteObjectPacketTypeEnum::InvokeReplyPacket:
        {
            int ackedSerialId;
            codec->deserializeInvokeReplyPacket(connection->d_func()->stream(), ackedSerialId, rxValue);
            QSharedPointer<QRemoteObjectReplicaImplementation> rep = qSharedPointerCast<QRemoteObjectReplicaImplementation>(replicas.value(rxName).toStrongRef());
            if (rep) {
                qROPrivDebug() << "Received InvokeReplyPacket ack'ing serial id:" << ackedSerialId;
                rxValue = decodeVariant(std::move(rxValue), {});
                rep->notifyAboutReply(ackedSerialId, rxValue);
            } else { //replica has been deleted, remove from list
                replicas.remove(rxName);
            }
            break;
        }
        case QRemoteObjectPacketTypeEnum::AddObject:
        case QRemoteObjectPacketTypeEnum::Invalid:
        case QRemoteObjectPacketTypeEnum::Ping:
            qROPrivWarning() << "Unexpected packet received";
        }
    } while (connection->bytesAvailable()); // have bytes left over, so do another iteration
}

/*!
    \class QRemoteObjectNode
    \inmodule QtRemoteObjects
    \brief A node on a Qt Remote Objects network.

    The QRemoteObjectNode class provides an entry point to a QtRemoteObjects
    network. A network can be as simple as two nodes, or an arbitrarily complex
    set of processes and devices.

    A QRemoteObjectNode does not have a url that other nodes can connect to,
    and thus is able to acquire replicas only. It is not able to share source
    objects (only QRemoteObjectHost and QRemoteObjectRegistryHost Nodes can
    share).

    Nodes may connect to each other directly using \l connectToNode, or
    they can use the QRemoteObjectRegistry to simplify connections.

    The QRemoteObjectRegistry is a special replica available to every node that
    connects to the Registry Url. It knows how to connect to every
    QRemoteObjectSource object on the network.

    \sa QRemoteObjectHost, QRemoteObjectRegistryHost
*/

/*!
    \class QRemoteObjectHostBase
    \inmodule QtRemoteObjects
    \brief The QRemoteObjectHostBase class provides base functionality common to \l
    {QRemoteObjectHost} {Host} and \l {QRemoteObjectRegistryHost} {RegistryHost} classes.

    QRemoteObjectHostBase is a base class that cannot be instantiated directly.
    It provides the enableRemoting and disableRemoting functionality shared by
    all host nodes (\l {QRemoteObjectHost} {Host} and \l
    {QRemoteObjectRegistryHost} {RegistryHost}) as well as the logic required
    to expose \l {Source} objects on the Remote Objects network.
*/

/*!
    \class QRemoteObjectHost
    \inmodule QtRemoteObjects
    \brief A (Host) Node on a Qt Remote Objects network.

    The QRemoteObjectHost class provides an entry point to a QtRemoteObjects
    network. A network can be as simple as two nodes, or an arbitrarily complex
    set of processes and devices.

    QRemoteObjectHosts have the same capabilities as QRemoteObjectNodes, but
    they can also be connected to and can share source objects on the network.

    Nodes may connect to each other directly using \l connectToNode, or they
    can use the QRemoteObjectRegistry to simplify connections.

    The QRemoteObjectRegistry is a special replica available to every node that
    connects to the registry Url. It knows how to connect to every
    QRemoteObjectSource object on the network.

    \sa QRemoteObjectNode, QRemoteObjectRegistryHost
*/

/*!
    \class QRemoteObjectRegistryHost
    \inmodule QtRemoteObjects
    \brief A (Host/Registry) node on a Qt Remote Objects network.

    The QRemoteObjectRegistryHost class provides an entry point to a QtRemoteObjects
    network. A network can be as simple as two Nodes, or an arbitrarily complex
    set of processes and devices.

    A QRemoteObjectRegistryHost has the same capability that a
    QRemoteObjectHost has (which includes everything a QRemoteObjectNode
    supports), and in addition is the owner of the Registry. Any
    QRemoteObjectHost node that connects to this Node will have all of their
    Source objects made available by the Registry.

    Nodes only support connection to one \l registry, calling \l
    QRemoteObjectNode::setRegistryUrl when a Registry is already set is
    considered an error. For something like a secure and insecure network
    (where different Registries would be applicable), the recommendation is to
    create separate Nodes to connect to each, in effect creating two
    independent Qt Remote Objects networks.

    Nodes may connect to each other directly using \l connectToNode, or they
    can use the QRemoteObjectRegistry to simplify connections.

    The QRemoteObjectRegistry is a special Replica available to every Node that
    connects to the Registry Url. It knows how to connect to every
    QRemoteObjectSource object on the network.

    \sa QRemoteObjectNode, QRemoteObjectHost
*/

/*!
    \enum QRemoteObjectNode::ErrorCode

    This enum type specifies the various error codes associated with QRemoteObjectNode errors:

    \value NoError No error.
    \value RegistryNotAcquired The registry could not be acquired.
    \value RegistryAlreadyHosted The registry is already defined and hosting Sources.
    \value NodeIsNoServer The given QRemoteObjectNode is not a host node.
    \value ServerAlreadyCreated The host node has already been initialized.
    \value UnintendedRegistryHosting An attempt was made to create a host QRemoteObjectNode and connect to itself as the registry.
    \value OperationNotValidOnClientNode The attempted operation is not valid on a client QRemoteObjectNode.
    \value SourceNotRegistered The given QRemoteObjectSource is not registered on this node.
    \value MissingObjectName The given QObject does not have objectName() set.
    \value HostUrlInvalid The given url has an invalid or unrecognized scheme.
    \value ProtocolMismatch The client and the server have different protocol versions.
    \value ListenFailed Can't listen on the specified host port.
    \value SocketAccessError The client isn't allowed to connect to the server. Ensure that QRemoteObjectHost::setLocalServerOptions is set appropriately.
*/

/*!
    \enum QRemoteObjectHostBase::AllowedSchemas

    This enum is used to specify whether a Node will accept a url with an
    unrecognized schema for the hostUrl. By default only urls with known
    schemas are accepted, but using \c AllowExternalRegistration will enable
    the \l Registry to pass your external (to QtRO) url to client Nodes.

    \value BuiltInSchemasOnly Only allow the hostUrl to be set to a QtRO
    supported schema. This is the default value, and causes a Node error to be
    set if an unrecognized schema is provided.
    \value AllowExternalRegistration The provided schema is registered as an
    \l {External Schemas}{External Schema}

    \sa QRemoteObjectHost
*/

/*!
    \fn template <class ObjectType> ObjectType *QRemoteObjectNode::acquire(const QString &name)

    Returns a pointer to a Replica of type ObjectType (which is a template
    parameter and must inherit from \l QRemoteObjectReplica). That is, the
    template parameter must be a \l {repc} generated type. The \a name
    parameter can be used to specify the \a name given to the object
    during the QRemoteObjectHost::enableRemoting() call.
*/

void QRemoteObjectNodePrivate::initialize()
{
    qRegisterMetaType<QRemoteObjectNode *>();
    qRegisterMetaType<QRemoteObjectNode::ErrorCode>();
    qRegisterMetaType<QAbstractSocket::SocketError>(); //For queued qnx error()
    qRegisterMetaType<QRemoteObjectPackets::QRO_>();
    qRegisterMetaType<QRemoteObjectPackets::QSQ_>();
    qRegisterMetaType<QRemoteObjectPackets::QAS_>();
    qRegisterMetaType<QtROSequentialContainer>();
    qRegisterMetaType<QtROAssociativeContainer>();
    // To support dynamic MODELs, we need to make sure the types are registered
    QAbstractItemModelSourceAdapter::registerTypes();
}

bool QRemoteObjectNodePrivate::checkSignatures(const QByteArray &a, const QByteArray &b)
{
    // if any of a or b is empty it means it's a dynamic ojects or an item model
    if (a.isEmpty() || b.isEmpty())
        return true;
    return a == b;
}


void QRemoteObjectNode::persistProperties(const QString &repName, const QByteArray &repSig, const QVariantList &props)
{
    Q_D(QRemoteObjectNode);
    if (d->persistedStore) {
        d->persistedStore->saveProperties(repName, repSig, props);
    } else {
        qCWarning(QT_REMOTEOBJECT) << qPrintable(objectName()) << "Unable to store persisted properties for" << repName;
        qCWarning(QT_REMOTEOBJECT) << "    No persisted store set.";
    }
}

QVariantList QRemoteObjectNode::retrieveProperties(const QString &repName, const QByteArray &repSig)
{
    Q_D(QRemoteObjectNode);
    if (d->persistedStore) {
        return d->persistedStore->restoreProperties(repName, repSig);
    }
    qCWarning(QT_REMOTEOBJECT) << qPrintable(objectName()) << "Unable to retrieve persisted properties for" << repName;
    qCWarning(QT_REMOTEOBJECT) << "    No persisted store set.";
    return QVariantList();
}

/*!
    Default constructor for QRemoteObjectNode with the given \a parent. A Node
    constructed in this manner can not be connected to, and thus can not expose
    Source objects on the network. It also will not include a \l
    QRemoteObjectRegistry, unless set manually using setRegistryUrl.

    \sa connectToNode, setRegistryUrl
*/
QRemoteObjectNode::QRemoteObjectNode(QObject *parent)
    : QObject(*new QRemoteObjectNodePrivate, parent)
{
    Q_D(QRemoteObjectNode);
    d->initialize();
}

/*!
    QRemoteObjectNode connected to a {QRemoteObjectRegistry} {Registry}. A Node
    constructed in this manner can not be connected to, and thus can not expose
    Source objects on the network. Finding and connecting to other (Host) Nodes
    is handled by the QRemoteObjectRegistry specified by \a registryAddress.

    \sa connectToNode, setRegistryUrl, QRemoteObjectHost, QRemoteObjectRegistryHost
*/
QRemoteObjectNode::QRemoteObjectNode(const QUrl &registryAddress, QObject *parent)
    : QObject(*new QRemoteObjectNodePrivate, parent)
{
    Q_D(QRemoteObjectNode);
    d->initialize();
    d->setRegistryUrlNodeImpl(registryAddress);
}

QRemoteObjectNode::QRemoteObjectNode(QRemoteObjectNodePrivate &dptr, QObject *parent)
    : QObject(dptr, parent)
{
    Q_D(QRemoteObjectNode);
    d->initialize();
}

/*!
    \qmltype Host
    \nativetype QRemoteObjectHost
    \inqmlmodule QtRemoteObjects
    \brief A host node on a Qt Remote Objects network.

    The Host type provides an entry point to a Qt Remote Objects network. A network
    can be as simple as two nodes, or an arbitrarily complex set of processes and devices.

    Hosts have the same capabilities as Nodes, but they can also be connected to and can
    share source objects on the network.
*/

/*!
    \internal This is a base class for both QRemoteObjectHost and
    QRemoteObjectRegistryHost to provide the shared features/functions for
    sharing \l Source objects.
*/
QRemoteObjectHostBase::QRemoteObjectHostBase(QRemoteObjectHostBasePrivate &d, QObject *parent)
    : QRemoteObjectNode(d, parent)
{ }

/*!
    Constructs a new QRemoteObjectHost Node (i.e., a Node that supports
    exposing \l Source objects on the QtRO network) with the given \a parent.
    This constructor is meant specific to support QML in the future as it will
    not be available to connect to until \l {QRemoteObjectHost::}{setHostUrl} is called.

    \sa setHostUrl(), setRegistryUrl()
*/
QRemoteObjectHost::QRemoteObjectHost(QObject *parent)
    : QRemoteObjectHostBase(*new QRemoteObjectHostPrivate, parent)
{ }

/*!
    Constructs a new QRemoteObjectHost Node (i.e., a Node that supports
    exposing \l Source objects on the QtRO network) with address \a address. If
    set, \a registryAddress will be used to connect to the \l
    QRemoteObjectRegistry at the provided address. The \a allowedSchemas
    parameter is only needed (and should be set to \l
    {QRemoteObjectHostBase::AllowExternalRegistration}
    {AllowExternalRegistration}) if the schema of the url should be used as an
    \l {External Schemas} {External Schema} by the registry.

    \sa setHostUrl(), setRegistryUrl()
*/
QRemoteObjectHost::QRemoteObjectHost(const QUrl &address, const QUrl &registryAddress,
                                     AllowedSchemas allowedSchemas, QObject *parent)
    : QRemoteObjectHostBase(*new QRemoteObjectHostPrivate, parent)
{
    if (!address.isEmpty()) {
        if (!d_func()->setHostUrlBaseImpl(address, allowedSchemas))
            return;
    }

    if (!registryAddress.isEmpty())
        d_func()->setRegistryUrlNodeImpl(registryAddress);
}

/*!
    Constructs a new QRemoteObjectHost Node (i.e., a Node that supports
    exposing \l Source objects on the QtRO network) with a url of \a
    address and the given \a parent. This overload is provided as a convenience for specifying a
    QObject parent without providing a registry address.

    \sa setHostUrl(), setRegistryUrl()
*/
QRemoteObjectHost::QRemoteObjectHost(const QUrl &address, QObject *parent)
    : QRemoteObjectHostBase(*new QRemoteObjectHostPrivate, parent)
{
    if (!address.isEmpty())
        d_func()->setHostUrlBaseImpl(address);
}

/*!
    \internal QRemoteObjectHost::QRemoteObjectHost
*/
QRemoteObjectHost::QRemoteObjectHost(QRemoteObjectHostPrivate &d, QObject *parent)
    : QRemoteObjectHostBase(d, parent)
{ }

QRemoteObjectHost::~QRemoteObjectHost() {}

/*!
    Constructs a new QRemoteObjectRegistryHost Node with the given \a parent. RegistryHost Nodes have
    the same functionality as \l QRemoteObjectHost Nodes, except rather than
    being able to connect to a \l QRemoteObjectRegistry, the provided Host QUrl
    (\a registryAddress) becomes the address of the registry for other Nodes to
    connect to.
*/
QRemoteObjectRegistryHost::QRemoteObjectRegistryHost(const QUrl &registryAddress, QObject *parent)
    : QRemoteObjectHostBase(*new QRemoteObjectRegistryHostPrivate, parent)
{
    if (registryAddress.isEmpty())
        return;

    d_func()->setRegistryUrlRegistryHostImpl(registryAddress);
}

/*!
    \internal
*/
QRemoteObjectRegistryHost::QRemoteObjectRegistryHost(QRemoteObjectRegistryHostPrivate &d, QObject *parent)
    : QRemoteObjectHostBase(d, parent)
{ }

QRemoteObjectRegistryHost::~QRemoteObjectRegistryHost() {}

QRemoteObjectNode::~QRemoteObjectNode()
{ }

QRemoteObjectHostBase::~QRemoteObjectHostBase()
{ }

/*!
    Sets \a name as the internal name for this Node.  This
    is then output as part of the logging (if enabled).
    This is primarily useful if you merge log data from multiple nodes.
*/
void QRemoteObjectNode::setName(const QString &name)
{
    setObjectName(name);
}

/*!
    Similar to QObject::setObjectName() (which this method calls), but this
    version also applies the \a name to internal classes as well, which are
    used in some of the debugging output.
*/
void QRemoteObjectHostBase::setName(const QString &name)
{
    Q_D(QRemoteObjectHostBase);
    setObjectName(name);
    if (d->remoteObjectIo)
        d->remoteObjectIo->setObjectName(name);
}

/*!
    \internal The HostBase version of this method is protected so the method
    isn't exposed on RegistryHost nodes.
*/
QUrl QRemoteObjectHostBase::hostUrl() const
{
    Q_D(const QRemoteObjectHostBase);
    if (d->remoteObjectIo)
        return d->remoteObjectIo->serverAddress();
    return QUrl();
}

/*!
    \internal

    The HostBase version of this method is protected so the method
    isn't exposed on RegistryHost nodes.
*/
bool QRemoteObjectHostBase::setHostUrl(const QUrl &hostAddress, AllowedSchemas allowedSchemas)
{
    return d_func()->setHostUrlBaseImpl(hostAddress, allowedSchemas);
}

/*!
 *  \internal
 */
bool QRemoteObjectHostBasePrivate::setHostUrlBaseImpl(
        const QUrl &hostAddress, QRemoteObjectHostBase::AllowedSchemas allowedSchemas)
{
    Q_Q(QRemoteObjectHostBase);
    if (remoteObjectIo) {
        setLastError(QRemoteObjectHostBase::ServerAlreadyCreated);
        return false;
    }

    if (allowedSchemas == QRemoteObjectHostBase::AllowedSchemas::BuiltInSchemasOnly && !QtROServerFactory::instance()->isValid(hostAddress)) {
        setLastError(QRemoteObjectHostBase::HostUrlInvalid);
        return false;
    }

    if (allowedSchemas == QRemoteObjectHostBase::AllowedSchemas::AllowExternalRegistration && QtROServerFactory::instance()->isValid(hostAddress)) {
        qWarning() << qPrintable(q->objectName()) << "Overriding a valid QtRO url (" << hostAddress << ") with AllowExternalRegistration is not allowed.";
        setLastError(QRemoteObjectHostBase::HostUrlInvalid);
        return false;
    }
    remoteObjectIo = new QRemoteObjectSourceIo(hostAddress, q);
    QLocalServer::SocketOptions socketOptions;
    {
        QMutexLocker lock(&s_managedTypesMutex);
        socketOptions = s_localServerOptions;
    }
    if (socketOptions != QLocalServer::NoOptions)
        remoteObjectIo->setSocketOptions(socketOptions);

    if (allowedSchemas == QRemoteObjectHostBase::AllowedSchemas::BuiltInSchemasOnly && !remoteObjectIo->startListening()) {
        setLastError(QRemoteObjectHostBase::ListenFailed);
        delete remoteObjectIo;
        remoteObjectIo = nullptr;
        return false;
    }

    //If we've given a name to the node, set it on the sourceIo as well
    if (!q->objectName().isEmpty())
        remoteObjectIo->setObjectName(q->objectName());
    //Since we don't know whether setHostUrl or setRegistryUrl/setRegistryHost will be called first,
    //break it into two pieces.  setHostUrl connects the RemoteObjectSourceIo->[add/remove]RemoteObjectSource to QRemoteObjectReplicaNode->[add/remove]RemoteObjectSource
    //setRegistry* calls appropriately connect RemoteObjecSourcetIo->[add/remove]RemoteObjectSource to the registry when it is created
    QObject::connect(remoteObjectIo, &QRemoteObjectSourceIo::remoteObjectAdded, q, &QRemoteObjectHostBase::remoteObjectAdded);
    QObject::connect(remoteObjectIo, &QRemoteObjectSourceIo::remoteObjectRemoved, q, &QRemoteObjectHostBase::remoteObjectRemoved);

    return true;
}

/*!
    \brief Sets the socket options for QLocalServer backends to \a options.
    \since 6.7

    It must be set before the QRemoteObjectHost object starts listening.
    It has no consequence for already listening QRemoteObjectHost
    objects or QRemoteObjectHost objects that use different
    backends than QLocalServer. QRemoteObjectHost objects start to
    listen during construction if the \e address argument is
    non-empty, otherwise when the address is set via setHostUrl().

    \sa setHostUrl(), QRemoteObjectHost()

*/
void QRemoteObjectHost::setLocalServerOptions(QLocalServer::SocketOptions options)
{
    QMutexLocker lock(&s_managedTypesMutex);
    s_localServerOptions = options;
}

/*!
    \qmlproperty url Host::hostUrl

    The host address for the node.

    This is the address where source objects remoted by this node will reside.
*/

/*!
    \property QRemoteObjectHost::hostUrl
    \brief The host address for the node.

    This is the address where source objects remoted by this node will reside.
*/

/*!
    Returns the host address for the QRemoteObjectNode as a QUrl. If the Node
    is not a Host node, returns an empty QUrl.

    \sa setHostUrl()
*/
QUrl QRemoteObjectHost::hostUrl() const
{
    return QRemoteObjectHostBase::hostUrl();
}

/*!
    Sets the \a hostAddress for a host QRemoteObjectNode.

    Returns \c true if the Host address is set, otherwise \c false.

    The \a allowedSchemas parameter is only needed (and should be set to \l
    {QRemoteObjectHostBase::AllowExternalRegistration}
    {AllowExternalRegistration}) if the schema of the url should be used as an
    \l {External Schemas} {External Schema} by the registry.
*/
bool QRemoteObjectHost::setHostUrl(const QUrl &hostAddress, AllowedSchemas allowedSchemas)
{
    return d_func()->setHostUrlHostImpl(hostAddress, allowedSchemas);
}

/*!
 *  \internal
 */
bool QRemoteObjectHostPrivate::setHostUrlHostImpl(
        const QUrl &hostAddress, QRemoteObjectHostBase::AllowedSchemas allowedSchemas)
{
    bool success = setHostUrlBaseImpl(hostAddress, allowedSchemas);
    if (success)
        q_func()->hostUrlChanged();
    return success;
}

/*!
    This method can be used to set the address of this Node to \a registryUrl
    (used for other Nodes to connect to this one), if the QUrl isn't set in the
    constructor. Since this Node becomes the Registry, calling this setter
    method causes this Node to use the url as the host address. All other
    Node's use the \l {QRemoteObjectNode::setRegistryUrl} method initiate a
    connection to the Registry.

    Returns \c true if the registry address is set, otherwise \c false.

    \sa QRemoteObjectRegistryHost(), QRemoteObjectNode::setRegistryUrl
*/
bool QRemoteObjectRegistryHost::setRegistryUrl(const QUrl &registryUrl)
{
    return d_func()->setRegistryUrlRegistryHostImpl(registryUrl);
}

bool QRemoteObjectRegistryHostPrivate::setRegistryUrlRegistryHostImpl(const QUrl &registryUrl)
{
    Q_Q(QRemoteObjectRegistryHost);
    if (setHostUrlBaseImpl(registryUrl)) {
        if (!remoteObjectIo) {
            setLastError(QRemoteObjectNode::ServerAlreadyCreated);
            return false;
        } else if (registry) {
            setLastError(QRemoteObjectNode::RegistryAlreadyHosted);
            return false;
        }

        QRegistrySource *remoteObject = new QRegistrySource(q);
        q->enableRemoting(remoteObject);
        registryAddress = remoteObjectIo->serverAddress();
        registrySource = remoteObject;
        //Connect RemoteObjectSourceIo->remoteObject[Added/Removde] to the registry Slot
        QObject::connect(q, &QRemoteObjectRegistryHost::remoteObjectAdded, registrySource, &QRegistrySource::addSource);
        QObject::connect(q, &QRemoteObjectRegistryHost::remoteObjectRemoved, registrySource, &QRegistrySource::removeSource);
        QObject::connect(remoteObjectIo, &QRemoteObjectSourceIo::serverRemoved, registrySource, &QRegistrySource::removeServer);
        //onAdd/Remove update the known remoteObjects list in the RegistrySource, so no need to connect to the RegistrySource remoteObjectAdded/Removed signals
        setRegistry(q->acquire<QRemoteObjectRegistry>());
        return true;
    }
    return false;
}

/*!
    Returns the last error set.
*/
QRemoteObjectNode::ErrorCode QRemoteObjectNode::lastError() const
{
    Q_D(const QRemoteObjectNode);
    return d->lastError;
}

/*!
    \qmlproperty url Node::registryUrl

    The address of the \l {QRemoteObjectRegistry} {Registry} used by this node.

    This is an empty QUrl if there is no registry in use.
*/

/*!
    \property QRemoteObjectNode::registryUrl
    \brief The address of the \l {QRemoteObjectRegistry} {Registry} used by this node.

    This is an empty QUrl if there is no registry in use.
*/
QUrl QRemoteObjectNode::registryUrl() const
{
    Q_D(const QRemoteObjectNode);
    return d->registryAddress;
}

bool QRemoteObjectNode::setRegistryUrl(const QUrl &registryAddress)
{
    Q_D(QRemoteObjectNode);
    return d->setRegistryUrlNodeImpl(registryAddress);
}

bool QRemoteObjectNodePrivate::setRegistryUrlNodeImpl(const QUrl &registryAddr)
{
    Q_Q(QRemoteObjectNode);
    if (registry) {
        setLastError(QRemoteObjectNode::RegistryAlreadyHosted);
        return false;
    }

    registryAddress = registryAddr;
    setRegistry(q->acquire<QRemoteObjectRegistry>());
    //Connect remoteObject[Added/Removed] to the registry Slot
    QObject::connect(q, &QRemoteObjectNode::remoteObjectAdded, registry, &QRemoteObjectRegistry::addSource);
    QObject::connect(q, &QRemoteObjectNode::remoteObjectRemoved, registry, &QRemoteObjectRegistry::removeSource);
    q->connectToNode(registryAddress);
    return true;
}

void QRemoteObjectNodePrivate::setRegistry(QRemoteObjectRegistry *reg)
{
    Q_Q(QRemoteObjectNode);
    registry = reg;
    reg->setParent(q);
    //Make sure when we get the registry initialized, we update our replicas
    QObject::connect(reg, &QRemoteObjectRegistry::initialized, q, [this]() {
        onRegistryInitialized();
    });
    //Make sure we handle new RemoteObjectSources on Registry...
    QObject::connect(reg, &QRemoteObjectRegistry::remoteObjectAdded,
                     q, [this](const QRemoteObjectSourceLocation &location) {
        onRemoteObjectSourceAdded(location);
    });
    QObject::connect(reg, &QRemoteObjectRegistry::remoteObjectRemoved,
                     q, [this](const QRemoteObjectSourceLocation &location) {
        onRemoteObjectSourceRemoved(location);
    });
}

QVariant QRemoteObjectNodePrivate::handlePointerToQObjectProperty(QConnectedReplicaImplementation *rep, int index, const QVariant &property)
{
    Q_Q(QRemoteObjectNode);
    using namespace QRemoteObjectPackets;

    QVariant retval;

    Q_ASSERT(property.canConvert<QRO_>());
    QRO_ childInfo = property.value<QRO_>();
    qROPrivDebug() << "QRO_:" << childInfo.name << replicas.contains(childInfo.name) << replicas.keys();
    if (childInfo.isNull) {
        // Either the source has changed the pointer and we need to update it, or the source pointer is a nullptr
        if (replicas.contains(childInfo.name))
            replicas.remove(childInfo.name);
        if (childInfo.type == ObjectType::CLASS)
            retval = QVariant::fromValue<QRemoteObjectDynamicReplica*>(nullptr);
        else
            retval = QVariant::fromValue<QAbstractItemModelReplica*>(nullptr);
        return retval;
    }

    const bool newReplica = !replicas.contains(childInfo.name) || rep->isInitialized();
    if (newReplica) {
        if (rep->isInitialized()) {
            auto childRep = qSharedPointerCast<QConnectedReplicaImplementation>(replicas.take(childInfo.name));
            if (childRep && !childRep->isShortCircuit()) {
                qCDebug(QT_REMOTEOBJECT) << "Checking if dynamic type should be added to dynamicTypeManager (type =" << childRep->m_metaObject->className() << ")";
                dynamicTypeManager.addFromMetaObject(childRep->m_metaObject);
            }
        }
        if (childInfo.type == ObjectType::CLASS)
            retval = QVariant::fromValue(q->acquireDynamic(childInfo.name));
        else
            retval = QVariant::fromValue(q->acquireModel(childInfo.name));
    } else //We are receiving the initial data for the QObject
        retval = rep->getProperty(index); //Use existing value so changed signal isn't emitted

    QSharedPointer<QConnectedReplicaImplementation> childRep = qSharedPointerCast<QConnectedReplicaImplementation>(replicas.value(childInfo.name).toStrongRef());
    if (childRep->connectionToSource.isNull())
        childRep->connectionToSource = rep->connectionToSource;
    QVariantList parameters;
    QDataStream ds(childInfo.parameters);
    if (childRep->needsDynamicInitialization()) {
        if (childInfo.classDefinition.isEmpty()) {
            auto typeName = childInfo.typeName;
            if (typeName == QLatin1String("QObject")) {
                // The sender would have included the class name if needed
                // So the acquire must have been templated, and we have the typeName
                typeName = QString::fromLatin1(rep->getProperty(index).typeName());
                if (typeName.endsWith(QLatin1String("Replica*")))
                    typeName.chop(8);
            }
            childRep->setDynamicMetaObject(dynamicTypeManager.metaObjectForType(typeName));
        } else {
            QDataStream in(childInfo.classDefinition);
            childRep->setDynamicMetaObject(dynamicTypeManager.addDynamicType(rep->connectionToSource, in));
        }
        if (!childInfo.parameters.isEmpty())
            ds >> parameters;
        handlePointerToQObjectProperties(childRep.data(), parameters);
        childRep->setDynamicProperties(std::move(parameters));
    } else {
        if (!childInfo.parameters.isEmpty())
            ds >> parameters;
        handlePointerToQObjectProperties(childRep.data(), parameters);
        childRep->initialize(std::move(parameters));
    }

    return retval;
}

void QRemoteObjectNodePrivate::handlePointerToQObjectProperties(QConnectedReplicaImplementation *rep, QVariantList &properties)
{
    for (const int index : rep->childIndices())
        properties[index] = handlePointerToQObjectProperty(rep, index, properties.at(index));
}

/*!
    Blocks until this Node's \l Registry is initialized or \a timeout (in
    milliseconds) expires. Returns \c true if the \l Registry is successfully
    initialized upon return, or \c false otherwise.
*/
bool QRemoteObjectNode::waitForRegistry(int timeout)
{
    Q_D(QRemoteObjectNode);
    if (!d->registry) {
        qCWarning(QT_REMOTEOBJECT) << qPrintable(objectName()) << "waitForRegistry() error: No valid registry url set";
        return false;
    }

    return d->registry->waitForSource(timeout);
}

/*!
    Connects a client node to the host node at \a address.

    Connections will remain valid until the host node is deleted or no longer
    accessible over a network.

    Once a client is connected to a host, valid Replicas can then be acquired
    if the corresponding Source is being remoted.

    Return \c true on success, \c false otherwise (usually an unrecognized url,
    or connecting to already connected address).
*/
bool QRemoteObjectNode::connectToNode(const QUrl &address)
{
    Q_D(QRemoteObjectNode);
    if (!d->initConnection(address)) {
        d->setLastError(RegistryNotAcquired);
        return false;
    }
    return true;
}

/*!
    \since 5.12

    In order to \l QRemoteObjectNode::acquire() \l Replica objects over \l
    {External QIODevices}, Qt Remote Objects needs access to the communications
    channel (a \l QIODevice) between the respective nodes. It is the
    addClientSideConnection() call that enables this, taking the \a ioDevice as
    input. Any acquire() call made without calling addClientSideConnection will
    still work, but the Node will not be able to initialize the \l Replica
    without being provided the connection to the Host node.

    \sa {QRemoteObjectHostBase::addHostSideConnection}
*/
void QRemoteObjectNode::addClientSideConnection(QIODevice *ioDevice)
{
    Q_D(QRemoteObjectNode);
    if (!ioDevice || !ioDevice->isOpen()) {
        qWarning() << "A null or closed QIODevice was passed to addClientSideConnection().  Ignoring.";
        return;
    }
    QtROExternalIoDevice *device = new QtROExternalIoDevice(ioDevice, this);
    connect(device, &QtROIoDeviceBase::readyRead, this, [d, device]() {
        d->onClientRead(device);
    });
    if (device->bytesAvailable())
        d->onClientRead(device);
}

/*!
    \fn void QRemoteObjectNode::remoteObjectAdded(const QRemoteObjectSourceLocation &loc)

    This signal is emitted whenever a new \l {Source} object is added to
    the Registry. The signal will not be emitted if there is no Registry set
    (i.e., Sources over connections made via connectToNode directly). The \a
    loc parameter contains the information about the added Source, including
    name, type and the QUrl of the hosting Node.

    \sa remoteObjectRemoved(), instances()
*/

/*!
    \fn void QRemoteObjectNode::remoteObjectRemoved(const QRemoteObjectSourceLocation &loc)

    This signal is emitted whenever a known \l {Source} object is
    removed from the Registry. The signal will not be emitted if there is no
    Registry set (i.e., Sources over connections made via connectToNode
    directly). The \a loc parameter contains the information about the removed
    Source, including name, type and the QUrl of the hosting Node.

    \sa remoteObjectAdded, instances
*/

/*!
    \fn template<typename T> QStringList QRemoteObjectNode::instances() const

    This templated function (taking a \l repc generated type as the template parameter) will
    return the list of names of every instance of that type on the Remote
    Objects network. For example, if you have a Shape class defined in a .rep file,
    and Circle and Square classes inherit from the Source definition, they can
    be shared on the Remote Objects network using \l {QRemoteObjectHostBase::enableRemoting} {enableRemoting}.
    \code
        Square square;
        Circle circle;
        myHost.enableRemoting(&square, "Square");
        myHost.enableRemoting(&circle, "Circle");
    \endcode
    Then instance can be used to find the available instances of Shape.
    \code
        QStringList instances = clientNode.instances<Shape>();
        // will return a QStringList containing "Circle" and "Square"
        auto instance1 = clientNode.acquire<Shape>("Circle");
        auto instance2 = clientNode.acquire<Shape>("Square");
        ...
    \endcode
*/

/*!
    \overload instances()

    This convenience function provides the same result as the templated
    version, but takes the name of the \l {Source} class as a parameter (\a
    typeName) rather than deriving it from the class type.
*/
QStringList QRemoteObjectNode::instances(QStringView typeName) const
{
    Q_D(const QRemoteObjectNode);
    QStringList names;
    for (auto it = d->connectedSources.cbegin(), end = d->connectedSources.cend(); it != end; ++it) {
        if (it.value().typeName == typeName) {
            names << it.key();
        }
    }
    return names;
}

/*!
    \keyword dynamic acquire
    Returns a QRemoteObjectDynamicReplica of the Source \a name.
*/
QRemoteObjectDynamicReplica *QRemoteObjectNode::acquireDynamic(const QString &name)
{
    return new QRemoteObjectDynamicReplica(this, name);
}

/*!
    \qmlmethod bool Host::enableRemoting(object object, string name)
    Enables a host node to dynamically provide remote access to the QObject \a
    object. Client nodes connected to the node hosting this object may obtain
    Replicas of this Source.

    The optional \a name defines the lookup-name under which the QObject can be acquired
    using \l QRemoteObjectNode::acquire() . If not explicitly set then the name
    given in the QCLASSINFO_REMOTEOBJECT_TYPE will be used. If no such macro
    was defined for the QObject then the \l QObject::objectName() is used.

    Returns \c false if the current node is a client node, or if the QObject is already
    registered to be remoted, and \c true if remoting is successfully enabled
    for the dynamic QObject.

    \sa disableRemoting()
*/

/*!
    Enables a host node to dynamically provide remote access to the QObject \a
    object. Client nodes connected to the node
    hosting this object may obtain Replicas of this Source.

    The optional \a name defines the lookup-name under which the QObject can be acquired
    using \l QRemoteObjectNode::acquire() . If not explicitly set then the name
    given in the QCLASSINFO_REMOTEOBJECT_TYPE will be used. If no such macro
    was defined for the QObject then the \l QObject::objectName() is used.

    Returns \c false if the current node is a client node, or if the QObject is already
    registered to be remoted, and \c true if remoting is successfully enabled
    for the dynamic QObject.

    \sa disableRemoting()
*/
bool QRemoteObjectHostBase::enableRemoting(QObject *object, const QString &name)
{
    Q_D(QRemoteObjectHostBase);
    if (!d->remoteObjectIo) {
        d->setLastError(OperationNotValidOnClientNode);
        return false;
    }

    const QMetaObject *meta = object->metaObject();
    QString _name = name;
    QString typeName = getTypeNameAndMetaobjectFromClassInfo(meta);
    if (typeName.isEmpty()) { //This is a passed in QObject, use its API
        if (_name.isEmpty()) {
            _name = object->objectName();
            if (_name.isEmpty()) {
                d->setLastError(MissingObjectName);
                qCWarning(QT_REMOTEOBJECT) << qPrintable(objectName()) << "enableRemoting() Error: Unable to Replicate an object that does not have objectName() set.";
                return false;
            }
        }
    } else if (_name.isEmpty())
        _name = typeName;
    return d->remoteObjectIo->enableRemoting(object, meta, _name, typeName);
}

/*!
    This overload of enableRemoting() is specific to \l QAbstractItemModel types
    (or any type derived from \l QAbstractItemModel). This is useful if you want
    to have a model and the HMI for the model in different processes.

    The three required parameters are the \a model itself, the \a name by which
    to lookup the model, and the \a roles that should be exposed on the Replica
    side. If you want to synchronize selection between \l Source and \l
    Replica, the optional \a selectionModel parameter can be used. This is only
    recommended when using a single Replica.

    Behind the scenes, Qt Remote Objects batches data() lookups and prefetches
    data when possible to make the model interaction as responsive as possible.

    Returns \c false if the current node is a client node, or if the QObject is already
    registered to be remoted, and \c true if remoting is successfully enabled
    for the QAbstractItemModel.

    \sa disableRemoting()
 */
bool QRemoteObjectHostBase::enableRemoting(QAbstractItemModel *model, const QString &name, const QList<int> roles, QItemSelectionModel *selectionModel)
{
    //This looks complicated, but hopefully there is a way to have an adapter be a template
    //parameter and this makes sure that is supported.
    QObject *adapter = QAbstractItemModelSourceAdapter::staticMetaObject.newInstance(Q_ARG(QAbstractItemModel*, model),
                                                                                     Q_ARG(QItemSelectionModel*, selectionModel),
                                                                                     Q_ARG(QList<int>, roles));
    QAbstractItemAdapterSourceAPI<QAbstractItemModel, QAbstractItemModelSourceAdapter> *api =
        new QAbstractItemAdapterSourceAPI<QAbstractItemModel, QAbstractItemModelSourceAdapter>(name);
    if (!this->objectName().isEmpty())
        adapter->setObjectName(this->objectName().append(QLatin1String("Adapter")));
    return enableRemoting(model, api, adapter);
}

/*!
    \fn template <template <typename> class ApiDefinition, typename ObjectType> bool QRemoteObjectHostBase::enableRemoting(ObjectType *object)

    This templated function overload enables a host node to provide remote
    access to a QObject \a object with a specified (and compile-time checked)
    interface. Client nodes connected to the node hosting this object may
    obtain Replicas of this Source.

    This is best illustrated by example:
    \code
        #include "rep_TimeModel_source.h"
        MinuteTimer timer;
        hostNode.enableRemoting<MinuteTimerSourceAPI>(&timer);
    \endcode

    Here the MinuteTimerSourceAPI is the set of Signals/Slots/Properties
    defined by the TimeModel.rep file. Compile time checks are made to verify
    the input QObject can expose the requested API, it will fail to compile
    otherwise. This allows a subset of \a object 's interface to be exposed,
    and allows the types of conversions supported by Signal/Slot connections.

    Returns \c false if the current node is a client node, or if the QObject is
    already registered to be remoted, and \c true if remoting is successfully
    enabled for the QObject.

    \sa disableRemoting()
*/

/*!
    \internal
    Enables a host node to provide remote access to a QObject \a object
    with the API defined by \a api. Client nodes connected to the node
    hosting this object may obtain Replicas of this Source.

    Returns \c false if the current node is a client node, or if the QObject is
    already registered to be remoted, and \c true if remoting is successfully
    enabled for the QObject.

    \sa disableRemoting()
*/
bool QRemoteObjectHostBase::enableRemoting(QObject *object, const SourceApiMap *api, QObject *adapter)
{
    Q_D(QRemoteObjectHostBase);
    return d->remoteObjectIo->enableRemoting(object, api, adapter);
}

/*!
    \qmlmethod bool Host::disableRemoting(object remoteObject)
    Disables remote access for the QObject \a remoteObject. Returns \c false if
    the current node is a client node or if the \a remoteObject is not
    registered, and returns \c true if remoting is successfully disabled for
    the Source object.

    \warning Replicas of this object will no longer be valid after calling this method.

    \sa enableRemoting()
*/

/*!
    Disables remote access for the QObject \a remoteObject. Returns \c false if
    the current node is a client node or if the \a remoteObject is not
    registered, and returns \c true if remoting is successfully disabled for
    the Source object.

    \warning Replicas of this object will no longer be valid after calling this method.

    \sa enableRemoting()
*/
bool QRemoteObjectHostBase::disableRemoting(QObject *remoteObject)
{
    Q_D(QRemoteObjectHostBase);
    if (!d->remoteObjectIo) {
        d->setLastError(OperationNotValidOnClientNode);
        return false;
    }

    if (!d->remoteObjectIo->disableRemoting(remoteObject)) {
        d->setLastError(SourceNotRegistered);
        return false;
    }

    return true;
}

/*!
    \since 5.12

    In order to \l QRemoteObjectHost::enableRemoting() \l Source objects over
    \l {External QIODevices}, Qt Remote Objects needs access to the
    communications channel (a \l QIODevice) between the respective nodes. It is
    the addHostSideConnection() call that enables this on the \l Source side,
    taking the \a ioDevice as input. Any enableRemoting() call will still work
    without calling addHostSideConnection, but the Node will not be able to
    share the \l Source objects without being provided the connection to
    the Replica node. Before calling this function you must call
    \l {QRemoteObjectHost::}{setHostUrl}() with a unique URL and
    \l {QRemoteObjectHost::}{AllowExternalRegistration}.

    \sa addClientSideConnection
*/
void QRemoteObjectHostBase::addHostSideConnection(QIODevice *ioDevice)
{
    Q_D(QRemoteObjectHostBase);
    if (!ioDevice || !ioDevice->isOpen()) {
        qWarning() << "A null or closed QIODevice was passed to addHostSideConnection().  Ignoring.";
        return;
    }
    if (!d->remoteObjectIo)
        d->remoteObjectIo = new QRemoteObjectSourceIo(this);
    QtROExternalIoDevice *device = new QtROExternalIoDevice(ioDevice, this);
    return d->remoteObjectIo->newConnection(device);
}

/*!
    Returns a pointer to a \l Replica which is specifically derived from \l
    QAbstractItemModel. The \a name provided must match the name used with the
    matching \l {QRemoteObjectHostBase::}{enableRemoting} that put
    the \l Model on the network. \a action specifies whether the model should
    fetch data before the \l {QRemoteObjectReplica::}{initialized} signal is
    emitted. If it's set to QtRemoteObjects::PrefetchData, then the data for
    roles in the \a rolesHint will be prefetched. If \a rolesHint is empty, then
    the data for all the roles exposed by \l Source will be prefetched.

    The returned model will be empty until it is initialized with the \l Source.
*/
QAbstractItemModelReplica *QRemoteObjectNode::acquireModel(const QString &name, QtRemoteObjects::InitialAction action, const QList<int> &rolesHint)
{
    QAbstractItemModelReplicaImplementation *rep = acquire<QAbstractItemModelReplicaImplementation>(name);
    return new QAbstractItemModelReplica(rep, action, rolesHint);
}
/*!
 * \internal
 */
QRemoteObjectHostBasePrivate::QRemoteObjectHostBasePrivate()
    : QRemoteObjectNodePrivate()
    , remoteObjectIo(nullptr)
{ }

QRemoteObjectHostBasePrivate::~QRemoteObjectHostBasePrivate()
{ }

QRemoteObjectHostPrivate::QRemoteObjectHostPrivate()
    : QRemoteObjectHostBasePrivate()
{ }

QRemoteObjectHostPrivate::~QRemoteObjectHostPrivate()
{ }

QRemoteObjectRegistryHostPrivate::QRemoteObjectRegistryHostPrivate()
    : QRemoteObjectHostBasePrivate()
    , registrySource(nullptr)
{ }

QRemoteObjectRegistryHostPrivate::~QRemoteObjectRegistryHostPrivate()
{ }

ProxyInfo::ProxyInfo(QRemoteObjectNode *node, QRemoteObjectHostBase *parent,
                     QRemoteObjectHostBase::RemoteObjectNameFilter filter)
    : QObject(parent)
    , proxyNode(node)
    , parentNode(parent)
    , proxyFilter(filter)
{
    const auto registry = node->registry();
    proxyNode->setObjectName(QLatin1String("_ProxyNode"));

    connect(registry, &QRemoteObjectRegistry::remoteObjectAdded, this,
            [this](const QRemoteObjectSourceLocation &entry)
    {
        this->proxyObject(entry, ProxyDirection::Forward);
    });
    connect(registry, &QRemoteObjectRegistry::remoteObjectRemoved, this,
            &ProxyInfo::unproxyObject);
    connect(registry, &QRemoteObjectRegistry::initialized, this, [registry, this]() {
        QRemoteObjectSourceLocations locations = registry->sourceLocations();
        QRemoteObjectSourceLocations::const_iterator i = locations.constBegin();
        while (i != locations.constEnd()) {
            proxyObject(QRemoteObjectSourceLocation(i.key(), i.value()));
            ++i;
        }
    });

    connect(registry, &QRemoteObjectRegistry::stateChanged, this,
            [this](QRemoteObjectRegistry::State state, QRemoteObjectRegistry::State /*oldState*/) {
        if (state != QRemoteObjectRegistry::Suspect)
            return;
        // unproxy all objects
        for (ProxyReplicaInfo* info : proxiedReplicas)
            disableAndDeleteObject(info);
        proxiedReplicas.clear();
    });
}

ProxyInfo::~ProxyInfo() {
    for (ProxyReplicaInfo* info : proxiedReplicas)
        delete info;
    delete proxyNode;
}

bool ProxyInfo::setReverseProxy(QRemoteObjectHostBase::RemoteObjectNameFilter filter)
{
    if (qobject_cast<QRemoteObjectRegistryHost *>(parentNode) == nullptr) {
        qWarning() << "Setting up reverseProxy() can only be done on a Registry node.";
        return false;
    }
    const auto registry = parentNode->registry();
    this->reverseFilter = filter;

    connect(registry, &QRemoteObjectRegistry::remoteObjectAdded, this,
            [this](const QRemoteObjectSourceLocation &entry)
    {
        this->proxyObject(entry, ProxyDirection::Reverse);
    });
    connect(registry, &QRemoteObjectRegistry::remoteObjectRemoved, this,
            &ProxyInfo::unproxyObject);
    connect(registry, &QRemoteObjectRegistry::initialized, this, [registry, this]() {
        QRemoteObjectSourceLocations locations = registry->sourceLocations();
        QRemoteObjectSourceLocations::const_iterator i = locations.constBegin();
        while (i != locations.constEnd()) {
            proxyObject(QRemoteObjectSourceLocation(i.key(), i.value()), ProxyDirection::Reverse);
            ++i;
        }
    });

    return true;
}

void ProxyInfo::proxyObject(const QRemoteObjectSourceLocation &entry, ProxyDirection direction)
{
    const QString name = entry.first;
    const QString typeName = entry.second.typeName;

    if (direction == ProxyDirection::Forward) {
        // If we are using the reverse proxy, this can be called when reverse proxy objects are added
        // Don't try to proxy those back.  We can detect this because the hosting node will be our proxyNode.
        auto host = qobject_cast<QRemoteObjectHost *>(proxyNode);
        if (host && entry.second.hostUrl == host->hostUrl())
            return;
        if (!proxyFilter(name, typeName))
            return;
        Q_ASSERT(!proxiedReplicas.contains(name));

        qCDebug(QT_REMOTEOBJECT) << "Starting proxy for" << name << "from" << entry.second.hostUrl;

        if (entry.second.typeName == QAIMADAPTER()) {
            QAbstractItemModelReplica *rep = proxyNode->acquireModel(name);
            proxiedReplicas.insert(name, new ProxyReplicaInfo{rep, direction});
            connect(rep, &QAbstractItemModelReplica::initialized, this,
                    [rep, name, this]() { this->parentNode->enableRemoting(rep, name, QList<int>()); });
        } else {
            QRemoteObjectDynamicReplica *rep = proxyNode->acquireDynamic(name);
            proxiedReplicas.insert(name, new ProxyReplicaInfo{rep, direction});
            connect(rep, &QRemoteObjectDynamicReplica::initialized, this,
                    [rep, name, this]() { this->parentNode->enableRemoting(rep, name); });
        }
    } else {
        // If we are using the reverse proxy, this can be called when proxy objects are added
        // Don't try to proxy those back.  We can detect this because the hosting node will be the parentNode.
        // Since we know the parentNode has to be a RegistryNode for reverse proxy to work, we compare against
        // the registryUrl().
        if (entry.second.hostUrl == parentNode->registryUrl())
            return;
        if (!reverseFilter(name, typeName))
            return;
        Q_ASSERT(!proxiedReplicas.contains(name));

        qCDebug(QT_REMOTEOBJECT) << "Starting reverse proxy for" << name << "from" << entry.second.hostUrl;

        if (entry.second.typeName == QAIMADAPTER()) {
            QAbstractItemModelReplica *rep = this->parentNode->acquireModel(name);
            proxiedReplicas.insert(name, new ProxyReplicaInfo{rep, direction});
            connect(rep, &QAbstractItemModelReplica::initialized, this,
                    [rep, name, this]()
            {
                QRemoteObjectHostBase *host = qobject_cast<QRemoteObjectHostBase *>(this->proxyNode);
                Q_ASSERT(host);
                host->enableRemoting(rep, name, QList<int>());
            });
        } else {
            QRemoteObjectDynamicReplica *rep = this->parentNode->acquireDynamic(name);
            proxiedReplicas.insert(name, new ProxyReplicaInfo{rep, direction});
            connect(rep, &QRemoteObjectDynamicReplica::initialized, this,
                    [rep, name, this]()
            {
                QRemoteObjectHostBase *host = qobject_cast<QRemoteObjectHostBase *>(this->proxyNode);
                Q_ASSERT(host);
                host->enableRemoting(rep, name);
            });
        }
    }

}

void ProxyInfo::unproxyObject(const QRemoteObjectSourceLocation &entry)
{
    const QString name = entry.first;

    if (proxiedReplicas.contains(name)) {
        qCDebug(QT_REMOTEOBJECT)  << "Stopping proxy for" << name;
        auto const info = proxiedReplicas.take(name);
        disableAndDeleteObject(info);
    }
}

void ProxyInfo::disableAndDeleteObject(ProxyReplicaInfo* info)
{
    if (info->direction == ProxyDirection::Forward)
        this->parentNode->disableRemoting(info->replica);
    else {
        QRemoteObjectHostBase *host = qobject_cast<QRemoteObjectHostBase *>(this->proxyNode);
        Q_ASSERT(host);
        host->disableRemoting(info->replica);
    }
    delete info;
}


QT_END_NAMESPACE

#include "moc_qremoteobjectnode.cpp"
