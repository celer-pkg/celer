// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qbytearrayview.h>

#include "qremoteobjectcontainers_p.h"
#include "qremoteobjectpendingcall.h"
#include "qremoteobjectsource.h"
#include "qremoteobjectsource_p.h"
#include "qremoteobjectpacket_p.h"
#include "qconnectionfactories.h"
#include "qconnectionfactories_p.h"
#include <cstring>

//#define QTRO_VERBOSE_PROTOCOL
QT_BEGIN_NAMESPACE


// Add methods so we can use QMetaEnum in a set
// Note for both functions we are skipping string comparisons/hashes.  Since the
// metaObjects are the same, we can just use the address of the string.
inline bool operator==(const QMetaEnum e1, const QMetaEnum e2)
{
    return e1.enclosingMetaObject() == e2.enclosingMetaObject()
           && e1.name() == e2.name()
           && e1.enumName() == e2.enumName()
           && e1.scope() == e2.scope();
}

inline size_t qHash(const QMetaEnum &key, size_t seed=0) Q_DECL_NOTHROW
{
    return qHash(key.enclosingMetaObject(), seed) ^ qHash(static_cast<const void *>(key.name()), seed)
           ^ qHash(static_cast<const void *>(key.enumName()), seed) ^ qHash(static_cast<const void *>(key.scope()), seed);
}

static bool isSequentialGadgetType(QMetaType metaType)
{
    if (QMetaType::canConvert(metaType, QMetaType::fromType<QMetaSequence::Iterable>())) {
        static QHash<int, bool> lookup;
        if (!lookup.contains(metaType.id())) {
            auto stubVariant = QVariant(metaType, nullptr);
            auto asIterable = stubVariant.value<QMetaSequence::Iterable>();
            auto valueMetaType = asIterable.metaContainer().valueMetaType();
            lookup[metaType.id()] = valueMetaType.flags().testFlag(QMetaType::IsGadget);
        }
        return lookup[metaType.id()];
    }
    return false;
}

static bool isAssociativeGadgetType(QMetaType metaType)
{
    if (QMetaType::canConvert(metaType, QMetaType::fromType<QMetaAssociation::Iterable>())) {
        static QHash<int, bool> lookup;
        if (!lookup.contains(metaType.id())) {
            auto stubVariant = QVariant(metaType, nullptr);
            auto asIterable = stubVariant.value<QMetaAssociation::Iterable>();
            auto valueMetaType = asIterable.metaContainer().mappedMetaType();
            lookup[metaType.id()] = valueMetaType.flags().testFlag(QMetaType::IsGadget);
        }
        return lookup[metaType.id()];
    }
    return false;
}

using namespace QtRemoteObjects;

