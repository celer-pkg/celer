// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qremoteobjectsource.h"
#include "qremoteobjectsource_p.h"
#include "qremoteobjectnode.h"
#include "qremoteobjectdynamicreplica.h"

#include "qconnectionfactories_p.h"
#include "qremoteobjectsourceio_p.h"
#include "qremoteobjectabstractitemmodeladapter_p.h"

#include <QtCore/qmetaobject.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qabstractitemmodel.h>

#include <algorithm>
#include <iterator>

QT_BEGIN_NAMESPACE

using namespace QRemoteObjectPackets;
using namespace QRemoteObjectStringLiterals;

const int QRemoteObjectSourceBase::qobjectPropertyOffset = QObject::staticMetaObject.propertyCount();
const int QRemoteObjectSourceBase::qobjectMethodOffset = QObject::staticMetaObject.methodCount();
static const QByteArray s_classinfoRemoteobjectSignature(QCLASSINFO_REMOTEOBJECT_SIGNATURE);

namespace QtPrivate {

// The stringData, methodMatch and QMetaObjectPrivate methods are modified versions of the code
// from qmetaobject_p.h/qmetaobject.cpp.  The modifications are based on our custom need to match
// a method name that comes from the .rep file.
// The QMetaObjectPrivate struct should only have members appended to maintain binary compatibility,
// so we should be fine with only the listed version with the fields we use.
inline const QByteArray apiStringData(const QMetaObject *mo, int index)
{
    uint offset = mo->d.stringdata[2*index];
    uint length = mo->d.stringdata[2*index + 1];
    const char *string = reinterpret_cast<const char *>(mo->d.stringdata) + offset;
    return QByteArray::fromRawData(string, length);
}


// From QMetaMethod in qmetaobject.h
struct Data {
    enum { Size = 6 };

    uint name() const { return d[0]; }
    uint argc() const { return d[1]; }
    uint parameters() const { return d[2]; }
    uint tag() const { return d[3]; }
    uint flags() const { return d[4]; }
    uint metaTypeOffset() const { return d[5]; }
    bool operator==(const Data &other) const { return d == other.d; }

    const uint *d;
};

inline bool apiMethodMatch(const QMetaObject *m, const Data &data, const QByteArray &name, int argc,
                           const int *types)
{
    if (data.argc() != uint(argc))
        return false;
    if (apiStringData(m, data.name()) != name)
        return false;
    for (int i = 0; i < argc; ++i) {
        auto mt = QMetaType(m->d.metaTypes[data.metaTypeOffset() + i + 1]);
        if (mt != QMetaType(types[i]))
            return false;
    }
    return true;
}

struct QMetaObjectPrivate
{
    // revision 7 is Qt 5.0 everything lower is not supported
    // revision 8 is Qt 5.12: It adds the enum name to QMetaEnum
    enum { OutputRevision = 8 }; // Used by moc, qmetaobjectbuilder and qdbus

    int revision;
    int className;
    int classInfoCount, classInfoData;
    int methodCount, methodData;
    int propertyCount, propertyData;
    int enumeratorCount, enumeratorData;
    int constructorCount, constructorData;
    int flags;
    int signalCount;
};

inline Data fromRelativeMethodIndex(const QMetaObject *mobj, int index)
{
    const auto priv = reinterpret_cast<const QMetaObjectPrivate*>(mobj->d.data);
    return { mobj->d.data + priv->methodData + index * Data::Size };
}

int qtro_method_index_impl(const QMetaObject * staticMetaObj, const char *className,
                                      const char *methodName, int *count, int const **types)
{
    int result = staticMetaObj->indexOfMethod(methodName);
    if (result >= 0)
        return result;
    // We can have issues, specifically with enums, since the compiler can infer the class.  Since
    // indexOfMethod() is doing string comparisons for registered types, "MyEnum" and "MyClass::MyEnum"
    // won't match.
    // Below is similar to QMetaObject->indexOfMethod, but template magic has already matched parameter
    // types, so we need to find a match for the API method name + parameters.  Neither approach works
    // 100%, as the below code doesn't match a parameter of type "size_t" (which the template match
    // identifies as "ulong").  These subtleties can cause the below string comparison fails.
    // There is no known case that would fail both methods.
    // TODO: is there a way to make this a constexpr so a failure is detected at compile time?
    int nameLength = strchr(methodName, '(') - methodName;
    const auto name = QByteArray::fromRawData(methodName, nameLength);
    for (const QMetaObject *m = staticMetaObj; m; m = m->d.superdata) {
        const auto priv = reinterpret_cast<const QMetaObjectPrivate*>(m->d.data);
        int i = (priv->methodCount - 1);
        const int end = priv->signalCount;
        for (; i >= end; --i) {
            const Data data = fromRelativeMethodIndex(m, i);
            if (apiMethodMatch(m, data, name, *count, *types))
                return i + m->methodOffset();
        }
    }
    qWarning() << "No matching method for" << methodName << "in the provided metaclass" << className;
    return -1;
}

} // namespace QtPrivate

