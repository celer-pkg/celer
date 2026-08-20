// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>
#include <QtQuick/private/qquickloader_p.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtQuickControlsTestUtils/private/controlstestutils_p.h>
#include <QtQuickTemplates2/private/qquickapplicationwindow_p.h>

using namespace QQuickControlsTestUtils;

class tst_QQuickStackView : public QQmlDataTest
{
    Q_OBJECT

public:
    tst_QQuickStackView();

private slots:
    void crashWhenContextAliveButEngineDestroyed();
};

tst_QQuickStackView::tst_QQuickStackView()
    : QQmlDataTest(QT_QMLTEST_DATADIR, FailOnWarningsPolicy::FailOnWarnings)
{
}

void tst_QQuickStackView::crashWhenContextAliveButEngineDestroyed()
{
    QQuickControlsApplicationHelper helper(this, "crashWhenContextAliveButEngineDestroyed.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
}

QTEST_MAIN(tst_QQuickStackView)

#include "tst_qquickstackview.moc"