namespace QRemoteObjectPackets {

QMetaType transferTypeForEnum(QMetaType enumType)
{
    const auto size = enumType.sizeOf();
    switch (size) {
    case 1: return QMetaType::fromType<qint8>();
    case 2: return QMetaType::fromType<qint16>();
    case 4: return QMetaType::fromType<qint32>();
    // Qt currently only supports enum values of 4 or less bytes (QMetaEnum value(index) returns int)
//                    case 8: args.push_back(QVariant(QMetaType::Int, argv[i + 1])); break;
    default:
        qCWarning(QT_REMOTEOBJECT_IO) << "Invalid enum detected (Dynamic Replica)" << enumType.name() << "with size" << size;
        return QMetaType::fromType<qint32>();
    }
}

// QDataStream sends QVariants of custom types by sending their typename, allowing decode
// on the receiving side.  For QtRO and enums, this won't work, as the enums have different
// scopes.  E.g., the examples have ParentClassSource::MyEnum and ParentClassReplica::MyEnum.
// Dynamic types will be created as ParentClass::MyEnum.  So instead, we change the variants
// to integers (encodeVariant) when sending them.  On the receive side, the we know the
// types of properties and the signatures for methods, so we can use that information to
// decode the integer variant into an enum variant (via decodeVariant).
QVariant encodeVariant(const QVariant &value)
{
    const auto metaType = value.metaType();
    if (metaType.flags().testFlag(QMetaType::IsEnumeration)) {
        auto converted = QVariant(value);
        auto transferType = transferTypeForEnum(metaType);
        converted.convert(transferType);
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug() << "Converting from enum to integer type" << transferType.sizeOf() << converted << value;
#endif
        return converted;
    }
    if (isSequentialGadgetType(metaType)) { // Doesn't include QtROSequentialContainer
        // TODO Way to create the QVariant without copying the QSQ_?
        QSQ_ sequence(value);
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug() << "Encoding sequential container" << metaType.name() << "to QSQ_ to transmit";
#endif
        return QVariant::fromValue<QSQ_>(sequence);
    }
    if (metaType == QMetaType::fromType<QtROSequentialContainer>()) {
        QSQ_ sequence(value);
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug() << "Encoding QtROSequentialContainer container to QSQ_ to transmit";
#endif
        return QVariant::fromValue<QSQ_>(sequence);
    }
    if (isAssociativeGadgetType(metaType)) { // Doesn't include QtROAssociativeContainer
        QAS_ map(value);
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug() << "Encoding associative container" << metaType.name() << "to QAS_ to transmit";
#endif
        return QVariant::fromValue<QAS_>(map);
    }
    if (metaType == QMetaType::fromType<QtROAssociativeContainer>()) {
        QAS_ map(value);
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug() << "Encoding QtROAssociativeContainer container to QAS_ to transmit";
#endif
        return QVariant::fromValue<QAS_>(map);
    }
    return value;
}

QVariant decodeVariant(QVariant &&value, QMetaType metaType)
{
    if (metaType.flags().testFlag(QMetaType::IsEnumeration)) {
#ifdef QTRO_VERBOSE_PROTOCOL
        QVariant encoded(value);
#endif
        value.convert(metaType);
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug() << "Converting to enum from integer type" << value << encoded;
#endif
    } else if (value.metaType() == QMetaType::fromType<QRemoteObjectPackets::QSQ_>()) {
        const auto *qsq_ = static_cast<const QRemoteObjectPackets::QSQ_ *>(value.constData());
        QDataStream in(qsq_->values);
        auto containerType = QMetaType::fromName(qsq_->typeName.constData());
        bool isRegistered = containerType.isRegistered();
        if (isRegistered) {
            QVariant seq{containerType, nullptr};
            if (!seq.canView<QMetaSequence::Iterable>()) {
                qWarning() << "Unsupported container" << qsq_->typeName.constData()
                           << "(not viewable)";
                return QVariant();
            }
            QMetaSequence::Iterable seqIter = seq.view<QMetaSequence::Iterable>();
            const QMetaSequence metaSequence = seqIter.metaContainer();
            if (!metaSequence.canAddValue()) {
                qWarning() << "Unsupported container" << qsq_->typeName.constData()
                           << "(Unable to add values)";
                return QVariant();
            }
            QByteArray valueTypeName;
            quint32 count;
            in >> valueTypeName;
            in >> count;
            QMetaType valueType = QMetaType::fromName(valueTypeName.constData());
            QVariant tmp{valueType, nullptr};
            for (quint32 i = 0; i < count; i++) {
                if (!valueType.load(in, tmp.data())) {
                    if (metaSequence.canRemoveValue() || i == 0) {
                        for (quint32 ii = 0; ii < i; ii++)
                            metaSequence.removeValue(seqIter.mutableIterable());
                        qWarning("QSQ_: unable to load type '%s', returning an empty list.", valueTypeName.constData());
                    } else {
                        qWarning("QSQ_: unable to load type '%s', returning a partial list.", valueTypeName.constData());
                    }
                    break;
                }

                QtPrivate::QVariantTypeCoercer coercer;
                metaSequence.addValue(
                        seqIter.mutableIterable(),
                        coercer.coerce(tmp, metaSequence.valueMetaType()));
            }
            value = seq;
#ifdef QTRO_VERBOSE_PROTOCOL
            qDebug() << "Decoding QSQ_ to sequential container" << containerType.name()
                     << valueTypeName;
#endif
        } else {
            QtROSequentialContainer container{};
            in >> container;
            container.m_typeName = qsq_->typeName;
            value = QVariant(QMetaType::fromType<QtROSequentialContainer>(), &container);
#ifdef QTRO_VERBOSE_PROTOCOL
            qDebug() << "Decoding QSQ_ to QtROSequentialContainer of"
                     << container.m_valueTypeName;
#endif
        }
    } else if (value.metaType() == QMetaType::fromType<QRemoteObjectPackets::QAS_>()) {
        const auto *qas_ = static_cast<const QRemoteObjectPackets::QAS_ *>(value.constData());
        QDataStream in(qas_->values);
        auto containerType = QMetaType::fromName(qas_->typeName.constData());
        bool isRegistered = containerType.isRegistered();
        if (isRegistered) {
            QVariant map{containerType, nullptr};
            if (!map.canView<QMetaAssociation::Iterable>()) {
                qWarning() << "Unsupported container" << qas_->typeName.constData()
                           << "(not viewable)";
                return QVariant();
            }
            QMetaAssociation::Iterable mapIter = map.view<QMetaAssociation::Iterable>();
            if (!mapIter.metaContainer().canSetMappedAtKey()) {
                qWarning() << "Unsupported container" << qas_->typeName.constData()
                           << "(Unable to insert values)";
                return QVariant();
            }
            QByteArray keyTypeName, valueTypeName;
            quint32 count;
            in >> keyTypeName;
            QMetaType keyType = QMetaType::fromName(keyTypeName.constData());
            if (!keyType.isValid()) {
                // This happens for class enums, where the passed keyType is <ClassName>::<enum>
                // For a compiled replica, the keyType is <ClassName>Replica::<enum>
                // Since the full typename is registered, we can pull the keyType from there
                keyType = mapIter.metaContainer().keyMetaType();
            }
            QMetaType transferType = keyType;
            if (keyType.flags().testFlag(QMetaType::IsEnumeration))
                transferType = transferTypeForEnum(keyType);
            QVariant key{transferType, nullptr};
            in >> valueTypeName;
            QMetaType valueType = QMetaType::fromName(valueTypeName.constData());
            QVariant val{valueType, nullptr};
            in >> count;
            for (quint32 i = 0; i < count; i++) {
                if (!transferType.load(in, key.data())) {
                    map = QVariant{containerType, nullptr};
                    qWarning("QAS_: unable to load key of type '%s', returning an empty map.",
                             keyTypeName.constData());
                    break;
                }
                if (!valueType.load(in, val.data())) {
                    map = QVariant{containerType, nullptr};
                    qWarning("QAS_: unable to load value of type '%s', returning an empty map.",
                             valueTypeName.constData());
                    break;
                }
                if (transferType != keyType) {
                    QVariant enumKey(key);
                    enumKey.convert(keyType);
                    mapIter.setValue(enumKey, val);
                } else {
                    mapIter.setValue(key, val);
                }
            }
            value = map;
#ifdef QTRO_VERBOSE_PROTOCOL
            qDebug() << "Decoding QAS_ to associative container" << containerType.name()
                     << valueTypeName << keyTypeName << count << mapIter.size() << map;
#endif
        } else {
            QtROAssociativeContainer container{};
            in >> container;
            container.m_typeName = qas_->typeName;
            value = QVariant(QMetaType::fromType<QtROAssociativeContainer>(), &container);
#ifdef QTRO_VERBOSE_PROTOCOL
            qDebug() << "Decoding QAS_ to QtROAssociativeContainer of"
                     << container.m_valueTypeName;
#endif
        }
    }
    return std::move(value);
}

void QDataStreamCodec::serializeProperty(const QRemoteObjectSourceBase *source, int internalIndex)
{
    serializeProperty(m_packet, source, internalIndex);
}

void QDataStreamCodec::serializeProperty(QDataStream &ds, const QRemoteObjectSourceBase *source, int internalIndex)
{
    const int propertyIndex = source->m_api->sourcePropertyIndex(internalIndex);
    Q_ASSERT (propertyIndex >= 0);
    const auto target = source->m_api->isAdapterProperty(internalIndex) ? source->m_adapter : source->m_object;
    const auto property = target->metaObject()->property(propertyIndex);
    const QVariant value = property.read(target);
    if (property.metaType().flags().testFlag(QMetaType::PointerToQObject)) {
        auto const childSource = source->m_children.value(internalIndex);
        auto valueAsPointerToQObject = qvariant_cast<QObject *>(value);
        if (childSource->m_object != valueAsPointerToQObject)
            childSource->resetObject(valueAsPointerToQObject);
        QRO_ qro(childSource);
        if (source->d->isDynamic && qro.type == ObjectType::CLASS && childSource->m_object && !source->d->sentTypes.contains(qro.typeName)) {
            QDataStream classDef(&qro.classDefinition, QIODevice::WriteOnly);
            serializeDefinition(classDef, childSource);
            source->d->sentTypes.insert(qro.typeName);
        }
        ds << QVariant::fromValue<QRO_>(qro);
        if (qro.isNull)
            return;
        const int propertyCount = childSource->m_api->propertyCount();
        // Put the properties in a buffer, the receiver may not know how to
        // interpret the types until it registers new ones.
        QDataStream params(&qro.parameters, QIODevice::WriteOnly);
        params << propertyCount;
        for (int internalIndex = 0; internalIndex < propertyCount; ++internalIndex)
            serializeProperty(params, childSource, internalIndex);
        ds << qro.parameters;
        return;
    }
    if (source->d->isDynamic && property.userType() == QMetaType::QVariant
        && value.metaType().flags().testFlag(QMetaType::IsGadget)) {
        const auto typeName = QString::fromLatin1(value.metaType().name());
        if (!source->d->sentTypes.contains(typeName)) {
            QRO_ qro(value);
            ds << QVariant::fromValue<QRO_>(qro);
            ds << qro.parameters;
            source->d->sentTypes.insert(typeName);
            return;
        }
    }
    ds << encodeVariant(value);
}

void QDataStreamCodec::serializeHandshakePacket()
{
    m_packet.setId(Handshake);
    m_packet << QString(protocolVersion);
    m_packet.finishPacket();
}

void QDataStreamCodec::serializeInitPacket(const QRemoteObjectRootSource *source)
{
    m_packet.setId(InitPacket);
    m_packet << source->name();
    serializeProperties(source);
    m_packet.finishPacket();
}

void QDataStreamCodec::serializeProperties(const QRemoteObjectSourceBase *source)
{
    const SourceApiMap *api = source->m_api;

    //Now copy the property data
    const int numProperties = api->propertyCount();
    m_packet << quint32(numProperties);  //Number of properties

    for (int internalIndex = 0; internalIndex < numProperties; ++internalIndex)
        serializeProperty(source, internalIndex);
}

bool deserializeQVariantList(QDataStream &s, QVariantList &l)
{
    // note: optimized version of: QDataStream operator>>(QDataStream& s, QList<T>& l)
    quint32 c;
    s >> c;

    const qsizetype count = static_cast<qsizetype>(c);
    const qsizetype listSize = l.size();
    if (listSize < count)
        l.reserve(count);
    else if (listSize > count)
        l.resize(count);

    for (int i = 0; i < l.size(); ++i)
    {
        if (s.atEnd())
            return false;
        s >> l[i];
    }
    for (auto i = l.size(); i < count; ++i)
    {
        if (s.atEnd())
            return false;
        s >> l.emplace_back();
    }
    return true;
}

void QDataStreamCodec::deserializeInitPacket(QDataStream &in, QVariantList &values)
{
    const bool success = deserializeQVariantList(in, values);
    Q_ASSERT(success);
    Q_UNUSED(success)
}

void QDataStreamCodec::serializeInitDynamicPacket(const QRemoteObjectRootSource *source)
{
    m_packet.setId(InitDynamicPacket);
    m_packet << source->name();
    serializeDefinition(m_packet, source);
    serializeProperties(source);
    m_packet.finishPacket();
}

static ObjectType getObjectType(const QString &typeName)
{
    if (typeName == QLatin1String("QAbstractItemModelAdapter"))
        return ObjectType::MODEL;
    auto tid = QMetaType::fromName(typeName.toUtf8()).id();
    if (tid == QMetaType::UnknownType)
        return ObjectType::CLASS;
    QMetaType type(tid);
    auto mo = type.metaObject();
    if (mo && mo->inherits(&QAbstractItemModel::staticMetaObject))
        return ObjectType::MODEL;
    return ObjectType::CLASS;
}

static QByteArrayView resolveEnumName(QMetaType t, bool &isFlag)
{
    // Takes types like `MyPOD::Position` or `QFlags<MyPOD::Position>` and returns 'Position`
    QByteArrayView enumName(t.name());
    isFlag = enumName.startsWith("QFlags<");
    auto lastColon = enumName.lastIndexOf(':');
    if (lastColon >= 0)
        enumName = QByteArrayView(t.name() + lastColon + 1);
    if (isFlag)
        enumName.chop(1);
    return enumName;
}

static QMetaEnum metaEnumFromType(QMetaType t)
{
    if (t.flags().testFlag(QMetaType::IsEnumeration)) {
        if (const QMetaObject *m = t.metaObject()) {
            bool isFlag;
            auto enumName = resolveEnumName(t, isFlag);
            if (isFlag) {
                for (int i = m->enumeratorOffset(); i < m->enumeratorCount(); i++) {
                    auto testType = m->enumerator(i);
                    if (testType.isFlag() &&
                        enumName.compare(QByteArrayView(testType.enumName())) == 0)
                        return testType;
                }
            }
            return m->enumerator(m->indexOfEnumerator(enumName.data()));
        }
    }
    return QMetaEnum();
}

static bool checkEnum(QMetaType metaType, QSet<QMetaEnum> &enums)
{
    if (metaType.flags().testFlag(QMetaType::IsEnumeration)) {
        QMetaEnum meta = metaEnumFromType(metaType);
        enums.insert(meta);
        return true;
    }
    return false;
}

static void recurseMetaobject(const QMetaObject *mo, QSet<const QMetaObject *> &gadgets, QSet<QMetaEnum> &enums)
{
    if (!mo || gadgets.contains(mo))
        return;
    gadgets.insert(mo);
    const int numProperties = mo->propertyCount();
    for (int i = 0; i < numProperties; ++i) {
        const auto property = mo->property(i);
        if (checkEnum(property.metaType(), enums))
            continue;
        if (property.metaType().flags().testFlag(QMetaType::IsGadget))
            recurseMetaobject(property.metaType().metaObject(), gadgets, enums);
    }
}

// A Source may only use a subset of the metaobjects properties/signals/slots, so we only search
// the ones in the API.  For nested pointer types, we will have another api to limit the search.
// For nested PODs/enums, we search the entire qobject (using the recurseMetaobject call()).
void recurseForGadgets(QSet<const QMetaObject *> &gadgets, QSet<QMetaEnum> &enums, const QRemoteObjectSourceBase *source)
{
    const SourceApiMap *api = source->m_api;

    const int numSignals = api->signalCount();
    const int numMethods = api->methodCount();
    const int numProperties = api->propertyCount();

    for (int si = 0; si < numSignals; ++si) {
        const int params = api->signalParameterCount(si);
        for (int pi = 0; pi < params; ++pi) {
            const int type = api->signalParameterType(si, pi);
            const auto metaType = QMetaType(type);
            if (checkEnum(metaType, enums))
                continue;
            if (!metaType.flags().testFlag(QMetaType::IsGadget))
                continue;
            const auto mo = metaType.metaObject();
            if (source->d->sentTypes.contains(QLatin1String(mo->className())))
                continue;
            recurseMetaobject(mo, gadgets, enums);
            source->d->sentTypes.insert(QLatin1String(mo->className()));
        }
    }

    for (int mi = 0; mi < numMethods; ++mi) {
        const int params = api->methodParameterCount(mi);
        for (int pi = 0; pi < params; ++pi) {
            const int type = api->methodParameterType(mi, pi);
            const auto metaType = QMetaType(type);
            if (checkEnum(metaType, enums))
                continue;
            if (!metaType.flags().testFlag(QMetaType::IsGadget))
                continue;
            const auto mo = metaType.metaObject();
            if (source->d->sentTypes.contains(QLatin1String(mo->className())))
                continue;
            recurseMetaobject(mo, gadgets, enums);
            source->d->sentTypes.insert(QLatin1String(mo->className()));
        }
    }
    for (int pi = 0; pi < numProperties; ++pi) {
        const int index = api->sourcePropertyIndex(pi);
        Q_ASSERT(index >= 0);
        const auto target = api->isAdapterProperty(pi) ? source->m_adapter : source->m_object;
        const auto metaProperty = target->metaObject()->property(index);
        const auto metaType = metaProperty.metaType();
        if (checkEnum(metaType, enums))
            continue;
        if (metaType.flags().testFlag(QMetaType::PointerToQObject)) {
            auto const objectType = getObjectType(QString::fromLatin1(metaProperty.typeName()));
            if (objectType == ObjectType::CLASS) {
                auto const childSource = source->m_children.value(pi);
                if (childSource->m_object)
                    recurseForGadgets(gadgets, enums, childSource);
            }
        }
        if (!metaType.flags().testFlag(QMetaType::IsGadget))
            continue;
        const auto mo = metaType.metaObject();
        if (source->d->sentTypes.contains(QLatin1String(mo->className())))
            continue;
        recurseMetaobject(mo, gadgets, enums);
        source->d->sentTypes.insert(QLatin1String(mo->className()));
    }
}

static bool checkForEnumsInSource(const QMetaObject *meta, const QRemoteObjectSourceBase *source)
{
    if (source->m_object->inherits(meta->className()))
        return true;
    for (const auto &child : source->m_children) {
        if (child->m_object && checkForEnumsInSource(meta, child))
            return true;
    }
    return false;
}

static void serializeEnum(QDataStream &ds, const QMetaEnum &enumerator)
{
    ds << QByteArray::fromRawData(enumerator.name(), qsizetype(qstrlen(enumerator.name())));
    ds << enumerator.isFlag();
    ds << enumerator.isScoped();
    const auto typeName = QByteArray(enumerator.scope()).append("::").append(enumerator.name());
    const quint32 size = quint32(QMetaType::fromName(typeName.constData()).sizeOf());
    ds << size;
#ifdef QTRO_VERBOSE_PROTOCOL
    qDebug("  Enum (name = %s, size = %d, isFlag = %s, isScoped = %s):", enumerator.name(), size, enumerator.isFlag() ? "true" : "false", enumerator.isScoped() ? "true" : "false");
#endif
    const int keyCount = enumerator.keyCount();
    ds << keyCount;
    for (int k = 0; k < keyCount; ++k) {
        ds << QByteArray::fromRawData(enumerator.key(k), qsizetype(qstrlen(enumerator.key(k))));
        ds << enumerator.value(k);
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug("    Key %d (name = %s, value = %d):", k, enumerator.key(k), enumerator.value(k));
#endif
    }
}

static void serializeGadgets(QDataStream &ds, const QSet<const QMetaObject *> &gadgets, const QSet<QMetaEnum> &enums, const QRemoteObjectSourceBase *source=nullptr)
{
    // Determine how to handle the enums found
    QSet<QMetaEnum> qtEnums;
    QSet<const QMetaObject *> dynamicEnumMetaObjects;
    for (const auto &metaEnum : enums) {
        auto const metaObject = metaEnum.enclosingMetaObject();
        if (gadgets.contains(metaObject)) // Part of a gadget will we serialize
            continue;
        // This checks if the enum is defined in our object heirarchy, in which case it will
        // already have been serialized.
        if (source && checkForEnumsInSource(metaObject, source->d->root))
            continue;
        // qtEnums are enumerations already known by Qt, so we only need register them.
        // We don't need to send all of the key/value data.
        if (metaObject == &Qt::staticMetaObject) // Are the other Qt metaclasses for enums?
            qtEnums.insert(metaEnum);
        else
            dynamicEnumMetaObjects.insert(metaEnum.enclosingMetaObject());
    }
    ds << quint32(qtEnums.size());
    for (const auto &metaEnum : qtEnums) {
        QByteArray enumName(metaEnum.scope());
        enumName.append("::", 2).append(metaEnum.name());
        ds << enumName;
    }
    const auto allMetaObjects = gadgets + dynamicEnumMetaObjects;
    ds << quint32(allMetaObjects.size());
#ifdef QTRO_VERBOSE_PROTOCOL
    qDebug() << "  Found" << gadgets.size() << "gadget/pod and" << (allMetaObjects.size() - gadgets.size()) << "enum types";
    int i = 0;
#endif
    // There isn't an easy way to update a metaobject incrementally, so we
    // send all of the metaobject's enums, but no properties, when an external
    // enum is requested.
    for (auto const meta : allMetaObjects) {
        ds << QByteArray::fromRawData(meta->className(), qsizetype(qstrlen(meta->className())));
        int propertyCount = gadgets.contains(meta) ? meta->propertyCount() : 0;
        ds << quint32(propertyCount);
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug("  Gadget %d (name = %s, # properties = %d, # enums = %d):", i++, meta->className(), propertyCount, meta->enumeratorCount() - meta->enumeratorOffset());
#endif
        for (int j = 0; j < propertyCount; j++) {
            auto prop = meta->property(j);
#ifdef QTRO_VERBOSE_PROTOCOL
            qDebug("    Data member %d (name = %s, type = %s):", j, prop.name(), prop.typeName());
#endif
            ds << QByteArray::fromRawData(prop.name(), qsizetype(qstrlen(prop.name())));
            ds << QByteArray::fromRawData(prop.typeName(), qsizetype(qstrlen(prop.typeName())));
        }
        int enumCount = meta->enumeratorCount() - meta->enumeratorOffset();
        ds << quint32(enumCount);
        for (int j = meta->enumeratorOffset(); j < meta->enumeratorCount(); j++) {
            auto const enumMeta = meta->enumerator(j);
            serializeEnum(ds, enumMeta);
        }
    }
}

void QDataStreamCodec::serializeDefinition(QDataStream &ds, const QRemoteObjectSourceBase *source)
{
    const SourceApiMap *api = source->m_api;
    const QByteArray desiredClassName(api->typeName().toLatin1());
    const QByteArray originalClassName = api->className();
    // The dynamic class will be called typeName on the receiving side of this definition
    // However, there are types like enums that have the QObject's class name.  Replace()
    // will convert a parameter such as "ParentClassSource::MyEnum" to "ParentClass::MyEnum"
    // so the type can be properly resolved and registered.
    auto replace = [&originalClassName, &desiredClassName](QByteArray &name) {
        name.replace(originalClassName, desiredClassName);
    };

    ds << source->m_api->typeName();
#ifdef QTRO_VERBOSE_PROTOCOL
    qDebug() << "Serializing definition for" << source->m_api->typeName();
#endif

    //Now copy the property data
    const int numEnums = api->enumCount();
    const auto metaObject = source->m_object->metaObject();
    ds << quint32(numEnums);  //Number of Enums
#ifdef QTRO_VERBOSE_PROTOCOL
    qDebug() << "  Found" << numEnums << "enumeration types";
#endif
    for (int i = 0; i < numEnums; ++i) {
        auto enumerator = metaObject->enumerator(api->sourceEnumIndex(i));
        Q_ASSERT(enumerator.isValid());
        serializeEnum(ds, enumerator);
    }

    QSet<const QMetaObject *> gadgets;
    QSet<QMetaEnum> enums;
    recurseForGadgets(gadgets, enums, source);
    serializeGadgets(ds, gadgets, enums, source);

    const int numSignals = api->signalCount();
    ds << quint32(numSignals);  //Number of signals
    for (int i = 0; i < numSignals; ++i) {
        const int index = api->sourceSignalIndex(i);
        Q_ASSERT(index >= 0);
        auto signature = api->signalSignature(i);
        replace(signature);
        const int count = api->signalParameterCount(i);
        for (int pi = 0; pi < count; ++pi) {
            const auto metaType = QMetaType(api->signalParameterType(i, pi));
            if (isSequentialGadgetType(metaType))
                signature.replace(metaType.name(), "QtROSequentialContainer");
            else if (isAssociativeGadgetType(metaType))
                signature.replace(metaType.name(), "QtROAssociativeContainer");
        }
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug() << "  Signal" << i << "(signature =" << signature << "parameter names =" << api->signalParameterNames(i) << ")";
#endif
        ds << signature;
        ds << api->signalParameterNames(i);
    }

    const int numMethods = api->methodCount();
    ds << quint32(numMethods);  //Number of methods
    for (int i = 0; i < numMethods; ++i) {
        const int index = api->sourceMethodIndex(i);
        Q_ASSERT(index >= 0);
        auto signature = api->methodSignature(i);
        replace(signature);
        const int count = api->methodParameterCount(i);
        for (int pi = 0; pi < count; ++pi) {
            const auto metaType = QMetaType(api->methodParameterType(i, pi));
            if (isSequentialGadgetType(metaType))
                signature.replace(metaType.name(), "QtROSequentialContainer");
            else if (isAssociativeGadgetType(metaType))
                signature.replace(metaType.name(), "QtROAssociativeContainer");
        }
        auto typeName = api->typeName(i);
        replace(typeName);
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug() << "  Slot" << i << "(signature =" << signature << "parameter names =" << api->methodParameterNames(i) << "return type =" << typeName << ")";
#endif
        ds << signature;
        ds << typeName;
        ds << api->methodParameterNames(i);
    }

    const int numProperties = api->propertyCount();
    ds << quint32(numProperties);  //Number of properties
    for (int i = 0; i < numProperties; ++i) {
        const int index = api->sourcePropertyIndex(i);
        Q_ASSERT(index >= 0);

        const auto target = api->isAdapterProperty(i) ? source->m_adapter : source->m_object;
        const auto metaProperty = target->metaObject()->property(index);
        ds << metaProperty.name();
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug() << "  Property" << i << "name =" << metaProperty.name();
#endif
        if (metaProperty.metaType().flags().testFlag(QMetaType::PointerToQObject)) {
            auto objectType = getObjectType(QLatin1String(metaProperty.typeName()));
            ds << (objectType == ObjectType::CLASS ? "QObject*" : "QAbstractItemModel*");
#ifdef QTRO_VERBOSE_PROTOCOL
            qDebug() << "    Type:" << (objectType == ObjectType::CLASS ? "QObject*" : "QAbstractItemModel*");
#endif
        } else {
            if (isSequentialGadgetType(metaProperty.metaType())) {
                ds << "QtROSequentialContainer";
#ifdef QTRO_VERBOSE_PROTOCOL
                qDebug() << "    Type:" << "QtROSequentialContainer";
#endif
            } else if (isAssociativeGadgetType(metaProperty.metaType())) {
                ds << "QtROAssociativeContainer";
#ifdef QTRO_VERBOSE_PROTOCOL
                qDebug() << "    Type:" << "QtROAssociativeContainer";
#endif
            } else {
                ds << metaProperty.typeName();
#ifdef QTRO_VERBOSE_PROTOCOL
                qDebug() << "    Type:" << metaProperty.typeName();
#endif
            }
        }
        if (metaProperty.notifySignalIndex() == -1) {
            ds << QByteArray();
#ifdef QTRO_VERBOSE_PROTOCOL
            qDebug() << "    Notification signal: None";
#endif
        } else {
            auto signature = metaProperty.notifySignal().methodSignature();
            replace(signature);
            ds << signature;
#ifdef QTRO_VERBOSE_PROTOCOL
            qDebug() << "    Notification signal:" << signature;
#endif
        }
    }
}

void QDataStreamCodec::serializeAddObjectPacket(const QString &name, bool isDynamic)
{
    m_packet.setId(AddObject);
    m_packet << name;
    m_packet << isDynamic;
    m_packet.finishPacket();
}

void QDataStreamCodec::deserializeAddObjectPacket(QDataStream &ds, bool &isDynamic)
{
    ds >> isDynamic;
}

void QDataStreamCodec::serializeRemoveObjectPacket(const QString &name)
{
    m_packet.setId(RemoveObject);
    m_packet << name;
    m_packet.finishPacket();
}
//There is no deserializeRemoveObjectPacket - no parameters other than id and name

void QDataStreamCodec::serializeInvokePacket(const QString &name, int call, int index, const QVariantList &args, int serialId, int propertyIndex)
{
    m_packet.setId(InvokePacket);
    m_packet << name;
    m_packet << call;
    m_packet << index;

    m_packet << quint32(args.size());
    for (const auto &arg : args)
        m_packet << encodeVariant(arg);

    m_packet << serialId;
    m_packet << propertyIndex;
    m_packet.finishPacket();
}

void QDataStreamCodec::deserializeInvokePacket(QDataStream& in, int &call, int &index, QVariantList &args, int &serialId, int &propertyIndex)
{
    in >> call;
    in >> index;
    const bool success = deserializeQVariantList(in, args);
    Q_ASSERT(success);
    Q_UNUSED(success)
    in >> serialId;
    in >> propertyIndex;
}

void QDataStreamCodec::serializeInvokeReplyPacket(const QString &name, int ackedSerialId, const QVariant &value)
{
    m_packet.setId(InvokeReplyPacket);
    m_packet << name;
    m_packet << ackedSerialId;
    m_packet << value;
    m_packet.finishPacket();
}

void QDataStreamCodec::deserializeInvokeReplyPacket(QDataStream& in, int &ackedSerialId, QVariant &value){
    in >> ackedSerialId;
    in >> value;
}

void QDataStreamCodec::serializePropertyChangePacket(QRemoteObjectSourceBase *source, int signalIndex)
{
    int internalIndex = source->m_api->propertyRawIndexFromSignal(signalIndex);
    m_packet.setId(PropertyChangePacket);
    m_packet << source->name();
    m_packet << internalIndex;
    serializeProperty(source, internalIndex);
    m_packet.finishPacket();
}

void QDataStreamCodec::deserializePropertyChangePacket(QDataStream& in, int &index, QVariant &value)
{
    in >> index;
    in >> value;
}

void QDataStreamCodec::serializeObjectListPacket(const ObjectInfoList &objects)
{
    m_packet.setId(ObjectList);
    m_packet << objects;
    m_packet.finishPacket();
}

void QDataStreamCodec::deserializeObjectListPacket(QDataStream &in, ObjectInfoList &objects)
{
    in >> objects;
}

void QDataStreamCodec::serializePingPacket(const QString &name)
{
    m_packet.setId(Ping);
    m_packet << name;
    m_packet.finishPacket();
}

void QDataStreamCodec::serializePongPacket(const QString &name)
{
    m_packet.setId(Pong);
    m_packet << name;
    m_packet.finishPacket();
}

QRO_::QRO_(QRemoteObjectSourceBase *source)
    : name(source->name())
    , typeName(source->m_api->typeName())
    , type(source->m_adapter ? ObjectType::MODEL : getObjectType(typeName))
    , isNull(source->m_object == nullptr)
    , classDefinition()
    , parameters()
{}

QRO_::QRO_(const QVariant &value)
    : type(ObjectType::GADGET)
    , isNull(false)
{
    const auto metaType = value.metaType();
    auto meta = metaType.metaObject();
    QDataStream out(&classDefinition, QIODevice::WriteOnly);
    const int numProperties = meta->propertyCount();
    const auto name = metaType.name();
    const auto typeName = QByteArray::fromRawData(name, qsizetype(qstrlen(name)));
    out << quint32(0) << quint32(1);
    out << typeName;
    out << numProperties;
#ifdef QTRO_VERBOSE_PROTOCOL
    qDebug("Serializing POD definition to QRO_ (name = %s)", typeName.constData());
#endif
    for (int i = 0; i < numProperties; ++i) {
        const auto property = meta->property(i);
#ifdef QTRO_VERBOSE_PROTOCOL
        qDebug("  Data member %d (name = %s, type = %s):", i, property.name(), property.typeName());
#endif
        out << QByteArray::fromRawData(property.name(), qsizetype(qstrlen(property.name())));
        out << QByteArray::fromRawData(property.typeName(), qsizetype(qstrlen(property.typeName())));
    }
    int enumCount = meta->enumeratorCount() - meta->enumeratorOffset();
    out << quint32(enumCount);
    for (int j = meta->enumeratorOffset(); j < meta->enumeratorCount(); j++) {
        auto const enumMeta = meta->enumerator(j);
        serializeEnum(out, enumMeta);
    }
    QDataStream ds(&parameters, QIODevice::WriteOnly);
    ds << value;
#ifdef QTRO_VERBOSE_PROTOCOL
    qDebug() << "  Value:" << value;
#endif
}

QDataStream &operator<<(QDataStream &stream, const QRO_ &info)
{
    stream << info.name << info.typeName << quint8(info.type) << info.classDefinition << info.isNull;
    qCDebug(QT_REMOTEOBJECT) << "Serializing " << info;
    // info.parameters will be filled in by serializeProperty
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QRO_ &info)
{
    quint8 tmpType;
    stream >> info.name >> info.typeName >> tmpType >> info.classDefinition >> info.isNull;
    info.type = static_cast<ObjectType>(tmpType);
    qCDebug(QT_REMOTEOBJECT) << "Deserializing " << info;
    if (!info.isNull)
        stream >> info.parameters;
    return stream;
}

QSQ_::QSQ_(const QVariant &variant)
{
    QMetaSequence::Iterable sequence;
    QMetaType valueType;
    if (variant.metaType() == QMetaType::fromType<QtROSequentialContainer>()) {
        auto container = static_cast<const QtROSequentialContainer *>(variant.constData());
        typeName = container->m_typeName;
        valueType = container->m_valueType;
        valueTypeName = container->m_valueTypeName;
        sequence = QMetaSequence::Iterable(reinterpret_cast<const QVariantList *>(variant.constData()));
    } else {
        sequence = variant.value<QMetaSequence::Iterable>();
        typeName = QByteArray(variant.metaType().name());
        valueType = sequence.metaContainer().valueMetaType();
        valueTypeName = QByteArray(valueType.name());
    }
#ifdef QTRO_VERBOSE_PROTOCOL
    qDebug("Serializing POD sequence to QSQ_ (type = %s, valueType = %s) with size = %lld",
           typeName.constData(), valueTypeName.constData(), sequence.size());
#endif
    QDataStream ds(&values, QIODevice::WriteOnly);
    ds << valueTypeName;
    auto pos = ds.device()->pos();
    ds << quint32(sequence.size());
    for (const auto &v : sequence) {
        if (!valueType.save(ds, v.data())) {
            ds.device()->seek(pos);
            ds.resetStatus();
            ds << quint32(0);
            values.resize(ds.device()->pos());
            qWarning("QSQ_: unable to save type '%s', sending empty list.", valueType.name());
            break;
        }
    }
}

QDataStream &operator<<(QDataStream &stream, const QSQ_ &sequence)
{
    stream << sequence.typeName << sequence.valueTypeName << sequence.values;
    qCDebug(QT_REMOTEOBJECT) << "Serializing " << sequence;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QSQ_ &sequence)
{
    stream >> sequence.typeName >> sequence.valueTypeName >> sequence.values;
    qCDebug(QT_REMOTEOBJECT) << "Deserializing " << sequence;
    return stream;
}

QAS_::QAS_(const QVariant &variant)
{
    QMetaAssociation::Iterable map;
    QMetaType keyType, transferType, valueType;
    const QtROAssociativeContainer *container = nullptr;
    if (variant.metaType() == QMetaType::fromType<QtROAssociativeContainer>()) {
        container = static_cast<const QtROAssociativeContainer *>(variant.constData());
        typeName = container->m_typeName;
        keyType = container->m_keyType;
        keyTypeName = container->m_keyTypeName;
        valueType = container->m_valueType;
        valueTypeName = container->m_valueTypeName;
        map = QMetaAssociation::Iterable(reinterpret_cast<const QVariantMap *>(variant.constData()));
    } else {
        map = variant.value<QMetaAssociation::Iterable>();
        typeName = QByteArray(variant.metaType().name());
        keyType = map.metaContainer().keyMetaType();
        keyTypeName = QByteArray(keyType.name());
        valueType = map.metaContainer().mappedMetaType();
        valueTypeName = QByteArray(valueType.name());
    }
    // Special handling for enums...
    transferType = keyType;
    if (keyType.flags().testFlag(QMetaType::IsEnumeration)) {
        transferType = transferTypeForEnum(keyType);
        auto meta = keyType.metaObject();
        // If we are the source, make sure the typeName is converted for any downstream replicas
        // the `meta` variable will be non-null when the enum is a class enum
        if (meta && !container) {
            const int ind = meta->indexOfClassInfo(QCLASSINFO_REMOTEOBJECT_TYPE);
            if (ind >= 0) {
                bool isFlag = keyTypeName.startsWith("QFlags<");
                if (isFlag || keyTypeName.startsWith(meta->className())) {
#ifdef QTRO_VERBOSE_PROTOCOL
                    QByteArray orig(keyTypeName);
#endif
                    if (isFlag) {
                        // Q_DECLARE_FLAGS(Flags, Enum) -> `typedef QFlags<Enum> Flags;`
                        // We know we have an enum for `Flags` because we sent the enums
                        // from the source, so we just need to look up the alias.
                        keyTypeName = keyTypeName.mid(7);
                        keyTypeName.chop(1); // Remove trailing '>'
                        int index = keyTypeName.lastIndexOf(':');
                        for (int i = meta->enumeratorOffset(); i < meta->enumeratorCount(); i++) {
                            auto en = meta->enumerator(i);
                            auto name = keyTypeName.data() + index + 1;
                            if (en.isFlag() && qstrcmp(en.enumName(), name) == 0)
                                keyTypeName.replace(index + 1, qstrlen(en.enumName()), en.name());
                        }
                    }
                    keyTypeName.replace(meta->className(), meta->classInfo(ind).value());
                    QByteArray repName(meta->classInfo(ind).value());
                    repName.append("Replica");
                    typeName.replace(meta->className(), repName);
#ifdef QTRO_VERBOSE_PROTOCOL
                    qDebug() << "Converted map key typename from" << orig << "to" << keyTypeName;
#endif
                }
            }
        }
    }

#ifdef QTRO_VERBOSE_PROTOCOL
    qDebug("Serializing POD map to QAS_ (type = %s, keyType = %s, valueType = %s), size = %lld",
           typeName.constData(), keyTypeName.constData(), valueTypeName.constData(),
           map.size());
#endif
    QDataStream ds(&values, QIODevice::WriteOnly);
    ds << keyTypeName;
    ds << valueTypeName;
    auto pos = ds.device()->pos();
    ds << quint32(map.size());
    QMetaAssociation::Iterable::const_iterator iter = map.begin();
    for (int i = 0; i < map.size(); i++) {
        QVariant key(container ? container->m_keys.at(i) : iter.key());
        if (transferType != keyType)
            key.convert(transferType);
        if (!transferType.save(ds, key.data())) {
            ds.device()->seek(pos);
            ds.resetStatus();
            ds << quint32(0);
            values.resize(ds.device()->pos());
            qWarning("QAS_: unable to save key '%s', sending empty map.", keyType.name());
            break;
        }
        if (!valueType.save(ds, iter.value().data())) {
            ds.device()->seek(pos);
            ds.resetStatus();
            ds << quint32(0);
            values.resize(ds.device()->pos());
            qWarning("QAS_: unable to save value '%s', sending empty map.", valueType.name());
            break;
        }
        iter++;
    }
}

QDataStream &operator<<(QDataStream &stream, const QAS_ &map)
{
    stream << map.typeName << map.keyTypeName << map.valueTypeName << map.values;
    qCDebug(QT_REMOTEOBJECT) << "Serializing " << map;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QAS_ &map)
{
    stream >> map.typeName >> map.keyTypeName >> map.valueTypeName >> map.values;
    qCDebug(QT_REMOTEOBJECT) << "Deserializing " << map;
    return stream;
}

DataStreamPacket::DataStreamPacket(quint16 id)
    : QDataStream(&array, QIODevice::WriteOnly), baseAddress(0), size(0)
{
    this->setVersion(QtRemoteObjects::dataStreamVersion);
    this->setByteOrder(QDataStream::LittleEndian);
    *this << quint32(0);
    *this << id;
}

void CodecBase::send(const QSet<QtROIoDeviceBase *> &connections)
{
    const auto bytearray = getPayload();
    for (auto conn : connections)
        conn->write(bytearray);
    reset();
}

void CodecBase::send(const QList<QtROIoDeviceBase *> &connections)
{
    const auto bytearray = getPayload();
    for (auto conn : connections)
        conn->write(bytearray);
    reset();
}

void CodecBase::send(QtROIoDeviceBase *connection)
{
    const auto bytearray = getPayload();
    connection->write(bytearray);
    reset();
}

} // namespace QRemoteObjectPackets

QT_IMPL_METATYPE_EXTERN_TAGGED(QRemoteObjectPackets::QRO_, QRemoteObjectPackets__QRO_)

QT_END_NAMESPACE