QByteArray QtPrivate::qtro_classinfo_signature(const QMetaObject *metaObject)
{
    if (!metaObject)
        return QByteArray{};

    for (int i = metaObject->classInfoOffset(); i < metaObject->classInfoCount(); ++i) {
        auto ci = metaObject->classInfo(i);
        if (s_classinfoRemoteobjectSignature == ci.name())
            return ci.value();
    }
    return QByteArray{};
}

inline bool qtro_is_cloned_method(const QMetaObject *mobj, int index)
{
    int local_method_index = index - mobj->methodOffset();
    if (local_method_index < 0 && mobj->superClass())
        return qtro_is_cloned_method(mobj->superClass(), index);
    const QtPrivate::Data data = QtPrivate::fromRelativeMethodIndex(mobj, local_method_index);
    if (data.flags() & 0x20 /*MethodFlags::MethodCloned*/)
        return true;
    return false;
}

SourceApiMap::~SourceApiMap()
    = default;

QRemoteObjectSourceBase::QRemoteObjectSourceBase(QObject *obj, Private *d, const SourceApiMap *api,
                                                 QObject *adapter)
    : QObject(obj),
      m_object(obj),
      m_adapter(adapter),
      m_api(api),
      d(d)
{
    if (!obj) {
        qCWarning(QT_REMOTEOBJECT) << "QRemoteObjectSourceBase: Cannot replicate a NULL object" << m_api->name();
        return;
    }

    setConnections();

    const auto nChildren = api->m_models.size() + api->m_subclasses.size();
    if (nChildren > 0) {
        QList<int> roles;
        const int numProperties = api->propertyCount();
        int modelIndex = 0, subclassIndex = 0;
        for (int i = 0; i < numProperties; ++i) {
            if (api->isAdapterProperty(i))
                continue;
            const int index = api->sourcePropertyIndex(i);
            const auto property = m_object->metaObject()->property(index);
            const auto metaType = property.metaType();
            if (metaType.flags().testFlag(QMetaType::PointerToQObject)) {
                auto propertyMeta = metaType.metaObject();
                QObject *child = property.read(m_object).value<QObject *>();
                const QMetaObject *meta = child ? child->metaObject() : propertyMeta;
                if (!meta)
                    continue;
                if (meta->inherits(&QAbstractItemModel::staticMetaObject)) {
                    const auto modelInfo = api->m_models.at(modelIndex++);
                    QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(child);
                    QAbstractItemAdapterSourceAPI<QAbstractItemModel, QAbstractItemModelSourceAdapter> *modelApi =
                        new QAbstractItemAdapterSourceAPI<QAbstractItemModel, QAbstractItemModelSourceAdapter>(modelInfo.name);
                    if (!model)
                        m_children.insert(i, new QRemoteObjectSource(nullptr, d, modelApi, nullptr, api->name()));
                    else {
                        roles.clear();
                        const auto knownRoles = model->roleNames();
                        for (auto role : modelInfo.roles.split('|')) {
                            if (role.isEmpty())
                                continue;
                            const int roleIndex = knownRoles.key(role, -1);
                            if (roleIndex == -1) {
                                qCWarning(QT_REMOTEOBJECT) << "Invalid role" << role << "for model" << model->metaObject()->className();
                                qCWarning(QT_REMOTEOBJECT) << "  known roles:" << knownRoles;
                            } else
                                roles << roleIndex;
                        }
                        auto adapter = new QAbstractItemModelSourceAdapter(model, nullptr,
                                                                           roles.isEmpty() ? knownRoles.keys().toVector() : roles);
                        m_children.insert(i, new QRemoteObjectSource(model, d, modelApi, adapter, api->name()));
                    }
                } else {
                    const auto classApi = api->m_subclasses.at(subclassIndex++);
                    m_children.insert(i, new QRemoteObjectSource(child, d, classApi, nullptr, api->name()));
                }
            }
        }
    }
}

