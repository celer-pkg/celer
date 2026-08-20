// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtProtobuf/qprotobuflazymessagepointer.h>
#include <QtProtobuf/qprotobufobject.h>

#include <QtTest/qtest.h>

// Unfortunately this test does not test the QML-ownership parts of the code
class tst_QProtobufLazyMessagePointer : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void ctor();
    void reset();
    void get();
};
// @todo: fix this test

class TestStruct : public QProtobufMessage
{
private:
    Q_PROTOBUF_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
public:
    TestStruct() : QProtobufMessage(&staticMetaObject, nullptr) { }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString m_name;
};

const QtProtobufPrivate::QProtobufPropertyOrdering TestStruct::staticPropertyOrdering = {};

void tst_QProtobufLazyMessagePointer::ctor()
{
    QtProtobufPrivate::QProtobufLazyMessagePointer<TestStruct> ptr;
    QVERIFY(!ptr);
    QtProtobufPrivate::QProtobufLazyMessagePointer<TestStruct> ptr2(nullptr);
    QVERIFY(!ptr2);
    TestStruct *obj = new TestStruct;
    QtProtobufPrivate::QProtobufLazyMessagePointer<TestStruct> ptr3(obj);
    QVERIFY(ptr3);
    QtProtobufPrivate::QProtobufLazyMessagePointer<TestStruct> ptr4 = std::move(ptr3);
    QVERIFY(ptr4);
}

void tst_QProtobufLazyMessagePointer::reset()
{
    TestStruct *obj = new TestStruct;
    QtProtobufPrivate::QProtobufLazyMessagePointer<TestStruct> ptr(obj);
    // @todo: messages are no longer a QObject, need to test this different
    // QPointer<QObject> objPtr = obj;
    ptr.reset();
    // QVERIFY(objPtr.isNull());
    ptr.reset();
    // QVERIFY(objPtr.isNull());
    obj = new TestStruct;
    ptr.reset(obj);
    QCOMPARE(ptr.get(), obj);
}

void tst_QProtobufLazyMessagePointer::get()
{
    TestStruct *obj = new TestStruct; // QProtobufLazyMessagePointer will deallocate
    obj->setProperty("name", "obj");
    QtProtobufPrivate::QProtobufLazyMessagePointer<TestStruct> ptr(obj);
    QCOMPARE(ptr.get(), obj);
    QCOMPARE(&(*ptr), obj);
    QCOMPARE(ptr->property("name"), obj->property("name"));
    obj->setProperty("name", "obj2");
    QCOMPARE(ptr->property("name"), obj->property("name"));
}

QTEST_APPLESS_MAIN(tst_QProtobufLazyMessagePointer)
#include "tst_qprotobuflazymessagepointer.moc"
