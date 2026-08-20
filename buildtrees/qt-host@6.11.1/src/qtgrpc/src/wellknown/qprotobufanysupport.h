// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef Q_PROTOBUF_ANYSUPPORT_H
#define Q_PROTOBUF_ANYSUPPORT_H

#include <QtProtobufWellKnownTypes/qtprotobufwellknowntypesexports.h>

#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/qanystringview.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>

#include <utility>
#include <optional>
#include <type_traits>

QT_BEGIN_NAMESPACE

class QAbstractProtobufSerializer;

namespace QtProtobuf {
class AnyPrivate;
class Any final : public QProtobufMessage
{
    Q_GADGET_EXPORT(Q_PROTOBUFWELLKNOWNTYPES_EXPORT)
    Q_PROPERTY(QString typeUrl READ typeUrl WRITE setTypeUrl SCRIPTABLE true)
    Q_PROPERTY(QByteArray value READ value WRITE setValue SCRIPTABLE true)
public:
    static void registerTypes();

    Q_PROTOBUFWELLKNOWNTYPES_EXPORT Any();
    Q_PROTOBUFWELLKNOWNTYPES_EXPORT ~Any();
    Q_PROTOBUFWELLKNOWNTYPES_EXPORT Any(const Any &other);
    Q_PROTOBUFWELLKNOWNTYPES_EXPORT Any &operator=(const Any &other);
    Any(Any &&other) noexcept = default;
    Any &operator =(Any &&other) noexcept = default;
    void swap(Any &other) noexcept
    {
        QProtobufMessage::swap(other);
    }

    Q_PROTOBUFWELLKNOWNTYPES_EXPORT QString typeUrl() const;
    Q_PROTOBUFWELLKNOWNTYPES_EXPORT QByteArray value() const;
    Q_PROTOBUFWELLKNOWNTYPES_EXPORT void setTypeUrl(const QString &typeUrl);
    Q_PROTOBUFWELLKNOWNTYPES_EXPORT void setValue(const QByteArray &value);

    template <typename T>
    std::optional<T> unpack(QAbstractProtobufSerializer *serializer) const
    {
        static_assert(QtProtobuf::has_q_protobuf_object_macro_v<T>,
                      "T must have the Q_PROTOBUF_OBJECT macro");

        std::optional<T> opt(std::in_place);
        if (!unpackImpl(serializer, &opt.value()))
             opt.reset();
        return opt;
    }

    template <typename T>
    static Any fromMessage(QAbstractProtobufSerializer *serializer, const T &message,
                           QAnyStringView typeUrlPrefix)
    {
        if constexpr (std::is_same_v<T, Any>)
            return fromAnyMessageImpl(serializer, &message, typeUrlPrefix);

        static_assert(QtProtobuf::has_q_protobuf_object_macro_v<T>,
                      "T must have the Q_PROTOBUF_OBJECT macro");
        return fromMessageImpl(serializer, &message, typeUrlPrefix);
    }

    template <typename T>
    static Any fromMessage(QAbstractProtobufSerializer *serializer, const T &message)
    {
        if constexpr (std::is_same_v<T, Any>)
            return fromAnyMessageImpl(serializer, &message);

        static_assert(QtProtobuf::has_q_protobuf_object_macro_v<T>,
                      "T must have the Q_PROTOBUF_OBJECT macro");
        return fromMessageImpl(serializer, &message);
    }

private:
    Q_DECLARE_PRIVATE(Any)

    Q_PROTOBUFWELLKNOWNTYPES_EXPORT bool unpackImpl(QAbstractProtobufSerializer *serializer,
                                                    QProtobufMessage *message) const;
    static Q_PROTOBUFWELLKNOWNTYPES_EXPORT Any
    fromMessageImpl(QAbstractProtobufSerializer *serializer, const QProtobufMessage *message,
                    QAnyStringView typeUrlPrefix);
    static Q_PROTOBUFWELLKNOWNTYPES_EXPORT Any
    fromAnyMessageImpl(QAbstractProtobufSerializer *serializer, const Any *message,
                       QAnyStringView typeUrlPrefix);

    static Q_PROTOBUFWELLKNOWNTYPES_EXPORT Any
    fromMessageImpl(QAbstractProtobufSerializer *serializer, const QProtobufMessage *message);
    static Q_PROTOBUFWELLKNOWNTYPES_EXPORT Any
    fromAnyMessageImpl(QAbstractProtobufSerializer *serializer, const Any *message);

    Q_PROTOBUFWELLKNOWNTYPES_EXPORT
    friend bool comparesEqual(const Any &lhs, const Any &rhs) noexcept;

    Q_DECLARE_EQUALITY_COMPARABLE(Any)
};

template <>
Q_PROTOBUFWELLKNOWNTYPES_EXPORT std::optional<Any>
Any::unpack<Any>(QAbstractProtobufSerializer *serializer) const;

Q_DECLARE_SHARED_NS(QtProtobuf, Any)

} // namespace QtProtobuf

QT_END_NAMESPACE

#endif // Q_PROTOBUF_ANYSUPPORT_H
