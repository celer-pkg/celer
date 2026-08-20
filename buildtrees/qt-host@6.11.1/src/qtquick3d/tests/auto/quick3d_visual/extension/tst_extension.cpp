// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QQuickView>

#include "../shared/util.h"

class tst_Extension : public QQuick3DDataTest
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void simple();
};

void tst_Extension::initTestCase()
{
    QQuick3DDataTest::initTestCase();
}

extern int extensionCtorCount;
extern int extensionDtorCount;
extern int extensionFunctional;
extern int childExtensionCtorCount;
extern int childExtensionDtorCount;
extern int childExtensionFunctional;

void tst_Extension::simple()
{
    {
        QScopedPointer<QQuickView> view(createView(QLatin1String("simple.qml"), QSize(640, 480)));
        QVERIFY(view);
        QVERIFY(QTest::qWaitForWindowExposed(view.data()));

        const QImage result = grab(view.data());
        QVERIFY(!result.isNull());
    }

    QCOMPARE(extensionCtorCount, 1);
    QCOMPARE(childExtensionCtorCount, 1);
    QCOMPARE(extensionDtorCount, 1);
    QCOMPARE(childExtensionDtorCount, 1);

    // prepareData, prepareRender, render
    QVERIFY(extensionFunctional >= 3);
    QVERIFY(childExtensionFunctional >= 3);
}

QTEST_MAIN(tst_Extension)
#include "tst_extension.moc"
