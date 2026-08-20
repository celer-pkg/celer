// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QMetaProperty>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <QtProtobuf/qprotobufobject.h>
#include <QtProtobuf/qprotobufoneof.h>

class TestMetaType : public QProtobufMessage
{
    Q_PROTOBUF_OBJECT
public:
    TestMetaType() : QProtobufMessage(&staticMetaObject, &TestMetaType::staticPropertyOrdering) { }
    ~TestMetaType() { ++m_destroyed; }
    explicit TestMetaType(int value)
        : QProtobufMessage(&TestMetaType::staticMetaObject, &TestMetaType::staticPropertyOrdering),
          m_value(value)
    {
    }
    TestMetaType(const TestMetaType &other)
        : QProtobufMessage(&TestMetaType::staticMetaObject, &TestMetaType::staticPropertyOrdering),
          m_value(other.m_value)
    {
        ++m_copied;
    }

    TestMetaType &operator =(const TestMetaType &other)
    {
        if (this == &other)
            return *this;

        m_value = other.m_value;
        ++m_copied;
        return *this;
    }

    void setIntValue(int value) { m_value = value; }

    int intValue() const { return m_value; }

    static int m_copied;
    static int m_destroyed;

    bool operator==(const TestMetaType &other) const { return m_value == other.m_value; }

private:
    int m_value = 0;
};
int TestMetaType::m_copied = 0;
int TestMetaType::m_destroyed = 0;
const QtProtobufPrivate::QProtobufPropertyOrdering TestMetaType::staticPropertyOrdering{};

class QProtobufOneofTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void valueReplacement();
    void valueImplicitSharing();
    void moveValue();
    void equalOperator();

    void init()
    {
        TestMetaType::m_copied = 0;
        TestMetaType::m_destroyed = 0;
    }
};

Q_DECLARE_METATYPE(TestMetaType)

void QProtobufOneofTest::valueReplacement()
{
    {
        QtProtobufPrivate::QProtobufOneof optional;
        optional.setValue(TestMetaType{ 5 }, 1);
        QCOMPARE(optional.message<TestMetaType>()->intValue(), 5);
        {
            optional.setValue(TestMetaType{ 10 }, 1);
            QCOMPARE(TestMetaType::m_destroyed, 3);
        }

        QCOMPARE(optional.message<TestMetaType>()->intValue(), 10);
        optional.message<TestMetaType>()->setIntValue(15);
        QCOMPARE(optional.message<TestMetaType>()->intValue(), 15);
        QCOMPARE(TestMetaType::m_destroyed, 3);
    }
    QCOMPARE(TestMetaType::m_destroyed, 4);
    QCOMPARE(TestMetaType::m_copied, 2);
}

void QProtobufOneofTest::valueImplicitSharing()
{
    {
        QtProtobufPrivate::QProtobufOneof optional;
        optional.setValue(TestMetaType{ 5 }, 1);
        // TestMetaType is destroyed after exiting the constructor stack
        QCOMPARE(TestMetaType::m_destroyed, 1);
        // setValue makes a copy of the value
        QCOMPARE(TestMetaType::m_copied, 1);
        QCOMPARE(optional.message<TestMetaType>()->intValue(), 5);
        {
            QtProtobufPrivate::QProtobufOneof optional2(optional);
            // Copying of the optional should be cheap so the stored pointer should not be copied
            // until the value is accessed.
            QCOMPARE(TestMetaType::m_copied, 1);
            QCOMPARE(TestMetaType::m_destroyed, 1);
            QCOMPARE(optional2.message<TestMetaType>()->intValue(), 5);

            optional.setValue(TestMetaType{ 15 }, 1);
            QCOMPARE(TestMetaType::m_copied, 3);
            QCOMPARE(TestMetaType::m_destroyed, 3);
            QCOMPARE(optional.message<TestMetaType>()->intValue(), 15);
            QCOMPARE(optional2.message<TestMetaType>()->intValue(), 5);
        }
        QCOMPARE(TestMetaType::m_destroyed, 4);
    }
    // First copy when setting value of 'optional'.
    // Second copy when accessing the value from the 'optional2', since we access
    // the detached value.
    // Third copy when rewriting it with new one.
    QCOMPARE(TestMetaType::m_destroyed, 5);
    QCOMPARE(TestMetaType::m_copied, 3);

    // Check if changing the value inside a copy doesn't affect the value of original.
    QtProtobufPrivate::QProtobufOneof optional3;
    optional3.setValue(TestMetaType{ 15 }, 1);

    QtProtobufPrivate::QProtobufOneof optional4(optional3);
    optional4.message<TestMetaType>()->setIntValue(5);

    QCOMPARE(optional3.message<TestMetaType>()->intValue(), 15);
    QCOMPARE(optional4.message<TestMetaType>()->intValue(), 5);
}

void QProtobufOneofTest::equalOperator()
{
    QtProtobufPrivate::QProtobufOneof optional1;
    optional1.setValue(TestMetaType{ 5 }, 1);

    QtProtobufPrivate::QProtobufOneof optional2;
    QVERIFY(optional1 != optional2);

    optional2.setValue(TestMetaType{}, 1);
    QVERIFY(optional1 != optional2);

    optional1.setValue(TestMetaType{}, 1);
    QVERIFY(optional1 == optional2);

    optional1.setValue(TestMetaType{ 5 }, 1);
    optional2.setValue(TestMetaType{ 5 }, 1);
    QVERIFY(optional1 == optional2);

    optional2.setValue(TestMetaType{ 5 }, 0);
    QVERIFY(optional1 != optional2);

    optional2.setValue(TestMetaType{ 3 }, 1);
    QVERIFY(optional1 != optional2);

    optional2 = optional1;
    QVERIFY(optional1 == optional2);

    optional2.setValue(10, 1);
    QVERIFY(optional1 != optional2);

    optional1.setValue(10, 2);
    QVERIFY(optional1 != optional2);

    optional1.setValue(10, 1);
    QVERIFY(optional1 == optional2);

    optional1.setValue(11, 1);
    QVERIFY(optional1 != optional2);

    optional2 = optional1;
    QVERIFY(optional1 == optional2);

    optional1.setValue(TestMetaType{ 5 }, 1);
    QVERIFY(optional1 != optional2);
}

void QProtobufOneofTest::moveValue()
{
    {
        QtProtobufPrivate::QProtobufOneof optional;
        optional.setValue(TestMetaType{ 5 }, 1);

        QVERIFY(optional.holdsField(1));
        QCOMPARE(optional.message<TestMetaType>()->intValue(), 5);
        {
            QtProtobufPrivate::QProtobufOneof optional2(std::move(optional));

            QVERIFY(optional2.holdsField(1));
            QCOMPARE(optional2.message<TestMetaType>()->intValue(), 5);

            optional = std::move(optional2);
            QVERIFY(optional.holdsField(1));
            QCOMPARE(optional.message<TestMetaType>()->intValue(), 5);
        }
        QCOMPARE(TestMetaType::m_destroyed, 1);

        QCOMPARE(optional.message<TestMetaType>()->intValue(), 5);
        {
            QtProtobufPrivate::QProtobufOneof optional3(std::move(optional));
        }
        QCOMPARE(TestMetaType::m_destroyed, 2);
    }
    QCOMPARE(TestMetaType::m_destroyed, 2);
    QCOMPARE(TestMetaType::m_copied, 1);
}
QTEST_MAIN(QProtobufOneofTest)
#include "tst_qprotobufoneof.moc"
