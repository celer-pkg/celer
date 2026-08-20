// Copyright (C) 2021 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#ifndef QTREMOTEOBJECTPACKET_P_H
#define QTREMOTEOBJECTPACKET_P_H

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

#include "qtremoteobjectglobal.h"
#include "qremoteobjectsource.h"
#include "qconnectionfactories.h"

#include <QtCore/qhash.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qmetaassociation.h>
#include <QtCore/qmetasequence.h>
#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qdatastream.h>
#include <QtCore/private/qglobal_p.h>

#include <cstdlib>

QT_BEGIN_NAMESPACE

class QMetaObjectBuilder;
class QRemoteObjectSourceBase;
class QRemoteObjectRootSource;

namespace QRemoteObjectPackets {

Q_NAMESPACE

class DataStreamPacket;

struct ObjectInfo
{
    QString name;
    QString typeName;
    QByteArray signature;
};

inline QDebug operator<<(QDebug dbg, const ObjectInfo &info)
{
    dbg.nospace() << "ObjectInfo(" << info.name << ", " << info.typeName << ", " << info.signature <<")";
    return dbg.space();
}

inline QDataStream& operator<<(QDataStream &stream, const ObjectInfo &info)
{
    return stream << info.name << info.typeName << info.signature;
}

inline QDataStream& operator>>(QDataStream &stream, ObjectInfo &info)
{
    return stream >> info.name >> info.typeName >> info.signature;
}

using ObjectInfoList = QList<ObjectInfo>;

enum class ObjectType : quint8 { CLASS, MODEL, GADGET };
Q_ENUM_NS(ObjectType)

// Use a short name, as QVariant::save writes the name every time a qvariant of
// this type is serialized
class QRO_
{
public:
    QRO_() : type(ObjectType::CLASS), isNull(true) {}
    explicit QRO_(QRemoteObjectSourceBase *source);
    explicit QRO_(const QVariant &value);
    QString name, typeName;
    ObjectType type;
    bool isNull;
    QByteArray classDefinition;
    QByteArray parameters;
};

inline QDebug operator<<(QDebug dbg, const QRO_ &info)
{
    dbg.nospace() << "QRO_(name: " << info.name << ", typeName: " << info.typeName
                  << ", type: " << info.type << ", valid: " << (info.isNull ? "true" : "false")
                  << ", parameters: {" << info.parameters << ")"
                  << (info.classDefinition.isEmpty() ? " no definitions)" : " with definitions)");
    return dbg.space();
}

QDataStream& operator<<(QDataStream &stream, const QRO_ &info);

QDataStream& operator>>(QDataStream &stream, QRO_ &info);

// Class for transmitting sequence data.  Needed because containers for custom
// types (and even primitive types in Qt5) are not registered with the metaObject
// system.  This wrapper allows us to create the desired container if it is
// registered, or a QtROSequentialContainer if it is not.  QtROSequentialContainer
// is derived from QVariantList, so it can be used from QML similar to the API
// type.
class QSQ_
{
public:
    QSQ_() {}
    explicit QSQ_(const QVariant &lst);
    QByteArray typeName, valueTypeName;
    QByteArray values;
};

inline QDebug operator<<(QDebug dbg, const QSQ_ &seq)
{
    dbg.nospace() << "QSQ_(typeName: " << seq.typeName << ", valueType: " << seq.valueTypeName
                  << ", values: {" << seq.values <<")";
    return dbg.space();
}

QDataStream& operator<<(QDataStream &stream, const QSQ_ &info);

QDataStream& operator>>(QDataStream &stream, QSQ_ &info);

// Class for transmitting associative containers.  Needed because containers for
// custom types (and even primitive types in Qt5) are not registered with the
// metaObject system.  This wrapper allows us to create the desired container if
// it is registered, or a QtROAssociativeContainer if it is not.
// QtROAssociativeContainer is derived from QVariantMap, so it can be used from
// QML similar to the API type.
class QAS_
{
public:
    QAS_() {}
    explicit QAS_(const QVariant &lst);
    QByteArray typeName, keyTypeName, valueTypeName;
    QByteArray values;
};

inline QDebug operator<<(QDebug dbg, const QAS_ &seq)
{
    dbg.nospace() << "QAS_(typeName: " << seq.typeName << ", keyType: " << seq.keyTypeName
                  << ", valueType: " << seq.valueTypeName << ", values: {" << seq.values <<")";
    return dbg.space();
}

QDataStream& operator<<(QDataStream &stream, const QAS_ &info);

QDataStream& operator>>(QDataStream &stream, QAS_ &info);

//Helper class for creating a QByteArray from a QRemoteObjectPacket
class DataStreamPacket : public QDataStream
{
public:
    DataStreamPacket(quint16 id = QtRemoteObjects::InvokePacket);

    void setId(quint16 id)
    {
        device()->seek(baseAddress);
        *this << quint32(0);
        *this << id;
    }

    void finishPacket()
    {
        size = device()->pos();
        device()->seek(baseAddress);
        *this << quint32(size - baseAddress - sizeof(quint32));
        baseAddress = size; // Allow appending until reset() is called
    }

    const QByteArray &payload()
    {
        array.resize(size);
        return array;
    }

