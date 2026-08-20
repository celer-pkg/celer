// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQuickTest/quicktest.h>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>

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
    if (!qEnvironmentVariableIsEmpty("QT_QPA_OFFSCREEN_NO_GLX")) {
        qWarning("This test would fail due to missing GLX, so it will be skipped.");
        tst_qmltest skip;
        return QTest::qExec(&skip, argc, argv);
    }
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen")) {
        qWarning("This test would fail due to offscreen target, so it will be skipped.");
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

    {
        QGuiApplication app(argc, argv);
        if (!QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::OpenGL)) {
            qWarning("OpenGL is not enabled in this configuration, so it will be skipped.");
            tst_qmltest skip;
            return QTest::qExec(&skip, argc, argv);
        }
    }

    qputenv("QSG_RHI_BACKEND", "opengl");
    QTEST_SET_MAIN_SOURCE_PATH
    return quick_test_main(argc, argv, "qmltest", QUICK_TEST_SOURCE_DIR);
}

#include "tst_qmltest.moc"
