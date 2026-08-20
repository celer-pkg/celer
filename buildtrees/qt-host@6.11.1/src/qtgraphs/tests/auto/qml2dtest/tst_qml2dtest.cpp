// Copyright (C) 2024 The Qt Company Ltd.
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
#ifdef Q_OS_QNX
    if (qEnvironmentVariable("QTEST_ENVIRONMENT").split(' ').contains("ci") &&
        qEnvironmentVariable("QT_QPA_PLATFORM").split(' ').contains("offscreen")
    ) {
        qWarning("This test would fail on CI QNX QEMU without OpenGL support, so it will be skipped.");
        tst_qmltest skip;
        return QTest::qExec(&skip, argc, argv);
    }
#endif
    QTEST_SET_MAIN_SOURCE_PATH
    return quick_test_main(argc, argv, "qml2dtest", QUICK_TEST_SOURCE_DIR);
}

#include "tst_qml2dtest.moc"
