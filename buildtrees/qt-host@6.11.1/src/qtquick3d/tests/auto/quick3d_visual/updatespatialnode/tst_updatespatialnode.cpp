// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QQuickView>

#include "../shared/util.h"

#include "testupdatespatialnode.h"

class tst_UpdateSpatialNode : public QQuick3DDataTest
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void updateResource();
};

void tst_UpdateSpatialNode::initTestCase()
{
    QQuick3DDataTest::initTestCase();
}

extern TestData testData_ResourceObject;
extern TestData testData_NodeObject;
extern TestData testData_ExtensionObject;

void tst_UpdateSpatialNode::updateResource()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("UpdateSpatialNode.qml"), QSize(640, 480)));
    QVERIFY(view);
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    const QImage result = grab(view.data());
    QVERIFY(!result.isNull());

    QTRY_VERIFY(testData_ResourceObject.callCount == testData_ResourceObject.expectedCallCount);
    QTRY_VERIFY(testData_NodeObject.callCount == testData_NodeObject.expectedCallCount);
    QTRY_VERIFY(testData_ExtensionObject.callCount == testData_ExtensionObject.expectedCallCount);

    // Verify that the objects are destroyed (once). If this is more then one the test code is wrong.
    QCOMPARE(testData_ResourceObject.destroyedCount, 1);
    QCOMPARE(testData_NodeObject.destroyedCount, 1);
    QCOMPARE(testData_ExtensionObject.destroyedCount, 1);
}

QTEST_MAIN(tst_UpdateSpatialNode)
#include "tst_updatespatialnode.moc"
