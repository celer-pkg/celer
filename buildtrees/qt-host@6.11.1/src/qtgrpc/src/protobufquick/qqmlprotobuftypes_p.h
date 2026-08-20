// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQMLPROTOBUFTYPES_H
#define QQMLPROTOBUFTYPES_H

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

#include <QtProtobuf/qtprotobuftypes.h>

#include <QtQmlIntegration/qqmlintegration.h>

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

struct QQmlProtobufInt32Foreign
{
    Q_GADGET
    QML_FOREIGN(QtProtobuf::int32)
    QML_USING(int32_t)
};

struct QQmlProtobufInt64Foreign
{
    Q_GADGET
    QML_FOREIGN(QtProtobuf::int64)
    QML_USING(int64_t)
};

struct QQmlProtobufFixed32Foreign
{
    Q_GADGET
    QML_FOREIGN(QtProtobuf::fixed32)
    QML_USING(uint32_t)
};

struct QQmlProtobufFixed64Foreign
{
    Q_GADGET
    QML_FOREIGN(QtProtobuf::fixed64)
    QML_USING(uint64_t)
};

struct QQmlProtobufSFixed32Foreign
{
    Q_GADGET
    QML_FOREIGN(QtProtobuf::sfixed32)
    QML_USING(int32_t)
};

struct QQmlProtobufSFixed64Foreign
{
    Q_GADGET
    QML_FOREIGN(QtProtobuf::sfixed64)
    QML_USING(int64_t)
};

struct QQmlProtobufUInt32Foreign
{
    Q_GADGET
    QML_FOREIGN(QtProtobuf::uint32)
    QML_USING(uint32_t)
};

struct QQmlProtobufUInt64Foreign
{
    Q_GADGET
    QML_FOREIGN(QtProtobuf::uint64)
    QML_USING(uint64_t)
};

struct QQmlProtobufSInt32Foreign
{
    Q_GADGET
    QML_FOREIGN(QtProtobuf::sint32)
    QML_USING(int32_t)
};

struct QQmlProtobufSInt64Foreign
{
    Q_GADGET
    QML_FOREIGN(QtProtobuf::sint64)
    QML_USING(int64_t)
};

struct QQmlProtobufBoolListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::boolList)
    QML_SEQUENTIAL_CONTAINER(bool)
};

struct QQmlProtobufFloatListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::floatList)
    QML_SEQUENTIAL_CONTAINER(float)
};

struct QQmlProtobufDoubleListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::doubleList)
    QML_SEQUENTIAL_CONTAINER(double)
};

struct QQmlProtobufInt32ListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::int32List)
    QML_SEQUENTIAL_CONTAINER(QtProtobuf::int32)
};

struct QQmlProtobufInt64ListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::int64List)
    QML_SEQUENTIAL_CONTAINER(QtProtobuf::int64)
};

struct QQmlProtobufUInt32ListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::uint32List)
    QML_SEQUENTIAL_CONTAINER(QtProtobuf::uint32)
};

struct QQmlProtobufUInt64ListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::uint64List)
    QML_SEQUENTIAL_CONTAINER(QtProtobuf::uint64)
};

struct QQmlProtobufSInt32ListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::sint32List)
    QML_SEQUENTIAL_CONTAINER(QtProtobuf::sint32)
};

struct QQmlProtobufSInt64ListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::sint64List)
    QML_SEQUENTIAL_CONTAINER(QtProtobuf::sint64)
};

struct QQmlProtobufFixed32ListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::fixed32List)
    QML_SEQUENTIAL_CONTAINER(QtProtobuf::fixed32)
};

struct QQmlProtobufFixed64ListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::fixed64List)
    QML_SEQUENTIAL_CONTAINER(QtProtobuf::fixed64)
};

struct QQmlProtobufSFixed32ListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::sfixed32List)
    QML_SEQUENTIAL_CONTAINER(QtProtobuf::sfixed32)
};

struct QQmlProtobufSFixed64ListForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QtProtobuf::sfixed64List)
    QML_SEQUENTIAL_CONTAINER(QtProtobuf::sfixed64)
};

QT_END_NAMESPACE

#endif // QQMLPROTOBUFTYPES_H
