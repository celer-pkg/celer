// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobuf/private/protobufscalarserializers_p.h>

QT_BEGIN_NAMESPACE

QByteArray ProtobufScalarSerializers::serializeVarintCommonImpl(quint64 value)
{
    if (value == 0)
        return { 1, char(0) };

    QByteArray result;
    while (value != 0) {
        // Put 7 bits to result buffer and mark as "not last" (0b10000000)
        result.append((value & 0b01111111) | 0b10000000);
        // Divide values to chunks of 7 bits and move to next chunk
        value >>= 7;
    }

    result.back() &= ~0b10000000;
    return result;
}

/*
    Gets length of a byte-array and prepends to it its serialized length value
    using the appropriate serialization algorithm

    Returns 'data' with its length prepended
*/
QByteArray ProtobufScalarSerializers::prependLengthDelimitedSize(const QByteArray &data)
{
    return serializeVarintCommonImpl(data.size()) + data;
}

std::optional<QByteArray>
ProtobufScalarSerializers::deserializeLengthDelimited(QProtobufSelfcheckIterator &it)
{
    if (it.bytesLeft() != 0) {
        if (auto opt = deserializeVarintCommon<QtProtobuf::uint64>(it); opt) {
            if (quint64 length = opt.value(); it.isValid() && quint64(it.bytesLeft()) >= length
                && length <= quint64(QByteArray::maxSize())) {
                QByteArray result(it.data(), qsizetype(length));
                it += length;
                return { std::move(result) };
            }
        }
    }
    return std::nullopt;
}

QT_END_NAMESPACE