QRemoteObjectSource::QRemoteObjectSource(QObject *obj, Private *dd, const SourceApiMap *api, QObject *adapter, const QString &parentName)
    : QRemoteObjectSourceBase(obj, dd, api, adapter)
    , m_name(api->typeName() == QLatin1String("QAbstractItemModelAdapter") ? MODEL().arg(parentName + QLatin1String("::") + api->name()) :
                                                                             CLASS().arg(parentName + QLatin1String("::") + api->name()))
{
    if (obj)
        d->m_sourceIo->registerSource(this);
}

QRemoteObjectRootSource::QRemoteObjectRootSource(QObject *obj, const SourceApiMap *api,
                                                 QObject *adapter, QRemoteObjectSourceIo *sourceIo)
    : QRemoteObjectSourceBase(obj, new Private(sourceIo, this), api, adapter)
    , m_name(api->name())
{
    d->m_sourceIo->registerSource(this);
}

QRemoteObjectSourceBase::~QRemoteObjectSourceBase()
{
    delete m_api;
}

void QRemoteObjectSourceBase::setConnections()
{
    const QMetaObject *meta = m_object->metaObject();

    const int index = meta->indexOfClassInfo(QCLASSINFO_REMOTEOBJECT_TYPE);
    if (index != -1) { //We have an object created from repc or at least with QCLASSINFO defined
        while (true) {
            Q_ASSERT(meta->superClass()); //This recurses to QObject, which doesn't have QCLASSINFO_REMOTEOBJECT_TYPE
            if (index != meta->superClass()->indexOfClassInfo(QCLASSINFO_REMOTEOBJECT_TYPE)) //At the point we don't find the same QCLASSINFO_REMOTEOBJECT_TYPE,
                            //we have the metaobject we should work from
                break;
            meta = meta->superClass();
        }
    }

    for (int idx = 0; idx < m_api->signalCount(); ++idx) {
        const int sourceIndex = m_api->sourceSignalIndex(idx);
        const bool isAdapter = m_api->isAdapterSignal(idx);
        const auto targetMeta = isAdapter ? m_adapter->metaObject() : meta;

        // don't connect cloned signals, or we end up with multiple emissions
        if (qtro_is_cloned_method(targetMeta, sourceIndex))
            continue;
        // This basically connects the parent Signals (note, all dynamic properties have onChange
        //notifications, thus signals) to us.  Normally each Signal is mapped to a unique index,
        //but since we are forwarding them all, we keep the offset constant.
        //
        //We know no one will inherit from this class, so no need to worry about indices from
        //derived classes.
        const auto target = isAdapter ? m_adapter : m_object;
        if (!QMetaObject::connect(target, sourceIndex, this, QRemoteObjectSource::qobjectMethodOffset+idx, Qt::DirectConnection, nullptr)) {
            qCWarning(QT_REMOTEOBJECT) << "QRemoteObjectSourceBase: QMetaObject::connect returned false. Unable to connect.";
            return;
        }

        qCDebug(QT_REMOTEOBJECT) << "Connection made" << idx << sourceIndex
                                 << targetMeta->method(sourceIndex).name();
    }
}

void QRemoteObjectSourceBase::resetObject(QObject *newObject)
{
    if (m_object)
        m_object->disconnect(this);
    if (m_adapter) {
        m_adapter->disconnect(this);
        delete m_adapter;
        m_adapter = nullptr;
    }
    // We need some dynamic replica specific code here, in case an object had null sub-classes that
    // have been replaced with real objects.  In this case, the ApiMap could be wrong and need updating.
    if (newObject && qobject_cast<QRemoteObjectDynamicReplica *>(newObject) && m_api->isDynamic()) {
        auto api = static_cast<const DynamicApiMap*>(m_api);
        if (api->m_properties[0] == 0) { // 0 is an index into QObject itself, so this isn't a valid QtRO index
            const auto rep = qobject_cast<QRemoteObjectDynamicReplica *>(newObject);
            auto tmp = m_api;
            m_api = new DynamicApiMap(newObject, rep->metaObject(), api->m_name, QLatin1String(rep->metaObject()->className()));
            qCDebug(QT_REMOTEOBJECT) << "  Reset m_api for" << api->m_name << "using new metaObject:" << rep->metaObject()->className();
            delete tmp;
        }
    }

    m_object = newObject;
    auto model = qobject_cast<QAbstractItemModel *>(newObject);
    if (model) {
        d->m_sourceIo->registerSource(this);
        m_adapter = new QAbstractItemModelSourceAdapter(model, nullptr, model->roleNames().keys().toVector());
    }

    setParent(newObject);
    if (newObject)
        setConnections();

    const auto nChildren = m_api->m_models.size() + m_api->m_subclasses.size();
    if (nChildren == 0)
        return;

    if (!newObject) {
        for (auto child : m_children)
            child->resetObject(nullptr);
        return;
    }

    for (int i : m_children.keys()) {
        const int index = m_api->sourcePropertyIndex(i);
        const auto property = m_object->metaObject()->property(index);
        QObject *child = property.read(m_object).value<QObject *>();
        m_children[i]->resetObject(child);
    }
}

