// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include <QtProtobuf/qprotobufserializer.h>

#include <QtProtobuf/private/protobufscalarserializers_p.h>
#include <QtProtobuf/private/qprotobufmessage_p.h>
#include <QtProtobuf/private/qprotobufregistration_p.h>
#include <QtProtobuf/private/qprotobufserializer_p.h>
#include <QtProtobuf/private/qprotobufserializerbase_p.h>
#include <QtProtobuf/private/qtprotobufdefs_p.h>
#include <QtProtobuf/private/qtprotobufserializerhelpers_p.h>
#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qreadwritelock.h>

QT_BEGIN_NAMESPACE

/*!
    \class QProtobufSerializer
    \inmodule QtProtobuf
    \since 6.5
    \brief The QProtobufSerializer class is interface that represents
           basic functions for serialization/deserialization.

    The QProtobufSerializer class registers serializers/deserializers for
    classes implementing a protobuf message, inheriting \l QProtobufMessage. These
    classes are generated automatically, based on a \c{.proto} file, using the CMake
    function \l qt_add_protobuf or by running
    \l {The qtprotobufgen Tool} {qtprotobufgen} directly.
*/

using namespace Qt::StringLiterals;
using namespace QtProtobufPrivate;
using namespace ProtobufScalarSerializers;

template<std::size_t N>
using SerializerRegistryType =
        std::array<QProtobufSerializerPrivate::ProtobufSerializationHandler, N>;

namespace {

#define QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(Type, WireType)             \
    { QMetaType::fromType<Type>(),                                              \
      QProtobufSerializerPrivate::serializeWrapper<Type, serializeBasic<Type>>, \
      deserializeBasic<Type>, ProtobufFieldPresenceChecker::isPresent<Type>, WireType }
#define QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(ListType, Type)               \
    { QMetaType::fromType<ListType>(),                                                 \
      QProtobufSerializerPrivate::serializeWrapper<ListType, serializeListType<Type>>, \
      deserializeList<Type>, ProtobufFieldPresenceChecker::isPresent<ListType>,        \
      QtProtobuf::WireTypes::LengthDelimited }

#define QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(ListType, Type, WireType) \
    { QMetaType::fromType<ListType>(),                                                        \
      QProtobufSerializerPrivate::serializeNonPackedWrapper<ListType,                         \
                                                            serializeNonPackedList<Type>>,    \
      deserializeNonPackedList<Type>, ProtobufFieldPresenceChecker::isPresent<ListType>,      \
      WireType }

constexpr SerializerRegistryType<30> IntegratedTypesSerializers = { {
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(float, QtProtobuf::WireTypes::Fixed32),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(double, QtProtobuf::WireTypes::Fixed64),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::int32,
                                                    QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::int64,
                                                    QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::uint32,
                                                    QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::uint64,
                                                    QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::sint32,
                                                    QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::sint64,
                                                    QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::fixed32,
                                                    QtProtobuf::WireTypes::Fixed32),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::fixed64,
                                                    QtProtobuf::WireTypes::Fixed64),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::sfixed32,
                                                    QtProtobuf::WireTypes::Fixed32),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::sfixed64,
                                                    QtProtobuf::WireTypes::Fixed64),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QtProtobuf::boolean,
                                                    QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QString,
                                                    QtProtobuf::WireTypes::LengthDelimited),
        QT_CONSTRUCT_PROTOBUF_SERIALIZATION_HANDLER(QByteArray,
                                                    QtProtobuf::WireTypes::LengthDelimited),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::floatList, float),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::doubleList, double),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::int32List, QtProtobuf::int32),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::int64List, QtProtobuf::int64),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::uint32List,
                                                         QtProtobuf::uint32),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::uint64List,
                                                         QtProtobuf::uint64),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::sint32List,
                                                         QtProtobuf::sint32),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::sint64List,
                                                         QtProtobuf::sint64),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::fixed32List,
                                                         QtProtobuf::fixed32),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::fixed64List,
                                                         QtProtobuf::fixed64),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::sfixed32List,
                                                         QtProtobuf::sfixed32),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::sfixed64List,
                                                         QtProtobuf::sfixed64),
        QT_CONSTRUCT_PROTOBUF_LIST_SERIALIZATION_HANDLER(QtProtobuf::boolList, QtProtobuf::boolean),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QStringList, QString, QtProtobuf::WireTypes::LengthDelimited),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QByteArrayList, QByteArray, QtProtobuf::WireTypes::LengthDelimited),
} };

