// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QQuickView>
#include <QQmlError>

#include "../shared/util.h"

#include <private/qsgrenderloop_p.h>
#include <private/qsgrhisupport_p.h>

class tst_SmokeTest : public QQuick3DDataTest
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void bringup();
};

void tst_SmokeTest::initTestCase()
{
    QQuick3DDataTest::initTestCase();
    if (!initialized())
        return;

    QSGRenderLoop *loop = QSGRenderLoop::instance();
    qDebug() << "Render loop:" << loop
             << "RHI backend:" << QSGRhiSupport::instance()->rhiBackendName();

    // Get as many graphics init logs printed as we can, to help
    // troubleshooting both locally and in CI VMs.
    qputenv("QSG_INFO", "1");
}

void tst_SmokeTest::bringup()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("view.qml"), QSize(640, 480)));
    QVERIFY(view);
    QVERIFY(view->errors().isEmpty());
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));
}

QTEST_MAIN(tst_SmokeTest)
#include "tst_smoketest.moc"
