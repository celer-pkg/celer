// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "recursive.qpb.h"

#include <QMetaProperty>
#include <QSignalSpy>
#include <QTest>
#include <QDebug>

#include <qprotobufserializer.h>
#include <qtprotobuftestscommon.h>

class QtProtobufRecursiveTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() { m_serializer.reset(new QProtobufSerializer); }
    void serializationTest();
    void deserializationTest();

private:
    std::unique_ptr<QProtobufSerializer> m_serializer;
};

using namespace qtprotobufnamespace::tests;

void initRecursive(RecursiveMessage &parent, int &times)
{
    if (times == 0)
        return;
    --times;

    RecursiveMessage child;
    child.setTestFieldInt(times);
    initRecursive(child, times);
    parent.setTestFieldRecursive(child);
}

void QtProtobufRecursiveTest::serializationTest()
{
    int times = 10;
    RecursiveMessage msg;
    QCOMPARE(msg.serialize(m_serializer.get()).toHex(), "");
    initRecursive(msg, times);
    QCOMPARE(msg.serialize(m_serializer.get()).toHex(),
             "12240809"
             "12200808"
             "121c0807"
             "12180806"
             "12140805"
             "12100804"
             "120c0803"
             "12080802"
             "12040801"
             "1200");
}

void QtProtobufRecursiveTest::deserializationTest()
{
    RecursiveMessage msg;
    msg.deserialize(m_serializer.get(), QByteArray::fromHex("083712080836120412020835"));

    QCOMPARE(msg.testFieldInt(), 55);
    QCOMPARE(msg.testFieldRecursive().testFieldInt(), 54);
    QCOMPARE(msg.testFieldRecursive().testFieldRecursive().testFieldInt(), 0);
    QCOMPARE(msg.testFieldRecursive().testFieldRecursive().testFieldRecursive().testFieldInt(), 53);
}

QTEST_MAIN(QtProtobufRecursiveTest)
#include "tst_protobuf_recursive.moc"
