// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFSERIALIZER_P_H
#define QPROTOBUFSERIALIZER_P_H

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

#include <QtProtobuf/qabstractprotobufserializer.h>
#include <QtProtobuf/qprotobufpropertyordering.h>
#include <QtProtobuf/qprotobufserializer.h>
#include <QtProtobuf/qprotobufrepeatediterator.h>
#include <QtProtobuf/qtprotobuftypes.h>

#include <QtProtobuf/private/protobuffieldpresencechecker_p.h>
#include <QtProtobuf/private/qtprotobuflogging_p.h>
#include <QtProtobuf/private/qprotobufdeserializerbase_p.h>
#include <QtProtobuf/private/qprotobufselfcheckiterator_p.h>
#include <QtProtobuf/private/qprotobufserializerbase_p.h>

#include <QtCore/qendian.h>
#include <QtCore/qstring.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QProtobufSerializerImpl final : public QProtobufSerializerBase
{
public:
    explicit QProtobufSerializerImpl(QProtobufSerializerPrivate *parent);
    ~QProtobufSerializerImpl();

    const QByteArray &result() const { return m_result; }

    void reset();
    void serializeUnknownFields(const QProtobufMessage *message);

    QProtobufSerializerPrivate *m_parent = nullptr;

private:
    bool serializeEnum(QVariant &value,
                       const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo) override;
    bool serializeScalarField(const QVariant &value,
                              const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo) override;
    void serializeMessageFieldBegin() override;
    void serializeMessageFieldEnd(const QProtobufMessage *message,
                                  const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo) override;

    static QByteArray encodeHeader(int fieldNumber, QtProtobuf::WireTypes wireType);

    QByteArray m_result;
    QList<QByteArray> m_state;

    Q_DISABLE_COPY_MOVE(QProtobufSerializerImpl)
};

class QProtobufDeserializerImpl final : public QProtobufDeserializerBase
{
public:
    explicit QProtobufDeserializerImpl(QProtobufSerializerPrivate *parent);
    ~QProtobufDeserializerImpl();

    void reset(QByteArrayView data);

    QProtobufSelfcheckIterator m_it = {};

private:
    void setError(QAbstractProtobufSerializer::Error error, QAnyStringView errorString) override;
    bool deserializeEnum(QVariant &value,
                         const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo) override;
    int nextFieldIndex(QProtobufMessage *message) override;
    bool deserializeScalarField(QVariant &value,
                                const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo) override;

    qsizetype skipField(const QProtobufSelfcheckIterator &fieldBegin);
    void skipVarint();
    void skipLengthDelimited();
    void setUnexpectedEndOfStreamError();

    [[nodiscard]]
    static bool decodeHeader(QProtobufSelfcheckIterator &it, int &fieldIndex,
                             QtProtobuf::WireTypes &wireType);

    QtProtobuf::WireTypes m_wireType = QtProtobuf::WireTypes::Unknown;
    QList<QByteArrayView::iterator> m_state;
    QProtobufSerializerPrivate *m_parent = nullptr;

    Q_DISABLE_COPY_MOVE(QProtobufDeserializerImpl)
};

class QProtobufSerializerPrivate
{
public:
    // Serializer is interface function for serialize method
    using Serializer = QByteArray (*)(const QVariant &, const QByteArray &);
    // Deserializer is interface function for deserialize method
    using Deserializer = bool (*)(QProtobufSelfcheckIterator &, QVariant &);
    // Function checks if value in QVariant is considered to be non-ignorable.

    // SerializationHandlers contains set of objects that required for class
    // serializaion/deserialization
    struct ProtobufSerializationHandler
    {
        QMetaType metaType;
        Serializer serializer; // serializer assigned to class
        Deserializer deserializer; // deserializer assigned to class
        ProtobufFieldPresenceChecker::Function isPresent; // checks if contains non-ignorable value
        QtProtobuf::WireTypes wireType; // Serialization WireType
    };

    QProtobufSerializerPrivate();
    ~QProtobufSerializerPrivate() = default;

    template<typename T, QByteArray (*s)(const T &)>
    static QByteArray serializeWrapper(const QVariant &variantValue, const QByteArray &header)
    {
        return header + s(variantValue.value<T>());
    }

    template<typename T, QByteArray (*s)(const T &, const QByteArray &)>
    static QByteArray serializeNonPackedWrapper(const QVariant &variantValue,
                                                const QByteArray &header)
    {
        return s(variantValue.value<T>(), header);
    }

    void clearError();

    QAbstractProtobufSerializer::Error lastError =
            QAbstractProtobufSerializer::Error::UnknownType;
    QString lastErrorString;

    bool preserveUnknownFields = true;

    QProtobufSerializerImpl serializer;
    QProtobufDeserializerImpl deserializer;

private:
    Q_DISABLE_COPY_MOVE(QProtobufSerializerPrivate)
};

QT_END_NAMESPACE

#endif // QPROTOBUFSERIALIZER_P_H
