// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/userapi.h"

#include <QtCore/qdebug.h>
#include <QtCore/qprocess.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace QtOpenApiCommon;

namespace QtOpenAPI {
static QProcess serverProcess;
void startServerProcess()
{
    serverProcess.start(SERVER_PATH);
    if (!serverProcess.waitForStarted()) {
        qFatal() << "Couldn't start the server: " << serverProcess.errorString();
        exit(EXIT_FAILURE);
    }
    // give the process some time to properly start up the server
    QThread::currentThread()->msleep(1000);
}

const int REPLY_OK = 200;

class UserApiTests : public QObject {
    Q_OBJECT

    User createRandomUser();

private Q_SLOTS:
    void initTestCase()
    {
        if (serverProcess.state() != QProcess::ProcessState::Running)
            startServerProcess();
    }
    void createUserTest();
    void createInQueryMapTest();
    void createUsersWithArrayInputTest();
    void createUsersWithListInputTest();
    void deleteUserTest();
    void getUserByNameTest();
    void loginUserTest();
    void logoutUserTest_data();
    void logoutUserTest();
    void updateUserTest();
    void cleanupTestCase();
};

User UserApiTests::createRandomUser() {
    User user;
    user.setId(QDateTime::currentMSecsSinceEpoch());
    user.setEmail("Jane.Doe@QtOpenAPItools.io");
    user.setFirstName("Jane");
    user.setLastName("Doe");
    user.setPhone("123456789");
    user.setUsername("janedoe");
    user.setPassword("secretPassword");
    user.setUserStatus(static_cast<int>(rand()));
    return user;
}

void UserApiTests::createUserTest() {
    UserApi api;
    bool userCreated = false;
    api.createUser(createRandomUser(), this, [&](QRestReply &reply) {
        if (!(userCreated = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userCreated, true, 14000);
}

void UserApiTests::createInQueryMapTest()
{
    UserApi api;
    bool usersCreated = false;
    int status = static_cast<int>(rand());
    api.createInQueryMap({{"Ivan", QString::number(status)}}, this, [&](QRestReply &reply) {
        if (!(usersCreated = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(usersCreated, true, 14000);

    bool userFetched = false;
    api.getUserByName({{"Ivan", status}}, this, [&](const QRestReply &reply, const User &summary) {
        if (!(userFetched = reply.isSuccess()))
            qWarning() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
        QCOMPARE(summary.getUsernameValue(), "Ivan");
        QCOMPARE(summary.getUserStatusValue(), status);
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userFetched, true, 14000);
}

void UserApiTests::createUsersWithArrayInputTest() {
    UserApi api;
    bool usersCreated = false;
    QList<User> users;
    users.append(createRandomUser());
    users.append(createRandomUser());
    users.append(createRandomUser());
    api.createUsersWithArrayInput(users, this, [&](QRestReply &reply) {
        if (!(usersCreated = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(usersCreated, true, 14000);
}

void UserApiTests::createUsersWithListInputTest() {
    UserApi api;
    bool usersCreated = false;
    QList<User> users;
    auto johndoe = createRandomUser();
    johndoe.setUsername("johndoe");
    auto rambo = createRandomUser();
    rambo.setUsername("rambo");
    users.append(johndoe);
    users.append(rambo);
    users.append(createRandomUser());
    api.createUsersWithListInput(users, this, [&](QRestReply &reply) {
        if (!(usersCreated = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(usersCreated, true, 14000);
}

void UserApiTests::deleteUserTest() {
    UserApi api;
    bool operationStatus = false;
    auto stallone = createRandomUser();

    stallone.setUsername("sylvester");
    api.createUser(stallone, this, [&](QRestReply &reply) {
        if (!(operationStatus = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(operationStatus, true, 14000);

    operationStatus = false;
    api.deleteUser(stallone, this, [&](QRestReply &reply) {
        if (!(operationStatus = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(operationStatus, true, 14000);
}

void UserApiTests::getUserByNameTest() {
    UserApi api;
    bool userFetched = false;
    auto mrSmith = createRandomUser();

    mrSmith.setUsername("mrSmith");
    api.createUser(mrSmith, this, [&](QRestReply &reply) {
        if (!(userFetched = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userFetched, true, 14000);

    userFetched = false;
    api.getUserByName({{mrSmith.getUsernameValue(), mrSmith.getUserStatusValue()}}, this, [&](const QRestReply &reply, const User &summary) {
        if (!(userFetched = reply.isSuccess()))
            qWarning() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
        QCOMPARE(summary.asJson(), mrSmith.asJson());
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userFetched, true, 14000);
}

void UserApiTests::loginUserTest() {
    UserApi api;
    bool userLogged = false;
    QString expectedString;

    connect(&api, &UserApi::loginUserFinished,
            this, [&](const QString &summary) {
        userLogged = true;
        expectedString = summary;
    });
    connect(&api, &UserApi::loginUserErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        userLogged = false;
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    QString user_name = "johndoe";

    api.loginUser(user_name);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userLogged, true, 14000);
    QCOMPARE(expectedString, "johndoe"); // Password was not set; only username is returned

    userLogged = false;
    api.loginUser(user_name, OptionalParameter<QString>(OptionalParameter<QString>::IsNull));
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userLogged, true, 14000);
    QCOMPARE(expectedString, "johndoe"); // Password is explicitly marked as null via OptionalParameter::IsNull; username is returned.

    userLogged = false;
    api.loginUser(OptionalParameter<QString>(), QString("123456789"_L1));
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userLogged, true, 14000);
    QCOMPARE(expectedString, "123456789"); // Username is empty; only password is returned.

    userLogged = false;
    api.loginUser(user_name, OptionalParameter<QString>("123456789"));
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userLogged, true, 14000);
    QCOMPARE(expectedString, "johndoe123456789"); // Username and password are non-empty; both are returned concatenated.
}

void UserApiTests::logoutUserTest_data()
{
    QTest::addColumn<QJsonValue>("jsonValue");
    QTest::newRow("QJsonValue(string)") << QJsonValue("johndoe");
    QTest::newRow("QJsonValue(int)")    << QJsonValue(100);
    QTest::newRow("QJsonValue(array)")  << QJsonValue({1, 2.2, QString("Strange")});
    QTest::newRow("QJsonValue(object)") << QJsonValue(createRandomUser().asJsonObject());
    QTest::newRow("QJsonValue()")       << QJsonValue();
    QTest::newRow("QJsonValue(Null)")   << QJsonValue(QJsonValue::Null);
}

void UserApiTests::logoutUserTest()
{
    QFETCH(QJsonValue, jsonValue);
    UserApi api;
    bool userLoggedOut = false;
    api.logoutUser(jsonValue, this, [&](QRestReply &reply) {
        if (!(userLoggedOut = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userLoggedOut, true, 14000);
}

void UserApiTests::updateUserTest() {
    UserApi api;
    bool operationStatus = false;
    auto grumpy = createRandomUser();

    grumpy.setUsername("MrGrump");
    api.createUser(grumpy, this, [&](QRestReply &reply) {
        if (!(operationStatus = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(operationStatus, true, 14000);

    operationStatus = false;
    grumpy.setFirstName("Stephan");
    grumpy.setLastName("Newman");
    api.updateUser("MrGrump", grumpy, this, [&](const QRestReply &reply, const User &summary) {
        if (!(operationStatus = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        QCOMPARE(reply.httpStatus(), REPLY_OK);
        QCOMPARE(QUrl::fromPercentEncoding(summary.asJson().toUtf8()), grumpy.asJson());
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(operationStatus, true, 14000);
}

void UserApiTests::cleanupTestCase()
{
    if (serverProcess.state() == QProcess::ProcessState::Running) {
        serverProcess.kill();
        serverProcess.waitForFinished();
    }
}
} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::UserApiTests)
#include "tst_userapitests.moc"
