// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include <QtProtobuf/qprotobufpropertyordering.h>

#include <QtProtobuf/private/qprotobufpropertyorderingbuilder_p.h>
#include <QtProtobuf/private/qtprotobufdefs_p.h>
#include <QtProtobuf/private/qtprotobuflogging_p.h>
#include <QtProtobuf/qprotobufregistration.h>

#include <QtCore/q20utility.h>
#include <QtCore/qhash.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qpair.h>
#include <QtCore/qreadwritelock.h>
#include <QtCore/qstring.h>

#include <limits>

namespace {
/*!
    \internal
    The ProtobufOrderingRegistry stores the mapping between
    QtProtobufPrivate::QProtobufPropertyOrdering and QMetaType.
*/
struct ProtobufOrderingRegistry
{
    using ProtobufOrderingRegistryRecord =
        QPair<QMetaType, QtProtobufPrivate::QProtobufPropertyOrdering>;

    void registerOrdering(QMetaType type, QtProtobufPrivate::QProtobufPropertyOrdering ordering)
    {
        QWriteLocker locker(&m_lock);
        m_registry[ordering.messageFullName().toString()] =
            ProtobufOrderingRegistryRecord(type, ordering);
    }

    QtProtobufPrivate::QProtobufPropertyOrdering getOrdering(QMetaType type) const
    {
        QtProtobufPrivate::QProtobufPropertyOrdering ordering{ nullptr };

        QReadLocker locker(&m_lock);
        for (auto it = m_registry.constBegin(); it != m_registry.constEnd(); ++it) {
            if (type == it.value().first) {
                ordering = it.value().second;
                break;
            }
        }

        return ordering;
    }

    ProtobufOrderingRegistryRecord
    findMessageByName(const QString &messageName) const
    {
        ProtobufOrderingRegistryRecord record = {
            QMetaType(QMetaType::UnknownType), { nullptr }
        };

        QReadLocker locker(&m_lock);
        auto it = m_registry.constFind(messageName);
        if (it != m_registry.constEnd())
            record = it.value();
        return record;
    }

private:
    mutable QReadWriteLock m_lock;
    QHash<QString, ProtobufOrderingRegistryRecord> m_registry;
};

Q_GLOBAL_STATIC(ProtobufOrderingRegistry, orderingRegistry)

} // namespace

QT_BEGIN_NAMESPACE

