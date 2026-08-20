// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/storeapi.h"

#include <QtCore/qdebug.h>
#include <QtCore/qprocess.h>
#include <QtTest/qtest.h>

using namespace std::chrono_literals;

namespace QtOpenAPI {
const QDateTime TestDate(QDateTime::fromString("1.30.1", "M.d.s"));

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

class StoreApiTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        if (serverProcess.state() != QProcess::ProcessState::Running)
            startServerProcess();
    }
    void placeOrderTest();
    void getOrderByIdTest();
    void getInventoryTest();
    void deleteOrderTest();
    void timeoutTest();
    void cleanupTestCase();
};

void StoreApiTests::placeOrderTest() {
    StoreApi api;
    bool orderPlaced = false;
    Order order;
    order.setId(500);
    order.setQuantity(10);
    order.setPetId(10000);
    order.setComplete(false);
    order.setStatus("shipping");
    order.setShipDate(QDateTime::currentDateTimeUtc());
    api.placeOrder(order, this, [&](const QRestReply &reply, const Order &respval) {
        if ((orderPlaced = reply.isSuccess())) {
            QCOMPARE(respval.getShipDateValue(), TestDate);
        } else {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderPlaced, true, 14000);
}

void StoreApiTests::getOrderByIdTest() {
    StoreApi api;
    api.setApiKey("api_key_2","testKey");
    bool orderFetched = false;
    api.getOrderById(500, nullptr, [&](const QRestReply &reply, const Order &respval) {
        if ((orderFetched = reply.isSuccess())) {
            QVERIFY(respval.getPetIdValue() == 10000);
            QVERIFY(respval.getIdValue() == 500);
        } else {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderFetched, true, 14000);
}

void StoreApiTests::getInventoryTest() {
    StoreApi api;
    api.setApiKey("api_key","special-key");
    bool inventoryFetched = false;
    api.getInventory(this, [&](const QRestReply &reply, const QMap<QString, qint32> &respval) {
        if ((inventoryFetched = reply.isSuccess())) {
            for (const auto &key : respval.keys()) {
                qDebug() << (key) << " Quantities " << respval.value(key);
            }
        } else {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        }
    });

    QTRY_COMPARE_EQ_WITH_TIMEOUT(inventoryFetched, true, 14000);
}

void StoreApiTests::deleteOrderTest()
{
    StoreApi api;
    bool orderPlaced = false;
    Order order;
    order.setId(600);
    order.setQuantity(10);
    order.setPetId(20000);
    order.setComplete(false);
    order.setStatus("shipping");
    order.setShipDate(QDateTime::currentDateTimeUtc());
    api.placeOrder(order, this, [&](const QRestReply &reply, const Order &respval) {
        if ((orderPlaced = reply.isSuccess())) {
            QCOMPARE(respval.getShipDateValue(), TestDate);
        } else {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderPlaced, true, 14000);

    bool orderDeleted = false;
    // delete existing order
    api.deleteOrder(QString::number(order.getIdValue()), this, [&](const QRestReply &reply) {
        if (!(orderDeleted = reply.isSuccess())) {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderDeleted, true, 14000);

    orderDeleted = false;
    // try to delete NOT existing order id = 33333
    api.deleteOrder("33333", this, [&](const QRestReply &reply) {
        if (!(orderDeleted = reply.isSuccess())) {
            qDebug() << "Error happened while issuing request 'deleteOrder(33333)': " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderDeleted, true, 14000);
}

void StoreApiTests::timeoutTest()
{
    Order order;
    order.setId(600);
    order.setQuantity(10);
    order.setPetId(20000);
    order.setComplete(false);
    order.setStatus("shipping");
    order.setShipDate(QDateTime::currentDateTimeUtc());

    StoreApi api;
    bool orderPlaced = false;
    api.placeOrder(order, this, [&](const QRestReply &summary) {
        orderPlaced = summary.isSuccess();
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderPlaced, true, 14000);

    bool orderDeleted = false;
    QNetworkReply::NetworkError netError;
    QString errorStr;

    // set a 5 sec timeout, the test server should be able to answer
    // within the timeout (the server sleeps for 1 sec before sending the response)
    api.setTimeOut(5000ms);
    // try to delete NOT existing order id = 33333
    api.deleteOrder("33333", this, [&](const QRestReply &reply) {
        if (!(orderDeleted = reply.isSuccess())) {
            qDebug() << "Error happened while issuing request: " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderDeleted, true, 14000);

    orderDeleted = true;
    // decrease the timeout to 0,1 sec, the server shouldn't be able
    // to response in time (the server sleeps for 1 sec before sending the response)
    api.setTimeOut(100ms);
    // delete existing order
    api.deleteOrder(QString::number(order.getIdValue()), this, [&](const QRestReply &reply) {
        if (!(orderDeleted = reply.isSuccess())) {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
            netError = reply.error();
            errorStr = reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderDeleted, false, 14000);
    QCOMPARE_EQ(netError, QNetworkReply::TimeoutError);
    QCOMPARE_EQ(errorStr, "Operation timed out");
}

void StoreApiTests::cleanupTestCase()
{
    if (serverProcess.state() == QProcess::ProcessState::Running) {
        serverProcess.kill();
        serverProcess.waitForFinished();
    }
}

} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::StoreApiTests)
#include "tst_storeapitests.moc"