constexpr SerializerRegistryType<13> IntegratedNonPackedSerializers = { {
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(QtProtobuf::floatList, float,
                                                                    QtProtobuf::WireTypes::Fixed32),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(QtProtobuf::doubleList, double,
                                                                    QtProtobuf::WireTypes::Fixed64),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::int32List, QtProtobuf::int32, QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::int64List, QtProtobuf::int64, QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::uint32List, QtProtobuf::uint32, QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::uint64List, QtProtobuf::uint64, QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::sint32List, QtProtobuf::sint32, QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::sint64List, QtProtobuf::sint64, QtProtobuf::WireTypes::Varint),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::fixed32List, QtProtobuf::fixed32, QtProtobuf::WireTypes::Fixed32),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::fixed64List, QtProtobuf::fixed64, QtProtobuf::WireTypes::Fixed64),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::sfixed32List, QtProtobuf::sfixed32, QtProtobuf::WireTypes::Fixed32),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::sfixed64List, QtProtobuf::sfixed64, QtProtobuf::WireTypes::Fixed64),
        QT_CONSTRUCT_PROTOBUF_NON_PACKED_LIST_SERIALIZATION_HANDLER(
                QtProtobuf::boolList, QtProtobuf::boolean, QtProtobuf::WireTypes::Varint),
} };

template<std::size_t N>
std::optional<QProtobufSerializerPrivate::ProtobufSerializationHandler>
findIntegratedTypeHandlerImpl(QMetaType metaType, const SerializerRegistryType<N> &registry)
{
    typename SerializerRegistryType<N>::const_iterator it = std::find_if(
            registry.begin(), registry.end(),
            [&metaType](const QProtobufSerializerPrivate::ProtobufSerializationHandler &handler) {
                return handler.metaType == metaType;
            });
    if (it == registry.end())
        return std::nullopt;
    return { *it };
}

std::optional<QProtobufSerializerPrivate::ProtobufSerializationHandler>
findIntegratedTypeHandler(QMetaType metaType, bool nonPacked)
{
    if (nonPacked)
        return findIntegratedTypeHandlerImpl(metaType, IntegratedNonPackedSerializers);

    return findIntegratedTypeHandlerImpl(metaType, IntegratedTypesSerializers);
}
} // namespace

QProtobufSerializerImpl::QProtobufSerializerImpl(QProtobufSerializerPrivate *parent) : m_parent(parent)
{

}

QProtobufSerializerImpl::~QProtobufSerializerImpl() = default;

void QProtobufSerializerImpl::reset()
{
    m_state.clear();
    m_result = {};
}

void QProtobufSerializerImpl::serializeUnknownFields(const QProtobufMessage *message)
{
    if (m_parent->preserveUnknownFields) {
        // Restore any unknown fields we have stored away:
        for (const auto &fields :
             std::as_const(QProtobufMessagePrivate::get(message)->unknownEntries)) {
            m_result += fields.join();
        }
    }
}

bool QProtobufSerializerImpl::serializeEnum(QVariant &value,
                                            const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo)
{
    const auto fieldFlags = fieldInfo.fieldFlags();
    if (fieldFlags.testFlag(QtProtobufPrivate::FieldFlag::Repeated)) {
        if (!value.canConvert<QList<QtProtobuf::int64>>())
            return false;
        QList<QtProtobuf::int64> listValue = value.value<QList<QtProtobuf::int64>>();
        if (listValue.isEmpty())
            return true;

        if (fieldFlags.testFlag(QtProtobufPrivate::FieldFlag::NonPacked)) {
            const auto header = encodeHeader(fieldInfo.fieldNumber(),
                                             QtProtobuf::WireTypes::Varint);
            m_result.append(serializeNonPackedList<QtProtobuf::int64>(listValue, header));
        } else {
            m_result.append(encodeHeader(fieldInfo.fieldNumber(),
                                         QtProtobuf::WireTypes::LengthDelimited));
            m_result.append(serializeListType<QtProtobuf::int64>(listValue));
        }
    } else {
        if (!value.canConvert<QtProtobuf::int64>())
            return false;

        if (!ProtobufFieldPresenceChecker::isPresent<QtProtobuf::int64>(value)
            && !isOneofOrOptionalField(fieldInfo.fieldFlags())) {
            return true;
        }

        m_result.append(encodeHeader(fieldInfo.fieldNumber(), QtProtobuf::WireTypes::Varint));
        m_result.append(serializeBasic<QtProtobuf::int64>(value.value<QtProtobuf::int64>()));
    }
    return true;
}