namespace QtProtobufPrivate {

static_assert(std::is_trivially_destructible_v<QProtobufPropertyOrdering>);
static_assert(std::is_same_v<std::underlying_type_t<QtProtobufPrivate::FieldFlag>, uint>);

constexpr uint jsonNameOffsetsOffset = 0;
// Use this constant to make the +/- 1 more easily readable
constexpr int NullTerminator = 1;
QUtf8StringView QProtobufPropertyOrdering::messageFullName() const
{
    Q_ASSERT(data);
    const char *name = char_data();
    return { name, qsizetype(data->fullPackageNameSize) };
}

QUtf8StringView QProtobufPropertyOrdering::jsonName(int index) const
{
    Q_ASSERT(data);
    if (Q_UNLIKELY(index < 0 || index >= fieldCount()))
        return {}; // Invalid index (out of bounds)
    const uint stringOffset = uint_dataForIndex(index, jsonNameOffsetsOffset);
    const char *name = char_data() + stringOffset;
    // This is fine because we store an extra offset for end-of-string
    const uint nameLength =
        uint_dataForIndex(index + 1, jsonNameOffsetsOffset) - NullTerminator - stringOffset;
    return { name, qsizetype(nameLength) };
}

int QProtobufPropertyOrdering::fieldNumber(int index) const
{
    Q_ASSERT(data);
    if (Q_UNLIKELY(index < 0 || index >= fieldCount()))
        return -1; // Invalid index (out of bounds)
    uint fieldNumber = uint_dataForIndex(index, data->fieldNumberOffset);
    if (Q_UNLIKELY(fieldNumber > uint(std::numeric_limits<int>::max())))
        return -1;
    return int(fieldNumber);
}

int QProtobufPropertyOrdering::propertyIndex(int index) const
{
    Q_ASSERT(data);
    if (Q_UNLIKELY(index < 0 || index >= fieldCount()))
        return -1; // Invalid index (out of bounds)
    uint propertyIndex = uint_dataForIndex(index, data->propertyIndexOffset);
    if (Q_UNLIKELY(propertyIndex > uint(std::numeric_limits<int>::max())))
        return -1;
    return int(propertyIndex);
}

int QProtobufPropertyOrdering::indexOfFieldNumber(int fieldNumber) const
{
    Q_ASSERT(data);
    if (Q_LIKELY(fieldNumber > 0)) {
        for (int i = 0; i < fieldCount(); ++i) {
            if (uint_dataForIndex(i, data->fieldNumberOffset) == static_cast<uint>(fieldNumber))
                return i;
        }
    }
    return -1;
}

FieldFlags QProtobufPropertyOrdering::fieldFlags(int index) const
{
    Q_ASSERT(data);
    if (Q_UNLIKELY(index < 0 || index >= fieldCount()))
        return {}; // Invalid index (out of bounds)
    return FieldFlags { int(uint_dataForIndex(index, data->flagsOffset)) };
}

/*!
    \internal
    Access the generated uint* metadata of message properties.
    [ Data, uint_data , char_data ] > Message_metadata
    [ . . , ^ x x x x , . . . . . ]
*/
uint *QProtobufPropertyOrdering::uint_data(NonConstTag /*unused*/) const
{
    Q_ASSERT(data);
    Q_ASSERT(data->version == 0);
    quintptr dataPtr = quintptr(data);
    dataPtr += sizeof(Data);
#if 0 // if Data is ever not just a bunch of uints, remove the #if 0
        if (dataPtr % alignof(uint) != 0)
            dataPtr += alignof(uint) - (dataPtr % alignof(uint));
#else
    static_assert(alignof(Data) == alignof(uint));
#endif
    return reinterpret_cast<uint *>(dataPtr);
}

const uint *QProtobufPropertyOrdering::uint_data() const
{
    return uint_data(NonConstTag{});
}

/*!
    \internal
    Access the generated char* metadata of message properties.
    [ Data, uint_data , char_data ] > Message_metadata
    [ . . , . . . . . , ^ x x x x ]
*/
char *QProtobufPropertyOrdering::char_data(NonConstTag /*unused*/) const
{
    Q_ASSERT(data);
    uint* uintDataPtr = uint_data(NonConstTag{});
    // char_data starts after the last field flag (flagsOffset + numFields)
    uint charDataOffset = data->flagsOffset + data->numFields;
    return reinterpret_cast<char *>(uintDataPtr + charDataOffset);
}

const char *QProtobufPropertyOrdering::char_data() const
{
    return char_data(NonConstTag{});
}

const uint &QProtobufPropertyOrdering::uint_dataForIndex(int index, uint offset) const
{
    Q_ASSERT(data);
    Q_ASSERT(index >= 0);
    Q_ASSERT(uint(index) < data->numFields
             || (offset == jsonNameOffsetsOffset && uint(index) == data->numFields));
    return *(uint_data() + offset + index);
}

void registerOrdering(QMetaType type, QProtobufPropertyOrdering ordering)
{
    orderingRegistry->registerOrdering(type, ordering);
}

QProtobufPropertyOrdering getOrderingByMetaType(QMetaType type)
{
    return orderingRegistry->getOrdering(type);
}

QProtobufMessagePointer constructMessageByName(const QString &messageType)
{
    qRegisterProtobufTypes();
    ProtobufOrderingRegistry::ProtobufOrderingRegistryRecord messageOrderingRecord =
        orderingRegistry->findMessageByName(messageType);
    QMetaType type = messageOrderingRecord.first;
    QProtobufMessagePointer pointer;
    if (type.id() != QMetaType::UnknownType) {
        void *msg = type.create();
        pointer.reset(static_cast<QProtobufMessage *>(msg));
        return pointer;
    }
    qProtoWarning() << "Unable to find protobuf message with name" << messageType
                    << ". Message is not registered.";
    return pointer;
}

class QProtobufPropertyOrderingBuilderPrivate
{
public:
    struct FieldDefinition
    {
        QByteArray jsonName;
        uint fieldNumber;
        uint propertyIndex;
        FieldFlags flags;
    };

