// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QCoreApplication>

#include <QtCoap/qcoapresource.h>
#include <QtCoap/qcoapresourcediscoveryreply.h>
#include <private/qcoapresourcediscoveryreply_p.h>

class tst_QCoapResource : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void parseCoreLink_data();
    void parseCoreLink();
};

void tst_QCoapResource::parseCoreLink_data()
{
    QTest::addColumn<int>("resourceNumber");
    QTest::addColumn<QString>("senderAddress");
    QTest::addColumn<QList<QString>>("pathList");
    QTest::addColumn<QList<QString>>("titleList");
    QTest::addColumn<QList<QString>>("resourceTypeList");
    QTest::addColumn<QList<uint>>("contentFormatList");
    QTest::addColumn<QList<QString>>("interfaceList");
    QTest::addColumn<QList<int>>("maximumSizeList");
    QTest::addColumn<QList<bool>>("observableList");
    QTest::addColumn<QByteArray>("coreLinkList");

    QList<QString> pathList;
    pathList << "/obs" << "/separate" << "/seg1" << "/seg1/seg2" << "/large-separate"
             << "/.well-known/core" << "/multi-format" << "/path"
             << "/path/sub1" << "/link1" << "/validate" << "/test"
             << "/query" << "/large-post" << "/obs-non" << "/shutdown";

    QList<QString> titleList;
    titleList << "Observable resource which changes every 5 seconds"
              << "Resource which cannot be served immediately and which cannot be acknowledged in a piggy-backed way"
              << "Long path resource"
              << "Long path resource"
              << "Large resource"
              << ""
              << "Resource that exists in different content formats (text/plain utf8 and application/xml)"
              << "Hierarchical link description entry"
              << "Hierarchical link description sub-resource"
              << "Link test resource"
              << "Resource which varies"
              << "Default test resource"
              << "Resource accepting query parameters"
              << "Handle PostOperation with two-way blockwise transfer"
              << "Observable resource which changes every 5 seconds"
              << "";

    QList<QString> resourceTypeList;
    resourceTypeList << "observe" << "" << "" << "" << "block" << "" << "" << "" << ""
                     << "Type1 Type2" << "" << "" << "" << "block" << "observe" << "";

    QList<uint> contentFormatList;
    contentFormatList << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 40 << 0 << 0 << 0 << 0
                      << 0 << 0 << 0 << 0;

    QList<QString> interfaceList;
    interfaceList << "" << "" << "" << "" << "" << "" << "" << "" << "" << "If1"
                  << "" << "" << "" << "" << "" << "";

    QList<int> maximumSizeList;
    maximumSizeList << -1 << -1 << -1 << -1 << 1280  << -1 << -1 << -1 << -1 << -1
                    << -1 << -1 << -1 << -1 << -1 << -1;

    QList<bool> observableList;
    observableList << true << false << false << false << false << false << false
                   << false << false << false << false << false << false << false
                   << true << false;

    QByteArray coreLinks;
    // Resources are separated by a comma
    coreLinks.append("</obs>;obs;rt=\"observe\";title=\"Observable resource which changes every"
                     " 5 seconds\",</separate>;title=\"Resource which cannot be served immediately"
                     " and which cannot be acknowledged in a piggy-backed way\",</seg1>;title=\""
                     "Long path resource\",</seg1/seg2>;title=\"Long path resource\","
                     "</large-separate>;rt=\"block\";sz=1280;title=\"Large resource\","
                     "</.well-known/core>,</multi-format>;ct=\"0 41\";title=\"Resource that exists"
                     " in different content formats (text/plain utf8 and application/xml)\","
                     "</path>;ct=40;title=\"Hierarchical link description entry\",</path/sub1>;"
                     "title=\"Hierarchical link description sub-resource\",</link1>;if=\"If1\";"
                     "rt=\"Type1 Type2\";title=\"Link test resource\",</validate>;title=\"Resource"
                     " which varies\",</test>;title=\"Default test resource\",</query>;"
                     "title=\"Resource accepting query parameters\",</large-post>;rt=\"block\";"
                     "title=\"Handle PostOperation with two-way blockwise transfer\",</obs-non>;"
                     "obs;rt=\"observe\";title=\"Observable resource which changes every 5 "
                     "seconds\",</shutdown>");

    QTest::newRow("parse") << 16
                           << QString("10.20.30.40")
                           << pathList
                           << titleList
                           << resourceTypeList
                           << contentFormatList
                           << interfaceList
                           << maximumSizeList
                           << observableList
                           << coreLinks;
}

void tst_QCoapResource::parseCoreLink()
{
#ifdef QT_BUILD_INTERNAL
    QFETCH(int, resourceNumber);
    QFETCH(QString, senderAddress);
    QFETCH(QList<QString>, pathList);
    QFETCH(QList<QString>, titleList);
    QFETCH(QList<QString>, resourceTypeList);
    QFETCH(QList<uint>, contentFormatList);
    QFETCH(QList<QString>, interfaceList);
    QFETCH(QList<int>, maximumSizeList);
    QFETCH(QList<bool>, observableList);
    QFETCH(QByteArray, coreLinkList);

    const auto resourceList =
            QCoapResourceDiscoveryReplyPrivate::resourcesFromCoreLinkList(
                QHostAddress(senderAddress), coreLinkList);

    QCOMPARE(resourceList.size(), resourceNumber);

    int resourceIndex = 0;
    for (const auto &resource : resourceList) {
        QCOMPARE(resource.host(), QHostAddress(senderAddress));
        QCOMPARE(resource.path(), pathList[resourceIndex]);
        QCOMPARE(resource.title(), titleList[resourceIndex]);
        QCOMPARE(resource.resourceType(), resourceTypeList[resourceIndex]);
        QCOMPARE(resource.contentFormat(), contentFormatList[resourceIndex]);
        QCOMPARE(resource.interface(), interfaceList[resourceIndex]);
        QCOMPARE(resource.maximumSize(), maximumSizeList[resourceIndex]);
        QCOMPARE(resource.observable(), observableList[resourceIndex]);
        ++resourceIndex;
    }
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}

QTEST_APPLESS_MAIN(tst_QCoapResource)

#include "tst_qcoapresource.moc"
