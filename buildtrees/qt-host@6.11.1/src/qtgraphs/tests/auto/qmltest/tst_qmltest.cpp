// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQuickTest/quicktest.h>

class tst_qmltest: public QObject
{
    Q_OBJECT
private slots:
    void skiptest() { QSKIP("This test will fail, skipping."); };
};

int main(int argc, char **argv)
{
    if (!qEnvironmentVariableIsEmpty("QEMU_LD_PREFIX")) {
        qWarning("This test would fail due to QEMU emulation shortcomings, so it will be skipped.");
        tst_qmltest skip;
        return QTest::qExec(&skip, argc, argv);
    }
    if (qEnvironmentVariable("QTEST_ENVIRONMENT").split(' ').contains("ci") &&
        qEnvironmentVariable("QT_QPA_PLATFORM").split(' ').contains("offscreen")
    ) {
        qWarning("This test would fail on CI on offscreen test targets, so it will be skipped.");
        tst_qmltest skip;
        return QTest::qExec(&skip, argc, argv);
    }
#ifdef Q_OS_VXWORKS
    qWarning("This test would fail due to VxWorks QtQuick3D support shortcomings, so it will "
             "be skipped.");
    tst_qmltest skip;
    return QTest::qExec(&skip, argc, argv);
#endif
#if defined(Q_OS_WINDOWS) && defined(_M_ARM64)
    qWarning("This test would fail due to ARM Windows QtQuick3D support shortcomings, so it will "
             "be skipped.");
    tst_qmltest skip;
    return QTest::qExec(&skip, argc, argv);
#endif
    QTEST_SET_MAIN_SOURCE_PATH
    return quick_test_main(argc, argv, "qmltest", QUICK_TEST_SOURCE_DIR);
}

#include "tst_qmltest.moc"
