// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

const char errorName[] = "org.qtproject.tst_QDBusContext.Error";
const char errorMsg[] = "A generic error";

class TestObject: public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.qtproject.tst_QDBusContext.TestObject")
    Q_PROPERTY(bool verifyDBus READ verifyDBus WRITE setVerifyDBus)
public:
    inline TestObject(QObject *parent) : QObject(parent) { }
    bool verifyDBus() const;
    void setVerifyDBus(bool);
public Q_SLOTS:
    void generateError();
    bool someSlot() const;
};

class tst_QDBusContext: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void sendErrorReply();
    void calledFromDBusPropertyRead();
    void calledFromDBusPropertyWrite();
    void calledFromDBusSlot();
    void notCalledFromDBus();
};

void TestObject::generateError()
{
    Q_ASSERT(calledFromDBus());
    sendErrorReply(errorName, errorMsg);
}

bool TestObject::verifyDBus() const
{
    return calledFromDBus();
}

void TestObject::setVerifyDBus(bool)
{
    if (!calledFromDBus()) {
        sendErrorReply(QDBusError::InternalError, "calledFromDBus() was false");
    }
}

bool TestObject::someSlot() const
{
    return calledFromDBus();
}

void tst_QDBusContext::initTestCase()
{
    TestObject *obj = new TestObject(this);
    QVERIFY(QDBusConnection::sessionBus().isConnected());
    QVERIFY(QDBusConnection::sessionBus().registerObject("/TestObject", obj,
                                                          QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllProperties));
}

void tst_QDBusContext::sendErrorReply()
{
    QDBusInterface iface(QDBusConnection::sessionBus().baseService(), "/TestObject");
    QVERIFY(iface.isValid());

    QDBusReply<void> reply = iface.call("generateError");
    QVERIFY(!reply.isValid());

    const QDBusError &error = reply.error();
    QCOMPARE(error.name(), QString::fromLatin1(errorName));
    QCOMPARE(error.message(), QString::fromLatin1(errorMsg));
}

void tst_QDBusContext::calledFromDBusPropertyRead()
{
    QDBusInterface iface(QDBusConnection::sessionBus().baseService(), "/TestObject", "org.freedesktop.DBus.Properties");
    QVERIFY(iface.isValid());

    QDBusReply<QVariant> reply = iface.call("Get", "org.qtproject.tst_QDBusContext.TestObject", "verifyDBus");
    QVERIFY(reply.isValid());
    QVERIFY(reply.value().toBool());
}

void tst_QDBusContext::calledFromDBusPropertyWrite()
{
    QDBusInterface iface(QDBusConnection::sessionBus().baseService(), "/TestObject", "org.freedesktop.DBus.Properties");
    QVERIFY(iface.isValid());

    QDBusReply<void> reply = iface.callWithArgumentList(QDBus::Block, "Set", {"org.qtproject.tst_QDBusContext.TestObject", "verifyDBus", QVariant::fromValue(QDBusVariant(true))});
    QVERIFY(reply.isValid());
}

void tst_QDBusContext::calledFromDBusSlot()
{
    QDBusInterface iface(QDBusConnection::sessionBus().baseService(), "/TestObject", "org.qtproject.tst_QDBusContext.TestObject");
    QVERIFY(iface.isValid());

    QDBusReply<bool> reply = iface.call("someSlot");
    QVERIFY(reply.isValid());
    QVERIFY(reply.value());
}

void tst_QDBusContext::notCalledFromDBus()
{
    TestObject obj(nullptr);
    QVERIFY(!obj.verifyDBus());
    QVERIFY(!obj.someSlot());
}

QTEST_MAIN(tst_QDBusContext)

#include "tst_qdbuscontext.moc"