bool QProtobufSerializerImpl::serializeScalarField(const QVariant &value,
                                                   const QProtobufFieldInfo &fieldInfo)
{
    const QtProtobufPrivate::FieldFlags flags = fieldInfo.fieldFlags();
    const bool isPacked = flags.testFlag(QtProtobufPrivate::FieldFlag::NonPacked);
    auto basicHandler = findIntegratedTypeHandler(value.metaType(), isPacked);
    // Is not a protobuf scalar value type
    if (!basicHandler)
        return false;

    // Field is empty
    if (!basicHandler->isPresent(value) && !isOneofOrOptionalField(flags))
        return true;

    const QByteArray header = encodeHeader(fieldInfo.fieldNumber(), basicHandler->wireType);
    m_result.append(basicHandler->serializer(value, header));
    return true;
}

void QProtobufSerializerImpl::serializeMessageFieldBegin()
{
    m_state.emplaceBack(std::move(m_result));
    m_result = {};
}

void QProtobufSerializerImpl::serializeMessageFieldEnd(const QProtobufMessage *message,
                                                       const QProtobufFieldInfo &fieldInfo)
{
    Q_ASSERT(!m_state.isEmpty());

    serializeUnknownFields(message);

    QByteArray last = m_state.takeLast();
    last.append(encodeHeader(fieldInfo.fieldNumber(), QtProtobuf::WireTypes::LengthDelimited));
    last.append(serializeVarintCommon<uint32_t>(m_result.size()));
    last.append(m_result);
    m_result = std::move(last);
}

QByteArray QProtobufSerializerImpl::encodeHeader(int fieldNumber, QtProtobuf::WireTypes wireType)
{
    // Encodes a property field number and its type into output bytes.

    // Header byte
    // Meaning    |  Field number |  Type
    // ---------- | ------------- | --------
    // bit number | 7  6  5  4  3 | 2  1  0

    // fieldNumber: The index of a property in parent object
    // wireType:    Serialization type used for the property with fieldNumber

    // Returns a varint-encoded fieldIndex and wireType
    [[maybe_unused]] static constexpr int32_t MaxFieldNumber =
        std::numeric_limits<uint32_t>::max() >> 3u;
    Q_ASSERT(fieldNumber >= 1 && fieldNumber <= MaxFieldNumber
             && (fieldNumber < 19000 || fieldNumber > 19999));
    uint32_t header = (uint(fieldNumber) << 3u) | uint32_t(wireType);
    return serializeVarintCommon<uint32_t>(header);
}

QProtobufDeserializerImpl::QProtobufDeserializerImpl(QProtobufSerializerPrivate *parent)
    : m_parent(parent)
{
}

QProtobufDeserializerImpl::~QProtobufDeserializerImpl()
    = default;

void QProtobufDeserializerImpl::reset(QByteArrayView data)
{
    m_it = QProtobufSelfcheckIterator::fromView(data);
    m_state.push_back(data.end());
    clearCachedValue();
}

void QProtobufDeserializerImpl::setError(QAbstractProtobufSerializer::Error error,
                                         QAnyStringView errorString)
{
    m_parent->lastError = error;
    m_parent->lastErrorString = errorString.toString();
}

bool QProtobufDeserializerImpl::deserializeEnum(QVariant &value,
                                                const QProtobufFieldInfo &fieldInfo)
{
    const auto fieldFlags = fieldInfo.fieldFlags();
    if (fieldFlags.testFlag(QtProtobufPrivate::FieldFlag::Repeated)) {
        QMetaType metaType = value.metaType();
        value.convert(QMetaType::fromType<QList<QtProtobuf::int64>>());
        bool result = false;
        if (m_wireType == QtProtobuf::WireTypes::Varint) {
            result = deserializeNonPackedList<QtProtobuf::int64>(m_it, value);
        } else if (m_wireType == QtProtobuf::WireTypes::LengthDelimited) {
            result = deserializeList<QtProtobuf::int64>(m_it, value);
        }
        value.convert(metaType);
        return result;
    }

    return deserializeBasic<QtProtobuf::int64>(m_it, value);
}

