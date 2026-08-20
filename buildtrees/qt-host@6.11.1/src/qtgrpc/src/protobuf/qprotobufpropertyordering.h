// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFPROPERTYORDERING_H
#define QPROTOBUFPROPERTYORDERING_H

#if 0
#  pragma qt_sync_skip_header_check
#  pragma qt_sync_stop_processing
#endif

#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qflags.h>
#include <QtCore/qtclasshelpermacros.h>
#include <QtCore/qutf8stringview.h>
#include <QtCore/qxptype_traits.h>

QT_BEGIN_NAMESPACE

namespace QtProtobufPrivate {

enum class FieldFlag : uint {
    NoFlags = 0x0,
    NonPacked = 0x1,
    Oneof = 0x02,
    Optional = 0x04,
    ExplicitPresence = 0x08,
    Message = 0x10,
    Enum = 0x20,
    Repeated = 0x40,
    Map = 0x80,
};
Q_DECLARE_FLAGS(FieldFlags, FieldFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(FieldFlags)

class QProtobufPropertyOrderingBuilder;
struct QProtobufPropertyOrdering
{
    const struct Data
    {
        uint version;
        uint numFields;
        uint fieldNumberOffset;
        uint propertyIndexOffset;
        uint flagsOffset;
        uint fullPackageNameSize;
    } *data;

    Q_PROTOBUF_EXPORT QUtf8StringView messageFullName() const;
    Q_PROTOBUF_EXPORT QUtf8StringView jsonName(int index) const;
    Q_PROTOBUF_EXPORT int fieldNumber(int index) const;
    Q_PROTOBUF_EXPORT int propertyIndex(int index) const;
    Q_PROTOBUF_EXPORT FieldFlags fieldFlags(int index) const;
    Q_PROTOBUF_EXPORT int indexOfFieldNumber(int fieldNumber) const;
    int fieldCount() const { return int(data->numFields); }

private:
    friend class QProtobufPropertyOrderingBuilder;
    QT_DEFINE_TAG_STRUCT(NonConstTag);
    uint *uint_data(NonConstTag) const;
    char *char_data(NonConstTag) const;
    const uint *uint_data() const;
    const char *char_data() const;
    const uint &uint_dataForIndex(int index, uint offset) const;
};

// Convenience structure to hold a reference to a single field information
struct QProtobufFieldInfo
{
    QProtobufFieldInfo(QProtobufPropertyOrdering ord, int ind)
        : ordering(ord), index(ind)
    {
        Q_ASSERT(index >= 0);
    }

    QUtf8StringView jsonName() const { return ordering.jsonName(index); }
    int fieldNumber() const { return ordering.fieldNumber(index); }
    int propertyIndex() const { return ordering.propertyIndex(index); }
    FieldFlags fieldFlags() const { return ordering.fieldFlags(index); }

private:
    const QProtobufPropertyOrdering ordering;
    const int index;
};

} // namespace QtProtobufPrivate

QT_END_NAMESPACE

#endif // QPROTOBUFPROPERTYORDERING_H
