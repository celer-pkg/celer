// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qbinaryjson_p.h"

#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>

#include <private/qbinaryjsonarray_p.h>
#include <private/qbinaryjsonobject_p.h>

QT_BEGIN_NAMESPACE

/*!
    \namespace QBinaryJson
    \inmodule QtCore5Compat
    \brief Contains functions for converting QJsonDocument to and from JSON binary format.

    This namespace provides utility functions to keep compatibility with
    older code, which uses the JSON binary format for serializing JSON. Qt JSON
    types can be converted to Qt CBOR types, which can in turn be serialized
    into the CBOR binary format and vice versa.
*/

/*! \enum QBinaryJson::DataValidation

    This enum is used to tell QJsonDocument whether to validate the binary data
    when converting to a QJsonDocument using fromBinaryData() or fromRawData().

    \value Validate Validate the data before using it. This is the default.
    \value BypassValidation Bypasses data validation. Only use if you received the
    data from a trusted place and know it's valid, as using of invalid data can crash
    the application.
*/

namespace QBinaryJson {

/*!
    Creates a QJsonDocument that uses the first \a size bytes from
    \a data. It assumes \a data contains a binary encoded JSON document.
    The created document does not take ownership of \a data. The data is
    copied into a different data structure, and the original data can be
    deleted or modified afterwards.

    \a data has to be aligned to a 4 byte boundary.

    \a validation decides whether the data is checked for validity before being used.
    By default the data is validated. If the \a data is not valid, the method returns
    a null document.

    Returns a QJsonDocument representing the data.

    \note The binary JSON encoding is only retained for backwards
    compatibility. It is undocumented and restrictive in the maximum size of JSON
    documents that can be encoded. Qt JSON types can be converted to Qt CBOR types,
    which can in turn be serialized into the CBOR binary format and vice versa. The
    CBOR format is a well-defined and less restrictive binary representation for a
    superset of JSON.

    \note Before Qt 5.15, the caller had to guarantee that \a data would not be
    deleted or modified as long as any QJsonDocument, QJsonObject or QJsonArray
    still referenced the data. From Qt 5.15 on, this is not necessary anymore.

    \sa toRawData(), fromBinaryData(), DataValidation, QCborValue
*/
QJsonDocument fromRawData(const char *data, int size, DataValidation validation)
{
    if (quintptr(data) & 3) {
        qWarning("QJsonDocument::fromRawData: data has to have 4 byte alignment");
        return QJsonDocument();
    }

    if (size < 0 || uint(size) < sizeof(QBinaryJsonPrivate::Header) + sizeof(QBinaryJsonPrivate::Base))
        return QJsonDocument();

    std::unique_ptr<QBinaryJsonPrivate::ConstData> binaryData
            = std::make_unique<QBinaryJsonPrivate::ConstData>(data, size);

    return (validation == BypassValidation || binaryData->isValid())
            ? binaryData->toJsonDocument()
            : QJsonDocument();
}

/*!
    Returns the raw binary representation of \a document.
    \a size will contain the size of the returned data.

    This method is useful to e.g. stream the JSON document
    in its binary form to a file.

    \note The binary JSON encoding is only retained for backwards
    compatibility. It is undocumented and restrictive in the maximum size of JSON
    documents that can be encoded. Qt JSON types can be converted to Qt CBOR types,
    which can in turn be serialized into the CBOR binary format and vice versa. The
    CBOR format is a well-defined and less restrictive binary representation for a
    superset of JSON.

    \sa fromRawData(), fromBinaryData(), toBinaryData(), QCborValue
*/
const char *toRawData(const QJsonDocument &document, int *size)
{
    if (document.isNull()) {
        *size = 0;
        return nullptr;
    }

    char *rawData = nullptr;
    uint rawDataSize = 0;
    if (document.isObject()) {
        QBinaryJsonObject o = QBinaryJsonObject::fromJsonObject(document.object());
        rawData = o.takeRawData(&rawDataSize);
    } else {
        QBinaryJsonArray a = QBinaryJsonArray::fromJsonArray(document.array());
        rawData = a.takeRawData(&rawDataSize);
    }

    // It would be quite miraculous if not, as we should have hit the 128MB limit then.
    Q_ASSERT(rawDataSize <= uint(std::numeric_limits<int>::max()));

    *size = static_cast<int>(rawDataSize);
    return rawData;
}

/*!
    Creates a QJsonDocument from \a data.

    \a validation decides whether the data is checked for validity before being used.
    By default the data is validated. If the \a data is not valid, the method returns
    a null document.

    \note The binary JSON encoding is only retained for backwards
    compatibility. It is undocumented and restrictive in the maximum size of JSON
    documents that can be encoded. Qt JSON types can be converted to Qt CBOR types,
    which can in turn be serialized into the CBOR binary format and vice versa. The
    CBOR format is a well-defined and less restrictive binary representation for a
    superset of JSON.

    \sa toBinaryData(), fromRawData(), DataValidation, QCborValue
*/
QJsonDocument fromBinaryData(const QByteArray &data, DataValidation validation)
{
    if (uint(data.size()) < sizeof(QBinaryJsonPrivate::Header) + sizeof(QBinaryJsonPrivate::Base))
        return QJsonDocument();

    QBinaryJsonPrivate::Header h;
    memcpy(&h, data.constData(), sizeof(QBinaryJsonPrivate::Header));
    QBinaryJsonPrivate::Base root;
    memcpy(&root, data.constData() + sizeof(QBinaryJsonPrivate::Header),
           sizeof(QBinaryJsonPrivate::Base));

    const uint size = sizeof(QBinaryJsonPrivate::Header) + root.size;
    if (h.tag != QJsonDocument::BinaryFormatTag || h.version != 1U || size > uint(data.size()))
        return QJsonDocument();

    std::unique_ptr<QBinaryJsonPrivate::ConstData> d
            = std::make_unique<QBinaryJsonPrivate::ConstData>(data.constData(), size);

    return (validation == BypassValidation || d->isValid())
            ? d->toJsonDocument()
            : QJsonDocument();
}

/*!
    Returns a binary representation of \a document.

    The binary representation is also the native format used internally in Qt,
    and is very efficient and fast to convert to and from.

    The binary format can be stored on disk and interchanged with other applications
    or computers. fromBinaryData() can be used to convert it back into a
    JSON document.

    \note The binary JSON encoding is only retained for backwards
    compatibility. It is undocumented and restrictive in the maximum size of JSON
    documents that can be encoded. Qt JSON types can be converted to Qt CBOR types,
    which can in turn be serialized into the CBOR binary format and vice versa. The
    CBOR format is a well-defined and less restrictive binary representation for a
    superset of JSON.

    \sa fromBinaryData(), QCborValue
*/
QByteArray toBinaryData(const QJsonDocument &document)
{
    int size = 0;
    const char *raw = toRawData(document, &size);
    return QByteArray(raw, size);
}

} // namespace QBinaryJson