    void reset()
    {
        baseAddress = 0;
        size = 0;
        array.clear();
    }

private:
    QByteArray array;
    int baseAddress;
    int size;

    Q_DISABLE_COPY(DataStreamPacket)
};

class CodecBase
{
public:
    CodecBase() = default;
    CodecBase(const CodecBase &) = default;
    CodecBase(CodecBase &&) = default;
    CodecBase &operator=(const CodecBase &) = default;
    CodecBase &operator=(CodecBase &&) = default;
    virtual ~CodecBase() = default;

    virtual void serializeObjectListPacket(const ObjectInfoList &) = 0;
    virtual void deserializeObjectListPacket(QDataStream &in, ObjectInfoList &) = 0;
    virtual void serializeInitPacket(const QRemoteObjectRootSource *) = 0;
    virtual void serializeInitDynamicPacket(const QRemoteObjectRootSource *) = 0;
    virtual void serializePropertyChangePacket(QRemoteObjectSourceBase *source,
                                               int signalIndex) = 0;
    virtual void deserializePropertyChangePacket(QDataStream &in, int &index, QVariant &value) = 0;
    virtual void serializeProperty(const QRemoteObjectSourceBase *source, int internalIndex) = 0;
    // Heartbeat packets
    virtual void serializePingPacket(const QString &name) = 0;
    virtual void serializePongPacket(const QString &name) = 0;
    virtual void serializeInvokePacket(const QString &name, int call, int index,
                                       const QVariantList &args, int serialId = -1,
                                       int propertyIndex = -1) = 0;
    virtual void deserializeInvokePacket(QDataStream &in, int &call, int &index, QVariantList &args,
                                         int &serialId, int &propertyIndex) = 0;
    virtual void serializeInvokeReplyPacket(const QString &name, int ackedSerialId,
                                            const QVariant &value) = 0;
    virtual void serializeHandshakePacket() = 0;
    virtual void serializeRemoveObjectPacket(const QString &name) = 0;
    //There is no deserializeRemoveObjectPacket - no parameters other than id and name
    virtual void serializeAddObjectPacket(const QString &name, bool isDynamic) = 0;
    virtual void deserializeAddObjectPacket(QDataStream &, bool &isDynamic) = 0;
    virtual void deserializeInitPacket(QDataStream &, QVariantList &) = 0;
    virtual void deserializeInvokeReplyPacket(QDataStream &in, int &ackedSerialId,
                                              QVariant &value) = 0;
    void send(const QSet<QtROIoDeviceBase *> &connections);
    void send(const QVector<QtROIoDeviceBase *> &connections);
    void send(QtROIoDeviceBase *connection);

protected:
    // A payload can consist of one or more packets
    virtual const QByteArray &getPayload() = 0;
    virtual void reset() {}
};

class QDataStreamCodec : public CodecBase
{
public:
    void serializeObjectListPacket(const ObjectInfoList &) override;
    void deserializeObjectListPacket(QDataStream &in, ObjectInfoList &) override;
    void serializeInitPacket(const QRemoteObjectRootSource *) override;
    void serializeInitDynamicPacket(const QRemoteObjectRootSource*) override;
    void serializePropertyChangePacket(QRemoteObjectSourceBase *source, int signalIndex) override;
    void deserializePropertyChangePacket(QDataStream &in, int &index, QVariant &value) override;
    void serializeProperty(const QRemoteObjectSourceBase *source, int internalIndex) override;
    void serializePingPacket(const QString &name) override;
    void serializePongPacket(const QString &name) override;
    void serializeInvokePacket(const QString &name, int call, int index, const QVariantList &args,
                               int serialId = -1, int propertyIndex = -1) override;
    void deserializeInvokePacket(QDataStream &in, int &call, int &index, QVariantList &args,
                                 int &serialId, int &propertyIndex) override;
    void serializeInvokeReplyPacket(const QString &name, int ackedSerialId,
                                    const QVariant &value) override;
    void serializeHandshakePacket() override;
    void serializeRemoveObjectPacket(const QString &name) override;
    void serializeAddObjectPacket(const QString &name, bool isDynamic) override;
    void deserializeAddObjectPacket(QDataStream &, bool &isDynamic) override;
    void deserializeInitPacket(QDataStream &, QVariantList &) override;
    void deserializeInvokeReplyPacket(QDataStream &in, int &ackedSerialId,
                                      QVariant &value) override;

protected:
    const QByteArray &getPayload() override {
        return m_packet.payload();
    }
    void reset() override {
        m_packet.reset();
    }
private:
    void serializeDefinition(QDataStream &, const QRemoteObjectSourceBase *);
    void serializeProperty(QDataStream &ds, const QRemoteObjectSourceBase *source, int internalIndex);
    void serializeProperties(const QRemoteObjectSourceBase *source);
    DataStreamPacket m_packet;
};

QMetaType transferTypeForEnum(QMetaType enumType);
QVariant encodeVariant(const QVariant &value);
QVariant decodeVariant(QVariant &&value, QMetaType metaType);

} // namespace QRemoteObjectPackets

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN_TAGGED(QRemoteObjectPackets::QRO_, QRemoteObjectPackets__QRO_,
                               /* not exported */)

#endif