int QProtobufDeserializerImpl::nextFieldIndex(QProtobufMessage *message)
{
    Q_ASSERT(message);

    const auto *ordering = message->propertyOrdering();
    Q_ASSERT(ordering != nullptr);

    while (m_it.isValid() && m_it != m_state.last()) {
        // Each iteration we expect iterator is setup to beginning of next chunk
        int fieldNumber = QtProtobuf::InvalidFieldNumber;
        const QProtobufSelfcheckIterator fieldBegin = m_it; // copy this, we may need it later
        if (!decodeHeader(m_it, fieldNumber, m_wireType)) {
            setError(QAbstractProtobufSerializer::Error::InvalidHeader,
                     "Message received doesn't contain valid header byte.");
            return -1;
        }

        int index = ordering->indexOfFieldNumber(fieldNumber);
        if (index == -1) {
            // This is an unknown field, it may have been added in a later revision
            // of the Message we are currently deserializing. We must store the
            // bytes for this field and re-emit them later if this message is
            // serialized again.
            if (auto length = skipField(fieldBegin); length < 0) {
                return -1;
            } else if (length > 0 && m_parent->preserveUnknownFields) {
                QByteArrayView fieldData(fieldBegin.data(), length);
                QProtobufMessagePrivate::storeUnknownEntry(message, fieldData, fieldNumber);
            }
            continue;
        }

        if (ordering->fieldFlags(index).testAnyFlags({ QtProtobufPrivate::FieldFlag::Message,
                                                       QtProtobufPrivate::FieldFlag::Map })) {
            auto opt = deserializeVarintCommon<QtProtobuf::uint64>(m_it);
            if (!opt) {
                setUnexpectedEndOfStreamError();
                return -1;
            }

            quint64 length = *opt;
            if (!m_it.isValid() || quint64(m_it.bytesLeft()) < length
                || length > quint64(QByteArray::maxSize())) {
                setUnexpectedEndOfStreamError();
                return -1;
            }

            m_state.push_back(m_it.data() + length);
        }
        return index;
    }

    if (!m_it.isValid())
        setUnexpectedEndOfStreamError();

    m_state.pop_back();
    return -1;
}

bool QProtobufDeserializerImpl::deserializeScalarField(QVariant &value,
                                                       const QtProtobufPrivate::QProtobufFieldInfo
                                                           &fieldInfo)
{
    QMetaType metaType = value.metaType();

    // All repeated scalar types should have LenghtDelimited wiretype.
    // If the wiretype received from the wire is not LenghtDelimited,
    // that most probably means that we received the list in wrong
    // format. This can happen because of mismatch of the field packed
    // option in the protobuf schema on the wire ends.
    // look for non-packed list in this case. Otherwise it's the regular
    // packed repeated field.
    // See the conformance tests
    // Required.Proto3.ProtobufInput.ValidDataRepeated.*.UnpackedInput
    // for details.
    bool isNonPacked = m_wireType != QtProtobuf::WireTypes::LengthDelimited &&
        fieldInfo.fieldFlags().testFlags(FieldFlag::Repeated);

    auto basicHandler = findIntegratedTypeHandler(metaType, isNonPacked);
    if (!basicHandler)
        return false;

    if (basicHandler->wireType != m_wireType) {
        setError(QAbstractProtobufSerializer::Error::InvalidHeader,
                 QCoreApplication::translate("QtProtobuf",
                                             "Invalid wiretype for the %1 "
                                             "field number %1. Expected %2, received %3")
                     .arg(QString::fromUtf8(metaType.name()))
                     .arg(fieldInfo.fieldNumber())
                     .arg(basicHandler ? static_cast<int>(basicHandler->wireType) : -1)
                     .arg(static_cast<int>(m_wireType)));
        value.clear();
    } else if (!basicHandler->deserializer(m_it, value)) {
        value.clear();
        setUnexpectedEndOfStreamError();
    }

    return true;
}