QRemoteObjectSource::~QRemoteObjectSource()
{
    for (auto it : m_children) {
        // We used QPointers for m_children because we don't control the lifetime of child QObjects
        // Since the this/source QObject's parent is the referenced QObject, it could have already
        // been deleted
        delete it;
    }
}

QRemoteObjectRootSource::~QRemoteObjectRootSource()
{
    for (auto it : m_children) {
        // We used QPointers for m_children because we don't control the lifetime of child QObjects
        // Since the this/source QObject's parent is the referenced QObject, it could have already
        // been deleted
        delete it;
    }
    d->m_sourceIo->unregisterSource(this);
    // removeListener tries to modify d->m_listeners, this is O(N²),
    // so clear d->m_listeners prior to calling unregister (consume loop).
    // We can do this, because we don't care about the return value of removeListener() here.
    for (QtROIoDeviceBase *io : std::exchange(d->m_listeners, {})) {
        removeListener(io, true);
    }
    delete d;
}

QVariantList* QRemoteObjectSourceBase::marshalArgs(int index, void **a)
{
    QVariantList &list = m_marshalledArgs;
    int N = m_api->signalParameterCount(index);
    if (N == 1 && QMetaType(m_api->signalParameterType(index, 0)).flags().testFlag(QMetaType::PointerToQObject))
        N = 0; // Don't try to send pointers, the will be handle by QRO_
    if (list.size() < N)
        list.reserve(N);
    const int minFill = std::min(int(list.size()), N);
    for (int i = 0; i < minFill; ++i) {
        const int type = m_api->signalParameterType(index, i);
        if (type == QMetaType::QVariant)
            list[i] = *reinterpret_cast<QVariant *>(a[i + 1]);
        else
            list[i] = QVariant(QMetaType(type), a[i + 1]);
    }
    for (int i = int(list.size()); i < N; ++i) {
        const int type = m_api->signalParameterType(index, i);
        if (type == QMetaType::QVariant)
            list << *reinterpret_cast<QVariant *>(a[i + 1]);
        else
            list << QVariant(QMetaType(type), a[i + 1]);
    }
    for (int i = N; i < list.size(); ++i)
        list.removeLast();
    return &m_marshalledArgs;
}

bool QRemoteObjectSourceBase::invoke(QMetaObject::Call c, int index, const QVariantList &args, QVariant* returnValue)
{
    int status = -1;
    int flags = 0;
    bool forAdapter = (c == QMetaObject::InvokeMetaMethod ? m_api->isAdapterMethod(index) : m_api->isAdapterProperty(index));
    int resolvedIndex = (c == QMetaObject::InvokeMetaMethod ? m_api->sourceMethodIndex(index) : m_api->sourcePropertyIndex(index));
    if (resolvedIndex < 0)
        return false;
    QVarLengthArray<void*, 10> param(args.size() + 1);

    if (c == QMetaObject::InvokeMetaMethod) {
        QMetaMethod method;
        if (!forAdapter)
            method = parent()->metaObject()->method(resolvedIndex);

        if (returnValue) {
            if (!forAdapter && method.isValid() && method.returnType() == QMetaType::QVariant)
                param[0] = const_cast<void*>(reinterpret_cast<const void*>(returnValue));
            else
                param[0] = returnValue->data();
        } else {
            param[0] = nullptr;
        }

        auto argument = [&](int i) -> void * {
            if ((forAdapter && m_api->methodParameterType(index, i) == QMetaType::QVariant) ||
                    (method.isValid() && method.parameterType(i) == QMetaType::QVariant)) {
                return const_cast<void*>(reinterpret_cast<const void*>(&args.at(i)));
            }
            return const_cast<void*>(args.at(i).data());
        };

        for (int i = 0; i < args.size(); ++i) {
            param[i + 1] = argument(i);
        }
    } else if (c == QMetaObject::WriteProperty || c == QMetaObject::ReadProperty) {
        bool isQVariant = !forAdapter && parent()->metaObject()->property(resolvedIndex).userType() == QMetaType::QVariant;
        for (int i = 0; i < args.size(); ++i) {
            if (isQVariant)
                param[i] = const_cast<void*>(reinterpret_cast<const void*>(&args.at(i)));
            else
                param[i] = const_cast<void*>(args.at(i).data());
        }
        if (c == QMetaObject::WriteProperty) {
            Q_ASSERT(param.size() == 2); // for return-value and setter value
            // check QMetaProperty::write for an explanation of these
            param.append(&status);
            param.append(&flags);
        }
    } else {
        // Better safe than sorry
        return false;
    }
    int r = -1;
    if (forAdapter)
        r = m_adapter->qt_metacall(c, resolvedIndex, param.data());
    else
        r = parent()->qt_metacall(c, resolvedIndex, param.data());
    return r == -1 && status == -1;
}

