// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "duplicated_metatypes.qpb.h"

#include <QTest>
#include <qtprotobuftestscommon.h>

class QtProtobufDuplicatedMetatypesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void simpleTest();
};

using namespace qtprotobufnamespace::duplicated_metatypes;

void QtProtobufDuplicatedMetatypesTest::simpleTest()
{
    qProtobufAssertMessagePropertyRegistered<Message1, Message1::OptsEntry>(1, "qtprotobufnamespace::duplicated_metatypes::Message1::OptsEntry", "opts");
    qProtobufAssertMessagePropertyRegistered<Message2, Message2::OptsEntry>(1, "qtprotobufnamespace::duplicated_metatypes::Message2::OptsEntry", "opts");
    qProtobufAssertMessagePropertyRegistered<Message3, Message3::OptsEntry>(1, "qtprotobufnamespace::duplicated_metatypes::Message3::OptsEntry", "opts");
    qProtobufAssertMessagePropertyRegistered<Message4, Message4::OptsEntry>(1, "qtprotobufnamespace::duplicated_metatypes::Message4::OptsEntry", "opts");
    qProtobufAssertMessagePropertyRegistered<Message5, Message5::OptsEntry>(1, "qtprotobufnamespace::duplicated_metatypes::Message5::OptsEntry", "opts");
    qProtobufAssertMessagePropertyRegistered<Message6, Message6::OptsEntry>(1, "qtprotobufnamespace::duplicated_metatypes::Message6::OptsEntry", "opts");
    qProtobufAssertMessagePropertyRegistered<Message7, Message7::OptsEntry>(1, "qtprotobufnamespace::duplicated_metatypes::Message7::OptsEntry", "opts");
    qProtobufAssertMessagePropertyRegistered<Message8, Message8::OptsEntry>(1, "qtprotobufnamespace::duplicated_metatypes::Message8::OptsEntry", "opts");
}

#include "tst_protobuf_duplicatedmetatypes.moc"
QTEST_MAIN(QtProtobufDuplicatedMetatypesTest)
