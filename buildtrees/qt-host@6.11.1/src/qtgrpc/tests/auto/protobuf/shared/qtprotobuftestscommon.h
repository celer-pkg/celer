// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTPROTOBUFTESTSCOMMON_H
#define QTPROTOBUFTESTSCOMMON_H

#include <QMetaType>
#include <QTest>

template<typename MessageType, typename PropertyType>
static void qProtobufAssertMessagePropertyRegistered(int fieldIndex, const char *propertyTypeName, const char *propertyName)
{
    // TODO: there should be(?) a mapping available: PropertyType -> propertyTypeName

    int index = MessageType::staticPropertyOrdering.indexOfFieldNumber(fieldIndex);
    const int propertyNumber = MessageType::staticPropertyOrdering.propertyIndex(index);
    // TODO Qt6: Property type name check is disable because metatype system changes in Qt6.
    // Q_PROPERTY returns non-aliased type for the aliases defined using the 'using' keyword.
    // QCOMPARE(QLatin1String(propertyTypeName), QLatin1String(MessageType::staticMetaObject.property(propertyNumber).typeName()));
    Q_UNUSED(propertyTypeName)
    QCOMPARE(QMetaType::fromType<PropertyType>(),
             MessageType::staticMetaObject.property(propertyNumber).metaType());
    QCOMPARE(QLatin1String(MessageType::staticMetaObject.property(propertyNumber).name()),
             QLatin1String(propertyName));
}

[[maybe_unused]]
static bool compareSerializedChunks(const QString &actual, const char *chunk1, const char *chunk2,
                                    const char *chunk3)
{
    return QLatin1StringView(chunk1) + QLatin1StringView(chunk2) + QLatin1StringView(chunk3)
            == actual
            || QLatin1StringView(chunk1) + QLatin1StringView(chunk3) + QLatin1StringView(chunk2)
            == actual
            || QLatin1StringView(chunk2) + QLatin1StringView(chunk1) + QLatin1StringView(chunk3)
            == actual
            || QLatin1StringView(chunk2) + QLatin1StringView(chunk3) + QLatin1StringView(chunk1)
            == actual
            || QLatin1StringView(chunk3) + QLatin1StringView(chunk2) + QLatin1StringView(chunk1)
            == actual
            || QLatin1StringView(chunk3) + QLatin1StringView(chunk1) + QLatin1StringView(chunk2)
            == actual;
}

#endif // QTPROTOBUFTESTSCOMMON_H