void QRemoteObjectSourceBase::handleMetaCall(int index, QMetaObject::Call call, void **a)
{
    if (d->m_listeners.empty())
        return;

    int propertyIndex = m_api->propertyIndexFromSignal(index);
    if (propertyIndex >= 0) {
        const int internalIndex = m_api->propertyRawIndexFromSignal(index);
        const auto target = m_api->isAdapterProperty(internalIndex) ? m_adapter : m_object;
        const QMetaProperty mp = target->metaObject()->property(propertyIndex);
        qCDebug(QT_REMOTEOBJECT) << "Sending Invoke Property" << (m_api->isAdapterSignal(internalIndex) ? "via adapter" : "") << internalIndex << propertyIndex << mp.name() << mp.read(target);

        d->codec->serializePropertyChangePacket(this, index);
        propertyIndex = internalIndex;
    }

    qCDebug(QT_REMOTEOBJECT) << "# Listeners" << d->m_listeners.size();
    qCDebug(QT_REMOTEOBJECT) << "Invoke args:" << m_object
                             << (call == 0 ? QLatin1String("InvokeMetaMethod") : QStringLiteral("Non-invoked call: %d").arg(call))
                             << m_api->signalSignature(index) << *marshalArgs(index, a);

    d->codec->serializeInvokePacket(name(), call, index, *marshalArgs(index, a), -1, propertyIndex);

    d->codec->send(d->m_listeners);
}

void QRemoteObjectRootSource::addListener(QtROIoDeviceBase *io, bool dynamic)
{
    d->m_listeners.append(io);
    d->isDynamic = d->isDynamic || dynamic;

    if (dynamic) {
        d->sentTypes.clear();
        d->codec->serializeInitDynamicPacket(this);
        d->codec->send(io);
    } else {
        d->codec->serializeInitPacket(this);
        d->codec->send(io);
    }
}

int QRemoteObjectRootSource::removeListener(QtROIoDeviceBase *io, bool shouldSendRemove)
{
    d->m_listeners.removeAll(io);
    if (shouldSendRemove)
    {
        d->codec->serializeRemoveObjectPacket(m_api->name());
        d->codec->send(io);
    }
    return int(d->m_listeners.size());
}

int QRemoteObjectSourceBase::qt_metacall(QMetaObject::Call call, int methodId, void **a)
{
    methodId = QObject::qt_metacall(call, methodId, a);
    if (methodId < 0)
        return methodId;

    if (call == QMetaObject::InvokeMetaMethod)
        handleMetaCall(methodId, call, a);

    return -1;
}

