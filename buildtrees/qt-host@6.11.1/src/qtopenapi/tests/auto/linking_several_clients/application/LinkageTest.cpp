// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "client1testapi.h"
#include "client2testapi.h"

#include <QtCore/qobject.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace QtOpenApiCommon;

namespace QtOpenAPI {

/**
 *
 * The following test checks if several separate client libraries
 * can use the common OpenApiCommon library and can be linked to
 * the resulting executable file without clashing of names.
 *
 * NOTE:
 * The test does not require a server side, as we are not testing
 * the data transfer here.
 *
**/
class LinkageTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testLibraries();
};

void LinkageTest::testLibraries()
{
    Client1TestApi api1;
    Client2TestApi api2;
    QOAIHttpFileElement object("NoName.txt");
    bool done = true;

    api1.simpleExplodeString("Test String"_L1, this,
                             [&](const QRestReply &reply, const QString &) {
                                 done = reply.isSuccess();
                             });

    QTRY_COMPARE_EQ(done, false);

    done = true;
    api2.simpleExplodeInt(12345, this,
                          [&](const QRestReply &reply, const QString &) {
                                 done = reply.isSuccess();
                          });
    QTRY_COMPARE_EQ(done, false);
}

} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::LinkageTest)
#include "LinkageTest.moc"
