// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTREMOTEOBJECTGLOBAL_H
#define QTREMOTEOBJECTGLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore/qhash.h>
#include <QtCore/qurl.h>
#include <QtCore/qloggingcategory.h>
#include <QtRemoteObjects/qtremoteobjectsexports.h>

QT_BEGIN_NAMESPACE

struct QRemoteObjectSourceLocationInfo
{
    QRemoteObjectSourceLocationInfo() = default;
    QRemoteObjectSourceLocationInfo(const QString &typeName_, const QUrl &hostUrl_)
        : typeName(typeName_), hostUrl(hostUrl_) {}

    inline bool operator==(const QRemoteObjectSourceLocationInfo &other) const Q_DECL_NOTHROW
    {
        return other.typeName == typeName && other.hostUrl == hostUrl;
    }
    inline bool operator!=(const QRemoteObjectSourceLocationInfo &other) const Q_DECL_NOTHROW
    {
        return !(*this == other);
    }

    QString typeName;
    QUrl hostUrl;
};

inline QDebug operator<<(QDebug dbg, const QRemoteObjectSourceLocationInfo &info)
{
    dbg.nospace() << "SourceLocationInfo(" << info.typeName << ", " << info.hostUrl << ")";
    return dbg.space();
}

inline QDataStream& operator<<(QDataStream &stream, const QRemoteObjectSourceLocationInfo &info)
{
    return stream << info.typeName << info.hostUrl;
}

inline QDataStream& operator>>(QDataStream &stream, QRemoteObjectSourceLocationInfo &info)
{
    return stream >> info.typeName >> info.hostUrl;
}

typedef QPair<QString, QRemoteObjectSourceLocationInfo> QRemoteObjectSourceLocation;
typedef QHash<QString, QRemoteObjectSourceLocationInfo> QRemoteObjectSourceLocations;
typedef QHash<int, QByteArray> QIntHash;

QT_END_NAMESPACE
QT_DECL_METATYPE_EXTERN(QRemoteObjectSourceLocation, Q_REMOTEOBJECTS_EXPORT)
QT_DECL_METATYPE_EXTERN(QRemoteObjectSourceLocations, Q_REMOTEOBJECTS_EXPORT)
QT_DECL_METATYPE_EXTERN(QIntHash, /* not exported */)
QT_BEGIN_NAMESPACE

#define QCLASSINFO_REMOTEOBJECT_TYPE "RemoteObject Type"
#define QCLASSINFO_REMOTEOBJECT_SIGNATURE "RemoteObject Signature"

class QDataStream;

namespace QRemoteObjectStringLiterals {

// when QStringLiteral is used with the same string in different functions,
// it creates duplicate static data. Wrapping it in inline functions prevents it.

inline QString local() { return QStringLiteral("local"); }
inline QString localabstract() { return QStringLiteral("localabstract"); }
inline QString tcp() { return QStringLiteral("tcp"); }
inline QString CLASS() { return QStringLiteral("Class::%1"); }
inline QString MODEL() { return QStringLiteral("Model::%1"); }
inline QString QAIMADAPTER() { return QStringLiteral("QAbstractItemModelAdapter"); }

}

Q_DECLARE_LOGGING_CATEGORY(QT_REMOTEOBJECT)
Q_DECLARE_LOGGING_CATEGORY(QT_REMOTEOBJECT_MODELS)
Q_DECLARE_LOGGING_CATEGORY(QT_REMOTEOBJECT_IO)

namespace QtRemoteObjects {

Q_NAMESPACE_EXPORT(Q_REMOTEOBJECTS_EXPORT)

Q_REMOTEOBJECTS_EXPORT void copyStoredProperties(const QMetaObject *mo, const void *src, void *dst);
Q_REMOTEOBJECTS_EXPORT void copyStoredProperties(const QMetaObject *mo, const void *src, QDataStream &dst);
Q_REMOTEOBJECTS_EXPORT void copyStoredProperties(const QMetaObject *mo, QDataStream &src, void *dst);

QString getTypeNameAndMetaobjectFromClassInfo(const QMetaObject *& meta);

template <typename T>
void copyStoredProperties(const T *src, T *dst)
{
    copyStoredProperties(&T::staticMetaObject, src, dst);
}

template <typename T>
void copyStoredProperties(const T *src, QDataStream &dst)
{
    copyStoredProperties(&T::staticMetaObject, src, dst);
}

template <typename T>
void copyStoredProperties(QDataStream &src, T *dst)
{
    copyStoredProperties(&T::staticMetaObject, src, dst);
}

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

enum QRemoteObjectPacketTypeEnum
{
    Invalid = 0,
    Handshake,
    InitPacket,
    InitDynamicPacket,
    AddObject,
    RemoveObject,
    InvokePacket,
    InvokeReplyPacket,
    PropertyChangePacket,
    ObjectList,
    Ping,
    Pong
};
Q_ENUM_NS(QRemoteObjectPacketTypeEnum)

enum InitialAction {
    FetchRootSize,
    PrefetchData
};
Q_ENUM_NS(InitialAction)

}

QT_END_NAMESPACE

#endif // QTREMOTEOBJECTSGLOBAL_H
