// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFREGISTRATION_H
#define QPROTOBUFREGISTRATION_H

#if 0
#  pragma qt_sync_skip_header_check
#  pragma qt_sync_stop_processing
#endif

#include <QtProtobuf/qtprotobufexports.h>

#include <QtProtobuf/qprotobufpropertyordering.h>
#include <QtProtobuf/qprotobufrepeatediterator.h>
#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qmetatype.h>

QT_BEGIN_NAMESPACE

class QProtobufMessage;

namespace QtProtobuf {
using RegisterFunction = void (*)();
// This struct is used for type registrations in generated code
struct ProtoTypeRegistrar
{
    Q_PROTOBUF_EXPORT explicit ProtoTypeRegistrar(QtProtobuf::RegisterFunction initializer);
};
}

namespace QtProtobufPrivate {
extern Q_PROTOBUF_EXPORT void registerOrdering(QMetaType type, QProtobufPropertyOrdering ordering);

template <typename T, typename std::enable_if_t<std::is_enum<T>::value, bool> = true>
static std::optional<QList<T>> intToEnumList(const QList<QtProtobuf::int64> &v)
{
    QList<T> enumList;
    for (const auto &intValue : v)
        enumList.append(static_cast<T>(intValue.t));

    return enumList;
}

template <typename T, typename std::enable_if_t<std::is_enum<T>::value, bool> = true>
static QList<QtProtobuf::int64> enumToIntList(const QList<T> &v)
{
    QList<QtProtobuf::int64> intList;
    for (const auto enumValue : v)
        intList.append(QtProtobuf::int64(qToUnderlying(enumValue)));

    return intList;
}

template <typename T, typename std::enable_if_t<std::is_enum<T>::value, bool> = true>
static std::optional<QList<T>> stringToEnumList(const QStringList &v)
{
    static const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    QList<T> enumList;
    bool ok = false;
    for (const auto &stringValue : v) {
        T enumV = T(metaEnum.keyToValue(stringValue.toUtf8().data(), &ok));
        if (!ok)
            return std::nullopt;

        enumList.append(enumV);
    }

    return enumList;
}

template <typename T, typename std::enable_if_t<std::is_enum<T>::value, bool> = true>
static QStringList enumToStringList(const QList<T> &v)
{
    static const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    QStringList stringList;
    for (const auto enumValue : v)
        stringList.append(QString::fromUtf8(metaEnum.valueToKey(qToUnderlying(enumValue))));

    return stringList;
}

} // namespace QtProtobufPrivate

Q_PROTOBUF_EXPORT void qRegisterProtobufTypes();

template<typename T, QtProtobuf::if_protobuf_message<T> = true>
inline void qRegisterProtobufType()
{
    T::registerTypes();
    QMetaType::registerMutableView<
        QList<T>, QProtobufRepeatedIterator>(&QProtobufRepeatedIterator::fromList<T>);
    QtProtobufPrivate::registerOrdering(QMetaType::fromType<T>(), T::staticPropertyOrdering);
}

template <typename K, typename V, QtProtobuf::if_protobuf_map<K, V> = true>
inline void qRegisterProtobufMapType()
{
    QMetaType::registerMutableView<
        QHash<K, V>, QProtobufRepeatedIterator>(&QProtobufRepeatedIterator::fromHash<K, V>);
}

template <typename T, typename std::enable_if_t<std::is_enum<T>::value, bool> = true>
inline void qRegisterProtobufEnumType()
{
    QMetaType::registerConverter<QList<T>,
                                 QList<QtProtobuf::int64>>(QtProtobufPrivate::enumToIntList<T>);
    QMetaType::registerConverter<QList<QtProtobuf::int64>,
                                 QList<T>>(QtProtobufPrivate::intToEnumList<T>);
    QMetaType::registerConverter<QList<T>, QStringList>(QtProtobufPrivate::enumToStringList<T>);
    QMetaType::registerConverter<QStringList, QList<T>>(QtProtobufPrivate::stringToEnumList<T>);
}

QT_END_NAMESPACE

#endif // QPROTOBUFREGISTRATION_H