namespace QBinaryJsonPrivate {

static Q_CONSTEXPR Base emptyArray  = {
    { qle_uint(sizeof(Base)) },
    { 0 },
    { qle_uint(0) }
};

static Q_CONSTEXPR Base emptyObject = {
    { qle_uint(sizeof(Base)) },
    { qToLittleEndian(1U) },
    { qle_uint(0) }
};

void MutableData::compact()
{
    static_assert(sizeof(Value) == sizeof(offset));

    Base *base = header->root();
    int reserve = 0;
    if (base->isObject()) {
        auto *o = static_cast<Object *>(base);
        for (uint i = 0; i < o->length(); ++i)
            reserve += o->entryAt(i)->usedStorage(o);
    } else {
        auto *a = static_cast<Array *>(base);
        for (uint i = 0; i < a->length(); ++i)
            reserve += a->at(i)->usedStorage(a);
    }

    uint size = sizeof(Base) + reserve + base->length() * sizeof(offset);
    uint alloc = sizeof(Header) + size;
    auto *h = reinterpret_cast<Header *>(malloc(alloc));
    Q_CHECK_PTR(h);
    h->tag = QJsonDocument::BinaryFormatTag;
    h->version = 1;
    Base *b = new (h->root()) Base{};
    b->size = size;
    if (header->root()->isObject())
        b->setIsObject();
    else
        b->setIsArray();
    b->setLength(base->length());
    b->tableOffset = reserve + sizeof(Array);

    uint offset = sizeof(Base);
    if (b->isObject()) {
        const auto *o = static_cast<const Object *>(base);
        auto *no = static_cast<Object *>(b);

        for (uint i = 0; i < o->length(); ++i) {
            no->table()[i] = offset;

            const Entry *e = o->entryAt(i);
            Entry *ne = no->entryAt(i);
            uint s = e->size();
            memcpy(ne, e, s);
            offset += s;
            uint dataSize = e->value.usedStorage(o);
            if (dataSize) {
                memcpy(reinterpret_cast<char *>(no) + offset, e->value.data(o), dataSize);
                ne->value.setValue(offset);
                offset += dataSize;
            }
        }
    } else {
        const auto *a = static_cast<const Array *>(base);
        auto *na = static_cast<Array *>(b);

        for (uint i = 0; i < a->length(); ++i) {
            const Value *v = a->at(i);
            Value *nv = na->at(i);
            *nv = *v;
            uint dataSize = v->usedStorage(a);
            if (dataSize) {
                memcpy(reinterpret_cast<char *>(na) + offset, v->data(a), dataSize);
                nv->setValue(offset);
                offset += dataSize;
            }
        }
    }
    Q_ASSERT(offset == uint(b->tableOffset));

    free(header);
    header = h;
    this->alloc = alloc;
    compactionCounter = 0;
}

bool ConstData::isValid() const
{
    if (header->tag != QJsonDocument::BinaryFormatTag || header->version != 1U)
        return false;

    const Base *root = header->root();
    const uint maxSize = alloc - sizeof(Header);
    return root->isObject()
            ? static_cast<const Object *>(root)->isValid(maxSize)
            : static_cast<const Array *>(root)->isValid(maxSize);
}

QJsonDocument ConstData::toJsonDocument() const
{
    const Base *root = header->root();
    return root->isObject()
            ? QJsonDocument(static_cast<const Object *>(root)->toJsonObject())
            : QJsonDocument(static_cast<const Array *>(root)->toJsonArray());
}

uint Base::reserveSpace(uint dataSize, uint posInTable, uint numItems, bool replace)
{
    Q_ASSERT(posInTable <= length());
    if (size + dataSize >= Value::MaxSize) {
        qWarning("QJson: Document too large to store in data structure %d %d %d",
                 uint(size), dataSize, Value::MaxSize);
        return 0;
    }

    offset off = tableOffset;
    // move table to new position
    if (replace) {
        memmove(reinterpret_cast<char *>(table()) + dataSize, table(), length() * sizeof(offset));
    } else {
        memmove(reinterpret_cast<char *>(table() + posInTable + numItems) + dataSize,
                table() + posInTable, (length() - posInTable) * sizeof(offset));
        memmove(reinterpret_cast<char *>(table()) + dataSize, table(), posInTable * sizeof(offset));
    }
    tableOffset += dataSize;
    for (uint i = 0; i < numItems; ++i)
        table()[posInTable + i] = off;
    size += dataSize;
    if (!replace) {
        setLength(length() + numItems);
        size += numItems * sizeof(offset);
    }
    return off;
}

uint Object::indexOf(QStringView key, bool *exists) const
{
    uint min = 0;
    uint n = length();
    while (n > 0) {
        uint half = n >> 1;
        uint middle = min + half;
        if (*entryAt(middle) >= key) {
            n = half;
        } else {
            min = middle + 1;
            n -= half + 1;
        }
    }
    if (min < length() && *entryAt(min) == key) {
        *exists = true;
        return min;
    }
    *exists = false;
    return min;
}

QJsonObject Object::toJsonObject() const
{
    QJsonObject object;
    for (uint i = 0; i < length(); ++i) {
        const Entry *e = entryAt(i);
        object.insert(e->key(), e->value.toJsonValue(this));
    }
    return object;
}

bool Object::isValid(uint maxSize) const
{
    if (size > maxSize || tableOffset + length() * sizeof(offset) > size)
        return false;

    QString lastKey;
    for (uint i = 0; i < length(); ++i) {
        if (table()[i] + sizeof(Entry) >= tableOffset)
            return false;
        const Entry *e = entryAt(i);
        if (!e->isValid(tableOffset - table()[i]))
            return false;
        const QString key = e->key();
        if (key < lastKey)
            return false;
        if (!e->value.isValid(this))
            return false;
        lastKey = key;
    }
    return true;
}

QJsonArray Array::toJsonArray() const
{
    QJsonArray array;
    const offset *values = table();
    for (uint i = 0; i < length(); ++i)
        array.append(reinterpret_cast<const Value *>(values + i)->toJsonValue(this));
    return array;
}

bool Array::isValid(uint maxSize) const
{
    if (size > maxSize || tableOffset + length() * sizeof(offset) > size)
        return false;

    const offset *values = table();
    for (uint i = 0; i < length(); ++i) {
        if (!reinterpret_cast<const Value *>(values + i)->isValid(this))
            return false;
    }
    return true;
}

uint Value::usedStorage(const Base *b) const
{
    uint s = 0;
    switch (type()) {
    case QJsonValue::Double:
        if (!isLatinOrIntValue())
            s = sizeof(double);
        break;
    case QJsonValue::String: {
        const char *d = data(b);
        s = isLatinOrIntValue()
                ? (sizeof(ushort)
                   + qFromLittleEndian(*reinterpret_cast<const ushort *>(d)))
                : (sizeof(int)
                   + sizeof(ushort) * qFromLittleEndian(*reinterpret_cast<const int *>(d)));
        break;
    }
    case QJsonValue::Array:
    case QJsonValue::Object:
        s = base(b)->size;
        break;
    case QJsonValue::Null:
    case QJsonValue::Bool:
    default:
        break;
    }
    return alignedSize(s);
}

QJsonValue Value::toJsonValue(const Base *b) const
{
    switch (type()) {
    case QJsonValue::Null:
        return QJsonValue(QJsonValue::Null);
    case QJsonValue::Bool:
        return QJsonValue(toBoolean());
    case QJsonValue::Double:
        return QJsonValue(toDouble(b));
    case QJsonValue::String:
        return QJsonValue(toString(b));
    case QJsonValue::Array:
        return static_cast<const Array *>(base(b))->toJsonArray();
    case QJsonValue::Object:
        return static_cast<const Object *>(base(b))->toJsonObject();
    case QJsonValue::Undefined:
        return QJsonValue(QJsonValue::Undefined);
    }
    Q_UNREACHABLE_RETURN(QJsonValue(QJsonValue::Undefined));
}

inline bool isValidValueOffset(uint offset, uint tableOffset)
{
    return offset >= sizeof(Base)
        && offset + sizeof(uint) <= tableOffset;
}

bool Value::isValid(const Base *b) const
{
    switch (type()) {
    case QJsonValue::Null:
    case QJsonValue::Bool:
        return true;
    case QJsonValue::Double:
        return isLatinOrIntValue() || isValidValueOffset(value(), b->tableOffset);
    case QJsonValue::String:
        if (!isValidValueOffset(value(), b->tableOffset))
            return false;
        if (isLatinOrIntValue())
            return asLatin1String(b).isValid(b->tableOffset - value());
        return asString(b).isValid(b->tableOffset - value());
    case QJsonValue::Array:
        return isValidValueOffset(value(), b->tableOffset)
                && static_cast<const Array *>(base(b))->isValid(b->tableOffset - value());
    case QJsonValue::Object:
        return isValidValueOffset(value(), b->tableOffset)
                && static_cast<const Object *>(base(b))->isValid(b->tableOffset - value());
    default:
        return false;
    }
}

uint Value::requiredStorage(const QBinaryJsonValue &v, bool *compressed)
{
    *compressed = false;
    switch (v.type()) {
    case QJsonValue::Double:
        if (QBinaryJsonPrivate::compressedNumber(v.toDouble()) != INT_MAX) {
            *compressed = true;
            return 0;
        }
        return sizeof(double);
    case QJsonValue::String: {
        QString s = v.toString();
        *compressed = QBinaryJsonPrivate::useCompressed(s);
        return QBinaryJsonPrivate::qStringSize(s, *compressed);
    }
    case QJsonValue::Array:
    case QJsonValue::Object:
        return v.base ? uint(v.base->size) : sizeof(QBinaryJsonPrivate::Base);
    case QJsonValue::Undefined:
    case QJsonValue::Null:
    case QJsonValue::Bool:
        break;
    }
    return 0;
}

uint Value::valueToStore(const QBinaryJsonValue &v, uint offset)
{
    switch (v.type()) {
    case QJsonValue::Undefined:
    case QJsonValue::Null:
        break;
    case QJsonValue::Bool:
        return v.toBool();
    case QJsonValue::Double: {
        int c = QBinaryJsonPrivate::compressedNumber(v.toDouble());
        if (c != INT_MAX)
            return c;
    }
        Q_FALLTHROUGH();
    case QJsonValue::String:
    case QJsonValue::Array:
    case QJsonValue::Object:
        return offset;
    }
    return 0;
}

void Value::copyData(const QBinaryJsonValue &v, char *dest, bool compressed)
{
    switch (v.type()) {
    case QJsonValue::Double:
        if (!compressed)
            qToLittleEndian(v.toDouble(), dest);
        break;
    case QJsonValue::String: {
        const QString str = v.toString();
        QBinaryJsonPrivate::copyString(dest, str, compressed);
        break;
    }
    case QJsonValue::Array:
    case QJsonValue::Object: {
        const QBinaryJsonPrivate::Base *b = v.base;
        if (!b)
            b = (v.type() == QJsonValue::Array ? &emptyArray : &emptyObject);
        memcpy(dest, b, b->size);
        break;
    }
    default:
        break;
    }
}

} // namespace QBinaryJsonPrivate

QT_END_NAMESPACE
