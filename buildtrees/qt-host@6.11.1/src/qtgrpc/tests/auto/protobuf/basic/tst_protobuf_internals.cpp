// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "basicmessages.qpb.h"
#include "non_final/basicmessages.qpb.h"

#include <QTest>

using namespace qtprotobufnamespace::tests;

class QtProtobufInternalsTest : public QObject
{
    Q_OBJECT
public:
    QtProtobufInternalsTest() = default;

    template <typename Str, typename StrClone, typename Int>
    void qprotobufmessageCastTest();

private Q_SLOTS:
    void nullPointerMessageTest();
    void nullPointerGetterMessageTest();
    void qprotobufmessageCastFinalMsgTest()
    {
        qprotobufmessageCastTest<qtprotobufnamespace::tests::SimpleStringMessage,
                                 qtprotobufnamespace::tests::SimpleStringMessageClone,
                                 qtprotobufnamespace::tests::SimpleIntMessage>();
    }
    void qprotobufmessageCastNonFinalMsgTest()
    {
        qprotobufmessageCastTest<nonfinal::qtprotobufnamespace::tests::SimpleStringMessage,
                                 nonfinal::qtprotobufnamespace::tests::SimpleStringMessageClone,
                                 nonfinal::qtprotobufnamespace::tests::SimpleIntMessage>();
    }
};

void QtProtobufInternalsTest::nullPointerMessageTest()
{
    SimpleStringMessage stringMsg;
    stringMsg.setTestFieldString({ "not null" });
    ComplexMessage msg;
    msg.setTestFieldInt(0);
    msg.setTestComplexField(stringMsg);

    msg.setProperty("testComplexField_p",
                    QVariant::fromValue(static_cast<SimpleStringMessage *>(nullptr)));
    QVERIFY(msg.testComplexField().testFieldString().isEmpty());
    QVERIFY(msg.property("testComplexField_p").value<SimpleStringMessage *>() != nullptr);
}

void QtProtobufInternalsTest::nullPointerGetterMessageTest()
{
    ComplexMessage msg;
    QVERIFY(!msg.hasTestComplexField());
    QVERIFY(msg.property("testComplexField_p").value<SimpleStringMessage *>() != nullptr);
    msg.setProperty("testComplexField_p",
                    QVariant::fromValue(static_cast<SimpleStringMessage *>(nullptr)));
    QVERIFY(msg.testComplexField().testFieldString().isEmpty());
    QVERIFY(msg.property("testComplexField_p").value<SimpleStringMessage *>() != nullptr);
}

class DerivedMessage : public nonfinal::qtprotobufnamespace::tests::SimpleIntMessage
{
    Q_PROTOBUF_OBJECT
public:
    QByteArray data;
};

template <typename Str, typename StrClone, typename Int>
void QtProtobufInternalsTest::qprotobufmessageCastTest()
{
    Str original;
    QProtobufMessage *base = &original;

    // Case 1: Successful downcast
    auto *castToSelf = qprotobufmessage_cast<Str *>(base);
    QCOMPARE_EQ(castToSelf, &original);
    // Case 2: Failed downcast to an unrelated type
    auto *castToOther = qprotobufmessage_cast<Int *>(base);
    QCOMPARE_EQ(castToOther, nullptr);
    auto *castToOtherClone = qprotobufmessage_cast<StrClone *>(base);
    QCOMPARE_EQ(castToOtherClone, nullptr);

    // Case 3: Casting a null pointer
    QProtobufMessage *nullBase = nullptr;
    auto *castFromNull = qprotobufmessage_cast<Str *>(nullBase);
    QCOMPARE_EQ(castFromNull, nullptr);

    // Case 4: Const-correct casting
    const QProtobufMessage *constBase = base;
    const auto *constCast = qprotobufmessage_cast<const Str *>(constBase);
    QCOMPARE_EQ(constCast, &original);
    // Case 5: Failed const-correct casting
    const auto *failedConstCast = qprotobufmessage_cast<const Int *>(constBase);
    QCOMPARE_EQ(failedConstCast, nullptr);

    // Case 5: Derived msg casts
    if constexpr (!std::is_final_v<Int>) {
        DerivedMessage derived;
        base = &derived;

        // We cannot cast to user-derived types because the QProtobufMessage ctor
        // stores the parent's metaObject (SimpleIntMessage), not DerivedMessage's.
        // There is currently no way to provide this for user-derived types.
        QEXPECT_FAIL("", "User-derived classes not detectable", Continue);
        QCOMPARE_EQ(qprotobufmessage_cast<DerivedMessage *>(base), &derived);

        QCOMPARE_EQ(qprotobufmessage_cast<Int *>(base), &derived);
        QCOMPARE_EQ(qprotobufmessage_cast<Str *>(base), nullptr);

        Int baseMsg;
        base = &baseMsg;
        // Correctly rejects invalid upcast
        QCOMPARE_EQ(qprotobufmessage_cast<DerivedMessage *>(base), nullptr);
    }
}

QTEST_MAIN(QtProtobufInternalsTest)
#include "tst_protobuf_internals.moc"
