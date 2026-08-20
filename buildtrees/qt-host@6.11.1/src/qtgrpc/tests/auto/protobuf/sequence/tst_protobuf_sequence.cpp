// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "sequence.qpb.h"

#include <QMetaProperty>
#include <QSignalSpy>
#include <QTest>

#include <qtprotobuftestscommon.h>

class QtProtobufGenerationSequenceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void sequenceTest();
    void cyclingTest();
    void mapRepeatedFieldSequenceTest();
};

using namespace qtprotobufnamespace::tests;

void QtProtobufGenerationSequenceTest::sequenceTest()
{
    qProtobufAssertMessagePropertyRegistered<sequence::TestMessageSequence, sequence::TestMessageSequence2*>(1, "qtprotobufnamespace::tests::sequence::TestMessageSequence2*", "testField_p");
    qProtobufAssertMessagePropertyRegistered<sequence::TestMessageSequence2, bool>(1, "bool", "testField");
}

void QtProtobufGenerationSequenceTest::cyclingTest()
{
    qProtobufAssertMessagePropertyRegistered<sequence::CyclingFirstDependency, sequence::CyclingSecondDependency*>(1, "qtprotobufnamespace::tests::sequence::CyclingSecondDependency*", "testField_p");
    qProtobufAssertMessagePropertyRegistered<sequence::CyclingSecondDependency, sequence::CyclingFirstDependency*>(1, "qtprotobufnamespace::tests::sequence::CyclingFirstDependency*", "testField_p");
    sequence::CyclingFirstDependency test;
    sequence::CyclingSecondDependency test2;
    test.setTestField(test2);
    test2.setTestField(test);
}

void QtProtobufGenerationSequenceTest::mapRepeatedFieldSequenceTest()
{
    qProtobufAssertMessagePropertyRegistered<sequence::RepeatedFieldSequence, QList<sequence::RepeatedFieldSequence2>>(1, "QList<qtprotobufnamespace::tests::sequence::RepeatedFieldSequence2>", "testFieldData");
    qProtobufAssertMessagePropertyRegistered<sequence::MapFieldSequence, sequence::MapFieldSequence::TestFieldEntry>(1, "qtprotobufnamespace::tests::sequence::MapFieldSequence::TestFieldEntry", "testField");
}

QTEST_MAIN(QtProtobufGenerationSequenceTest)
#include "tst_protobuf_sequence.moc"