    std::vector<FieldDefinition> fields;
    QByteArray packageName;
};

QProtobufPropertyOrderingBuilder::QProtobufPropertyOrderingBuilder(QByteArray packageName)
    : d_ptr(new QProtobufPropertyOrderingBuilderPrivate())
{

    d_ptr->packageName = std::move(packageName);
}

QProtobufPropertyOrderingBuilder::~QProtobufPropertyOrderingBuilder()
{
    delete d_ptr;
}

void QProtobufPropertyOrderingBuilder::addV0Field(QByteArray jsonName, uint fieldNumber,
                                                  uint propertyIndex, FieldFlags flags)
{
    Q_D(QProtobufPropertyOrderingBuilder);
    Q_ASSERT_X(fieldNumber <= ProtobufFieldNumMax && fieldNumber >= ProtobufFieldNumMin,
               "QProtobufPropertyOrderingBuilder::addV0Field",
               "Field number is out of the valid field number range.");
    Q_ASSERT(d->fields.size() < ProtobufFieldMaxCount);
    d->fields.push_back({ std::move(jsonName), fieldNumber, propertyIndex, flags });
}

/*!
    \internal
    Builds the QProtobufPropertyOrdering object from the builder data.

    The Data struct must be manually destructed (var->~Data()) and then
    free()d by the caller.
*/
QProtobufPropertyOrdering::Data *QProtobufPropertyOrderingBuilder::build() const
{
    Q_D(const QProtobufPropertyOrderingBuilder);

    if (d->fields.size() > ProtobufFieldMaxCount)
        return nullptr;

    qsizetype charSpaceNeeded = NullTerminator + d->packageName.size() + NullTerminator;
    for (const auto &field : d->fields)
        charSpaceNeeded += field.jsonName.size() + NullTerminator;

    const size_t uintSpaceNeeded = sizeof(QProtobufPropertyOrdering::Data)
        + d->fields.size() * sizeof(uint) * 4 + sizeof(uint) /* eos marker offset */;

    static_assert(sizeof(QProtobufPropertyOrdering::Data) == 24,
                  "Data size changed, update builder code");

    const size_t spaceNeeded = uintSpaceNeeded + charSpaceNeeded;

    auto *storage = static_cast<char *>(calloc(1, spaceNeeded));
    auto *data = new (storage) QProtobufPropertyOrdering::Data();
    auto raii = qScopeGuard([data] { data->~Data(); free(data); });

    data->version = 0u;
    data->numFields = uint(d->fields.size());
    data->fieldNumberOffset = uint(d->fields.size() * 1 + 1);
    data->propertyIndexOffset = uint(d->fields.size() * 2 + 1);
    data->flagsOffset = uint(d->fields.size() * 3 + 1);
    data->fullPackageNameSize = uint(d->packageName.size());

    using NonConstTag = QProtobufPropertyOrdering::NonConstTag;
    QProtobufPropertyOrdering ordering{data};

    uint *uintData = ordering.uint_data(NonConstTag{});
    uint jsonArrayOffset = data->fullPackageNameSize + NullTerminator;
    for (uint i = 0; i < data->numFields; ++i) {
        const auto &field = d->fields[i];
        if (field.fieldNumber > ProtobufFieldNumMax || field.fieldNumber < ProtobufFieldNumMin)
            return nullptr;

        uintData[i] = jsonArrayOffset;
        jsonArrayOffset += field.jsonName.size() + NullTerminator;
        uintData[i + data->fieldNumberOffset] = field.fieldNumber;
        uintData[i + data->propertyIndexOffset] = field.propertyIndex;
        uintData[i + data->flagsOffset] = uint(field.flags.toInt());
    }
    uintData[d->fields.size()] = jsonArrayOffset;
    Q_ASSERT(q20::cmp_equal(jsonArrayOffset + NullTerminator, charSpaceNeeded));

    char *charData = ordering.char_data(NonConstTag{});
    [[maybe_unused]]
    char * const charStart = charData;
    charData = std::copy_n(d->packageName.constData(), d->packageName.size() + NullTerminator,
                           charData);
    for (auto &field : d->fields) {
        charData = std::copy_n(field.jsonName.constData(), field.jsonName.size() + NullTerminator,
                               charData);
    }
    charData[0] = '\0'; // Empty string at the end of the char array
    ++charData; // Trailing null terminator
    Q_ASSERT(std::distance(charStart, charData) == charSpaceNeeded);
    Q_ASSERT(quintptr(charData) - quintptr(data) == quintptr(spaceNeeded));

    raii.dismiss();
    return data;
}
} // namespace QtProtobufPrivate

QT_END_NAMESPACE
