// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <QtLocation/QPlaceUser>

QT_USE_NAMESPACE

class tst_QPlaceUser : public QObject
{
    Q_OBJECT

public:
    tst_QPlaceUser();

private Q_SLOTS:
    void constructorTest();
    void nameTest();
    void userIdTest();
    void operatorsTest();
    void operatorsTest_data();
};

tst_QPlaceUser::tst_QPlaceUser()
{
}

void tst_QPlaceUser::constructorTest()
{
    QPlaceUser user;
    QVERIFY(user.name().isEmpty());
    QVERIFY(user.userId().isEmpty());

    user.setName(QStringLiteral("Thomas Anderson"));
    user.setUserId(QStringLiteral("Neo"));

    QPlaceUser user2(user);
    QCOMPARE(user2.name(), QStringLiteral("Thomas Anderson"));
    QCOMPARE(user2.userId(), QStringLiteral("Neo"));
}

void tst_QPlaceUser::nameTest()
{
    QPlaceUser user;
    user.setName(QStringLiteral("Thomas Anderson"));
    QCOMPARE(user.name(), QStringLiteral("Thomas Anderson"));
    user.setName(QString());
    QVERIFY(user.name().isEmpty());
}

void tst_QPlaceUser::userIdTest()
{
    QPlaceUser user;
    user.setUserId(QStringLiteral("Neo"));
    QCOMPARE(user.userId(), QStringLiteral("Neo"));
    user.setUserId(QString());
    QVERIFY(user.userId().isEmpty());
}

void tst_QPlaceUser::operatorsTest()
{
    QPlaceUser user1;
    user1.setName(QStringLiteral("Thomas Anderson"));
    user1.setUserId(QStringLiteral("Neo"));

    QPlaceUser user2;
    user2.setName(QStringLiteral("Thomas Anderson"));
    user2.setUserId(QStringLiteral("Neo"));

    QVERIFY(user1 == user2);
    QVERIFY(!(user1 != user2));
    QVERIFY(user2 == user1);
    QVERIFY(!(user2 != user1));

    QPlaceUser user3;
    QVERIFY(!(user1 == user3));
    QVERIFY(user1 != user3);
    QVERIFY(!(user3 == user1));
    QVERIFY(user3 != user1);

    user3 = user1;
    QVERIFY(user1 == user3);
    QVERIFY(!(user1 != user3));
    QVERIFY(user3 == user1);
    QVERIFY(!(user3 != user1));

    QFETCH(QString, field);

    if (field == QStringLiteral("name"))
        user3.setName(QStringLiteral("bob"));
    else if (field == QStringLiteral("userId"))
        user3.setUserId(QStringLiteral("Morpheus"));
    else
        qFatal("Unknown data field");

    QVERIFY(!(user1 == user3));
    QVERIFY(user1 != user3);
    QVERIFY(!(user3 == user1));
    QVERIFY(user3 != user1);
}

void tst_QPlaceUser::operatorsTest_data()
{
    QTest::addColumn<QString>("field");

    QTest::newRow("user name") << "name";
    QTest::newRow("user id") << "userId";
}

QTEST_APPLESS_MAIN(tst_QPlaceUser)

#include "tst_qplaceuser.moc"
