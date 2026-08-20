// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QQuickView>
#include <QQuickWindow>

#include <memory>

#include "../shared/util.h"

class tst_SourceItem : public QQuick3DDataTest
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void sourceItem();
};

void tst_SourceItem::initTestCase()
{
    QQuick3DDataTest::initTestCase();
    if (!initialized())
        return;
}

const int FUZZ = 5;

void tst_SourceItem::sourceItem()
{
    std::unique_ptr<QQuickWindow> window(createWindow(QLatin1String("SourceItem.qml"), QSize(640, 480)));

    QVERIFY(QTest::qWaitForWindowExposed(window.get()));

    // Find the root item and reparent it to null, then back to the contentItem()
    QQuickItem *rootItem = window->contentItem()->childItems().first();
    rootItem->setParentItem(window->contentItem());

    QImage result = grab(window.get());
    if (result.isNull())
        return; // was QFAIL'ed already

    const qreal dpr = window->devicePixelRatio();

    QVERIFY(comparePixel(result, 50, 50, dpr, Qt::black, FUZZ));

    // The model is a rect with a red source item, so it should still be red
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));

    // Destroy window and set rootItem parent to null
    rootItem->setParentItem(nullptr);

    window->destroy();

    // Create window again and set rootItem parent to contentItem
    window->create();
    rootItem->setParentItem(window->contentItem());
    window->show();

    QVERIFY(QTest::qWaitForWindowExposed(window.get()));

    result = grab(window.get());
    if (result.isNull())
        return; // was QFAIL'ed already

    rootItem->setParentItem(window->contentItem());
    result = grab(window.get());
    if (result.isNull())
        return; // was QFAIL'ed already

    QVERIFY(comparePixel(result, 50, 50, dpr, Qt::black, FUZZ));

    // The model is a rect with a red source item, so it should still be red
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));
}

QTEST_MAIN(tst_SourceItem)
#include "tst_sourceitem.moc"
