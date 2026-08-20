// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mytestserver.h"
#include "rep_subclass_source.h"
#include "../shared.h"

#include <QCoreApplication>
#include <QtTest/QtTest>

#include "../../../shared/testutils.h"

static QMap<int, MyPOD> int_map{{1, initialValue},
                                {16, initialValue}};
static MyTestServer::ActivePositions flags1 = MyTestServer::Position::position1;
static MyTestServer::ActivePositions flags2 = MyTestServer::Position::position2
                                              | MyTestServer::Position::position3;
static QMap<MyTestServer::ActivePositions, MyPOD> my_map{{flags1, initialValue},
                                                         {flags2, initialValue}};
static QHash<NS2::NamespaceEnum, MyPOD> my_hash{{NS2::NamespaceEnum::Alpha, initialValue},
                                                {NS2::NamespaceEnum::Charlie, initialValue}};

class tst_Server_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        const auto objectMode = qEnvironmentVariable("ObjectMode");
        bool templated = qEnvironmentVariableIsSet("TEMPLATED_REMOTING");

        qDebug() << "Starting tests:" << objectMode << "templated =" << templated;
        QRemoteObjectRegistryHost srcNode(QUrl(QStringLiteral(LOCAL_SOCKET ":testRegistry")));

        MyTestServer parent;
        SubClassSimpleSource subclass;
        subclass.setMyPOD(initialValue);
        subclass.setI(initialI);
        QStringListModel model;
        model.setStringList(QStringList() << "Track1" << "Track2" << "Track3");
        if (objectMode == QLatin1String("ObjectPointer")) {
            parent.setSubClass(&subclass);
            parent.setTracks(&model);
            parent.setMyEnum(ParentClassSource::bar);
            parent.setDate(Qt::RFC2822Date);
            parent.setNsEnum(NS::Bravo);
            parent.setNs2Enum(NS2::NamespaceEnum::Bravo);
            parent.setVariant(QVariant::fromValue(42.0f));
            parent.setSimpleList(QList<QString>() << "one" << "two");
            parent.setPodList(QList<MyPOD>() << initialValue << initialValue);
            parent.setIntMap(int_map);
            parent.setEnumMap(my_map);
            parent.setPodHash(my_hash);
        }

        if (templated)
            srcNode.enableRemoting<ParentClassSourceAPI>(&parent);
        else
            srcNode.enableRemoting(&parent);

        qDebug() << "Waiting for incoming connections";

        QSignalSpy waitForStartedSpy(&parent, &MyTestServer::startedChanged);
        QVERIFY(waitForStartedSpy.isValid());
        QVERIFY(waitForStartedSpy.wait());
        QCOMPARE(waitForStartedSpy.value(0).value(0).toBool(), true);

        // wait for delivery of events
        QTest::qWait(200);

        //Change SubClass and make sure change propagates
        SubClassSimpleSource updatedSubclass;
        updatedSubclass.setMyPOD(updatedValue);
        updatedSubclass.setI(updatedI);
        parent.setSubClass(&updatedSubclass);
        if (objectMode == QLatin1String("NullPointer"))
            parent.setTracks(&model);
        parent.setMyEnum(ParentClassSource::foobar);
        parent.setDate(Qt::ISODateWithMs);
        parent.setNsEnum(NS::Charlie);
        parent.setNs2Enum(NS2::NamespaceEnum::Charlie);
        parent.setVariant(QVariant::fromValue(podValue));
        emit parent.enum2(ParentClassSource::foo, ParentClassSource::bar);

        emit parent.advance();

        // wait for quit
        bool quit = false;
        connect(&parent, &MyTestServer::quitApp, this, [&quit] { quit = true; });
        QTRY_VERIFY_WITH_TIMEOUT(quit, 5000);

        // wait for delivery of events
        QTest::qWait(200);

        qDebug() << "Done. Shutting down.";
    }
};

QTEST_MAIN(tst_Server_Process)

#include "main.moc"