qsizetype QProtobufDeserializerImpl::skipField(const QProtobufSelfcheckIterator &fieldBegin)
{
    switch (m_wireType) {
    case QtProtobuf::WireTypes::Varint:
        skipVarint();
        break;
    case QtProtobuf::WireTypes::Fixed32:
        m_it += sizeof(decltype(QtProtobuf::fixed32::t));
        break;
    case QtProtobuf::WireTypes::Fixed64:
        m_it += sizeof(decltype(QtProtobuf::fixed64::t));
        break;
    case QtProtobuf::WireTypes::LengthDelimited:
        skipLengthDelimited();
        break;
    case QtProtobuf::WireTypes::Unknown:
    default:
        Q_UNREACHABLE();
        return 0;
    }

    if (!m_it.isValid()) {
        setUnexpectedEndOfStreamError();
        return -1;
    }

    return std::distance(fieldBegin, m_it);
}

void QProtobufDeserializerImpl::skipVarint()
{
    while ((*m_it) & 0x80)
        ++m_it;
    ++m_it;
}

void QProtobufDeserializerImpl::skipLengthDelimited()
{
    //Get length of length-delimited field
    auto opt = deserializeVarintCommon<QtProtobuf::uint64>(m_it);
    if (!opt) {
        m_it += m_it.bytesLeft() + 1;
        return;
    }
    QtProtobuf::uint64 length = opt.value();
    m_it += length;
}

void QProtobufDeserializerImpl::setUnexpectedEndOfStreamError()
{
    setError(QAbstractProtobufSerializer::Error::UnexpectedEndOfStream,
             QCoreApplication::translate("QtProtobuf", "Unexpected end of stream"));
}

bool QProtobufDeserializerImpl::decodeHeader(QProtobufSelfcheckIterator &it, int &fieldIndex,
                                             QtProtobuf::WireTypes &wireType)
{
    if (it.bytesLeft() == 0)
        return false;
    auto opt = deserializeVarintCommon<uint32_t>(it);
    if (!opt)
        return false;
    uint32_t header = opt.value();
    wireType = static_cast<QtProtobuf::WireTypes>(header & 0b00000111);
    fieldIndex = header >> 3;

    constexpr int maxFieldIndex = (1 << 29) - 1;
    return fieldIndex <= maxFieldIndex && fieldIndex > 0
        && (wireType == QtProtobuf::WireTypes::Varint || wireType == QtProtobuf::WireTypes::Fixed64
            || wireType == QtProtobuf::WireTypes::Fixed32
            || wireType == QtProtobuf::WireTypes::LengthDelimited);
}

QProtobufSerializerPrivate::QProtobufSerializerPrivate() : serializer(this), deserializer(this)
{
}

void QProtobufSerializerPrivate::clearError()
{
    lastError = QAbstractProtobufSerializer::Error::None;
    lastErrorString.clear();
}

/*!
    Constructs a new serializer instance.
*/
QProtobufSerializer::QProtobufSerializer() : d_ptr(new QProtobufSerializerPrivate())
{
}

/*!
    Destroys the serializer instance.
*/
QProtobufSerializer::~QProtobufSerializer() = default;

QByteArray QProtobufSerializer::serializeMessage(const QProtobufMessage *message) const
{
    d_ptr->clearError();
    d_ptr->serializer.reset();
    d_ptr->serializer.serializeMessage(message);
    d_ptr->serializer.serializeUnknownFields(message);
    auto result = d_ptr->serializer.result();
    d_ptr->serializer.reset();
    return result;
}

bool QProtobufSerializer::deserializeMessage(QProtobufMessage *message, QByteArrayView data) const
{
    d_ptr->clearError();
    d_ptr->deserializer.reset(data);
    d_ptr->deserializer.deserializeMessage(message);
    d_ptr->deserializer.reset({});
    return d_ptr->lastError == QAbstractProtobufSerializer::Error::None;
}

/*!
   Returns the last deserialization error for the serializer instance.
   \sa lastErrorString()
*/
QAbstractProtobufSerializer::Error QProtobufSerializer::lastError() const
{
    return d_ptr->lastError;
}

/*!
   Returns the last deserialization error string for the serializer instance.
   \sa lastError()
*/
QString QProtobufSerializer::lastErrorString() const
{
    return d_ptr->lastErrorString;
}

/*!
    Controls whether the unknown fields received from the wire should be
    stored in the resulting message or if it should be omitted, based
    on \a preserveUnknownFields.
    \since 6.7
*/
void QProtobufSerializer::shouldPreserveUnknownFields(bool preserveUnknownFields)
{
    d_ptr->preserveUnknownFields = preserveUnknownFields;
}

QT_END_NAMESPACE
