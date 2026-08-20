// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QQuickView>

#include "../shared/util.h"

class tst_SimpleScene : public QQuick3DDataTest
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cube();
};

void tst_SimpleScene::initTestCase()
{
    QQuick3DDataTest::initTestCase();
    if (!initialized())
        return;
}

const int FUZZ = 5;

void tst_SimpleScene::cube()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("cube.qml"), QSize(640, 480)));
    QVERIFY(view);
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    const QImage result = grab(view.data());
    if (result.isNull())
        return; // was QFAIL'ed already

    const qreal dpr = view->devicePixelRatio();

    QVERIFY(comparePixel(result, 50, 50, dpr, Qt::black, FUZZ));

    // front face of the cube is lighter gray-ish
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(239, 239, 239), FUZZ));

    // the top is darker
    QVERIFY(comparePixelNormPos(result, 0.5, 0.375, QColor::fromRgb(181, 181, 181), FUZZ));
}

QTEST_MAIN(tst_SimpleScene)
#include "tst_simplescene.moc"
