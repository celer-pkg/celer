// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../../../../shared/testutils.h"

#include <QtTest/QtTest>
#include <QMetaType>
#include <QProcess>

typedef QLatin1String _;
class tst_Signature: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        QVERIFY(TestUtils::init("signatureTests"));
        QLoggingCategory::setFilterRules("qt.remoteobjects.warning=false");
    }

    void cleanup()
    {
        // wait for delivery of RemoveObject events to the source
        QTest::qWait(200);
    }

    void testRun()
    {
#ifdef Q_OS_ANDROID
        QSKIP("QProcess doesn't support running user bundled binaries on Android");
#endif
        qDebug() << "Starting signatureServer process";
        QProcess serverProc;
        serverProc.setProcessChannelMode(QProcess::ForwardedChannels);
        serverProc.start(TestUtils::findExecutable("signatureServer", "/signatureServer"),
                         QStringList());
        QVERIFY(serverProc.waitForStarted());

        // wait for server start
        QTest::qWait(200);

        const QLatin1String tests[] = {
            _("differentGlobalEnum"),
            _("differentClassEnum"),
            _("differentPropertyCount"),
            _("differentPropertyCountChild"),
            _("differentPropertyType"),
            _("scrambledProperties"),
            _("differentSlotCount"),
            _("differentSlotType"),
            _("differentSlotParamCount"),
            _("differentSlotParamType"),
            _("scrambledSlots"),
            _("differentSignalCount"),
            _("differentSignalParamCount"),
            _("differentSignalParamType"),
            _("scrambledSignals"),
            _("state"),
            _("matchAndQuit"), // matchAndQuit should be the last one
        };

        for (const auto &test : tests) {
            qDebug() << "Starting" << test << "process";
            QProcess testProc;
            testProc.setProcessChannelMode(QProcess::ForwardedChannels);
            testProc.start(TestUtils::findExecutable(test, "/" + test ),
                           QStringList());
            QVERIFY(testProc.waitForStarted());
            QVERIFY(testProc.waitForFinished());
            QCOMPARE(testProc.exitCode(), 0);
        }

        QVERIFY(serverProc.waitForFinished());
        QCOMPARE(serverProc.exitCode(), 0);
    }
};

QTEST_MAIN(tst_Signature)

#include "tst_signature.moc"