DynamicApiMap::DynamicApiMap(QObject *object, const QMetaObject *metaObject, const QString &name, const QString &typeName)
    : m_name(name),
      m_typeName(typeName),
      m_metaObject(metaObject),
      m_cachedMetamethodIndex(-1)
{
    m_enumOffset = metaObject->enumeratorOffset();
    m_enumCount = metaObject->enumeratorCount() - m_enumOffset;

    const int propCount = metaObject->propertyCount();
    const int propOffset = metaObject->propertyOffset();
    m_properties.reserve(propCount-propOffset);
    QSet<int> invalidSignals;
    for (int i = propOffset; i < propCount; ++i) {
        const QMetaProperty property = metaObject->property(i);
        const auto metaType = property.metaType();
        if (metaType.flags().testFlag(QMetaType::PointerToQObject)) {
            auto propertyMeta = metaType.metaObject();
            QObject *child = property.read(object).value<QObject *>();
            const QMetaObject *meta = child ? child->metaObject() : propertyMeta;
            if (!meta) {
                const int notifyIndex = metaObject->property(i).notifySignalIndex();
                if (notifyIndex != -1)
                    invalidSignals << notifyIndex;
                continue;
            }
            if (meta->inherits(&QAbstractItemModel::staticMetaObject)) {
                const QByteArray name = QByteArray::fromRawData(property.name(),
                                                                qsizetype(qstrlen(property.name())));
                const QByteArray infoName = name.toUpper() + QByteArrayLiteral("_ROLES");
                const int infoIndex = metaObject->indexOfClassInfo(infoName.constData());
                QByteArray roleInfo;
                if (infoIndex >= 0) {
                    auto ci = metaObject->classInfo(infoIndex);
                    roleInfo = QByteArray::fromRawData(ci.value(), qsizetype(qstrlen(ci.value())));
                }
                m_models << ModelInfo({qobject_cast<QAbstractItemModel *>(child),
                                       QString::fromLatin1(property.name()),
                                       roleInfo});
            } else {
                QString typeName = QtRemoteObjects::getTypeNameAndMetaobjectFromClassInfo(meta);
                if (typeName.isNull()) {
                    typeName = QString::fromLatin1(meta->className());
                    if (typeName.contains(QLatin1String("QQuick")))
                        typeName.remove(QLatin1String("QQuick"));
                    else if (int index = typeName.indexOf(QLatin1String("_QMLTYPE_")))
                        typeName.truncate(index);
                    // TODO better way to ensure we have consistent typenames between source/replicas?
                    else if (typeName.endsWith(QLatin1String("Source")))
                        typeName.chop(6);
                }

                m_subclasses << new DynamicApiMap(child, meta, QString::fromLatin1(property.name()), typeName);
            }
        }
        m_properties << i;
        const int notifyIndex = metaObject->property(i).notifySignalIndex();
        if (notifyIndex != -1) {
            m_signals << notifyIndex;
            m_propertyAssociatedWithSignal.append(i-propOffset);
            //The starting values of _signals will be the notify signals
            //So if we are processing _signal with index i, api->sourcePropertyIndex(_propertyAssociatedWithSignal.at(i))
            //will be the property that changed.  This is only valid if i < _propertyAssociatedWithSignal.size().
        }
    }
    const int methodCount = metaObject->methodCount();
    const int methodOffset = metaObject->methodOffset();
    for (int i = methodOffset; i < methodCount; ++i) {
        const QMetaMethod mm = metaObject->method(i);
        const QMetaMethod::MethodType m = mm.methodType();
        if (m == QMetaMethod::Signal) {
            if (m_signals.indexOf(i) >= 0)  // Already added as a property notifier
                continue;
            if (invalidSignals.contains(i)) // QObject with no metatype
                continue;
            m_signals << i;
        } else if (m == QMetaMethod::Slot || m == QMetaMethod::Method)
            m_methods << i;
    }

    m_objectSignature = QtPrivate::qtro_classinfo_signature(metaObject);
}

QByteArrayList DynamicApiMap::signalParameterNames(int index) const
{
    const int objectIndex = m_signals.at(index);
    checkCache(objectIndex);
    return m_cachedMetamethod.parameterNames();
}

int DynamicApiMap::parameterCount(int objectIndex) const
{
    checkCache(objectIndex);
    return m_cachedMetamethod.parameterCount();
}

int DynamicApiMap::parameterType(int objectIndex, int paramIndex) const
{
    checkCache(objectIndex);
    return m_cachedMetamethod.parameterType(paramIndex);
}

const QByteArray DynamicApiMap::signature(int objectIndex) const
{
    checkCache(objectIndex);
    return m_cachedMetamethod.methodSignature();
}

QMetaMethod::MethodType DynamicApiMap::methodType(int index) const
{
    const int objectIndex = m_methods.at(index);
    checkCache(objectIndex);
    return m_cachedMetamethod.methodType();
}

const QByteArray DynamicApiMap::typeName(int index) const
{
    const int objectIndex = m_methods.at(index);
    checkCache(objectIndex);
    return m_cachedMetamethod.typeName();
}

QByteArrayList DynamicApiMap::methodParameterNames(int index) const
{
    const int objectIndex = m_methods.at(index);
    checkCache(objectIndex);
    return m_cachedMetamethod.parameterNames();
}

QRemoteObjectSourceBase::Private::Private(QRemoteObjectSourceIo *io, QRemoteObjectRootSource *root)
    : m_sourceIo(io), codec(io->m_codec.data()), isDynamic(false), root(root)
{
}

QT_END_NAMESPACE
