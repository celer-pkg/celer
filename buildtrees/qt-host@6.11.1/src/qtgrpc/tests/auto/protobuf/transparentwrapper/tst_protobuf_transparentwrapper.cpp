// Copyright (C) 2024 The Qt Company Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtProtobuf/qtprotobuftypes.h>

#include <QtTest/QtTest>

using namespace QtProtobuf;

class ProtobufTransparentWrapperTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void specialFunctions() const;
    void hashValueDoesNotDependOnTag() const;
};

void ProtobufTransparentWrapperTest::specialFunctions() const
{
    TransparentWrapper<int32_t, struct int_tag> v(42);

    QCOMPARE_EQ(qHash(v), qHash(v.t));

    QCOMPARE_EQ(qbswap(v), qbswap(v.t));
}

void ProtobufTransparentWrapperTest::hashValueDoesNotDependOnTag() const
{
    TransparentWrapper<int32_t, struct int_tag> v1(42);
    TransparentWrapper<int32_t, struct other_tag> v2(42);

    QCOMPARE_EQ(qHash(v1), qHash(v2));
}

QTEST_MAIN(ProtobufTransparentWrapperTest)
#include "tst_protobuf_transparentwrapper.moc"
