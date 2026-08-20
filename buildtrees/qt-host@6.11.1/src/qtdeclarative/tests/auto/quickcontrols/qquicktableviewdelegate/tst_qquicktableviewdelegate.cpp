// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testmodel.h"

#include <QtQuickTest/quicktest.h>

#include <QtQuick/qquickview.h>
#include <QtQuick/private/qquicktableview_p.h>
#include <QtQuick/private/qquicktableview_p_p.h>

#include <QtQuickTemplates2/private/qquicktableviewdelegate_p.h>

#include <QtQuickTestUtils/private/viewtestutils_p.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>

#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

using namespace QQuickViewTestUtils;
using namespace QQuickVisualTestUtils;

#define LOAD_TABLEVIEW(fileName) \
    view->setSource(testFileUrl(fileName)); \
    view->show(); \
    QVERIFY(QTest::qWaitForWindowActive(view.get())); \
    QQuickTableView *tableView = getTableView(); \
    QVERIFY(tableView); \
    [[maybe_unused]] auto tableViewPrivate = QQuickTableViewPrivate::get(tableView); \
    [[maybe_unused]] auto model = tableView->model().value<TestModel *>()

// ########################################################

class tst_qquicktableviewdelegate : public QQmlDataTest
{
    Q_OBJECT

public:
    tst_qquicktableviewdelegate();

private slots:
    void initTestCase() override;
    void showTableView();
    void checkProperties();
    void checkCurrentIndex();
    void checkClickedSignal_data();
    void checkClickedSignal();
    void clearSelectionOnClick();
    void dragToSelect();
    void pressAndHoldToSelect();

private:
    QQuickTableView *getTableView();
    QQuickTableViewDelegate *getDelegateItem(int row, int column);

private:
    std::unique_ptr<QQuickView> view = nullptr;
};

tst_qquicktableviewdelegate::tst_qquicktableviewdelegate()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
}

void tst_qquicktableviewdelegate::initTestCase()
{
    QQmlDataTest::initTestCase();
    qmlRegisterType<TestModel>("TestModel", 1, 0, "TestModel");
    view.reset(createView());
}

void tst_qquicktableviewdelegate::showTableView()
{
    LOAD_TABLEVIEW("unmodified.qml");
    QCOMPARE(tableViewPrivate->loadedRows.count(), 1);
    QCOMPARE(tableViewPrivate->loadedColumns.count(), 1);
}

void tst_qquicktableviewdelegate::checkProperties()
{
    LOAD_TABLEVIEW("unmodified.qml");
    QCOMPARE(tableViewPrivate->loadedRows.count(), 1);
    QCOMPARE(tableViewPrivate->loadedColumns.count(), 1);

    const QQuickTableViewDelegate *item = getDelegateItem(0, 0);
    QVERIFY(item);
    QCOMPARE(item->property("displayText"), "0,0");
    QCOMPARE(item->tableView(), tableView);

    constexpr auto TEXT_0_1 = "0,1";
    model->appendColumn();
    model->setData(0, 1, TEXT_0_1, Qt::DisplayRole);
    QVERIFY(QQuickTest::qWaitForPolish(tableView));

    QCOMPARE(tableViewPrivate->loadedRows.count(), 1);
    QCOMPARE(tableViewPrivate->loadedColumns.count(), 2);

    item = getDelegateItem(0, 1);
    QVERIFY(item);
    QCOMPARE(item->property("displayText"), TEXT_0_1);
    QCOMPARE(item->tableView(), tableView);

    constexpr auto TEXT_1_0 = "1,0";
    constexpr auto TEXT_1_1 = "1,1";
    model->appendRow();
    model->setData(1, 0, TEXT_1_0, Qt::DisplayRole);
    model->setData(1, 1, TEXT_1_1, Qt::DisplayRole);
    QVERIFY(QQuickTest::qWaitForPolish(tableView));

    QCOMPARE(tableViewPrivate->loadedRows.count(), 2);
    QCOMPARE(tableViewPrivate->loadedColumns.count(), 2);

    item = getDelegateItem(1, 0);
    QVERIFY(item);
    QCOMPARE(item->property("displayText"), TEXT_1_0);
    QCOMPARE(item->tableView(), tableView);

    item = getDelegateItem(1, 1);
    QVERIFY(item);
    QCOMPARE(item->property("displayText"), TEXT_1_1);
    QCOMPARE(item->tableView(), tableView);
}

void tst_qquicktableviewdelegate::checkCurrentIndex()
{
    // Check that that a cell becomes current when you click on it
    LOAD_TABLEVIEW("unmodified.qml");
    QCOMPARE(tableViewPrivate->loadedRows.count(), 1);

    const QQuickTableViewDelegate *item = getDelegateItem(0, 0);
    QVERIFY(item);
    QVERIFY(!item->current());
    QVERIFY(!item->selected());
    QVERIFY(!item->editing());
    QVERIFY(!tableView->hasActiveFocus());

    // Click on the label
    const QPoint localPos = QPoint(item->width() / 2, item->height() / 2);
    const QPoint pos = item->window()->contentItem()->mapFromItem(item, localPos).toPoint();
    QTest::mouseClick(item->window(), Qt::LeftButton, Qt::NoModifier, pos);
    QVERIFY(item->current());
    QVERIFY(!item->selected());
    QVERIFY(!item->editing());
    QVERIFY_ACTIVE_FOCUS(tableView);

    // Select the cell
    tableView->selectionModel()->setCurrentIndex(tableView->modelIndex({0, 0}), QItemSelectionModel::Select);
    QVERIFY(item->current());
    QVERIFY(item->selected());
    QVERIFY(!item->editing());
}

void tst_qquicktableviewdelegate::checkClickedSignal_data()
{
    QTest::addColumn<bool>("pointerNavigationEnabled");
    QTest::newRow("pointer navigation enabled") << true;
    QTest::newRow("pointer navigation disabled") << false;
}

void tst_qquicktableviewdelegate::checkClickedSignal()
{
    // Check that the delegate emits clicked when clicking on the
    // label, but not when clicking on the indicator. This API is
    // a part of the AbstractButton API, and should work with or
    // without TableView.pointerNavigationEnabled set.
    QFETCH(bool, pointerNavigationEnabled);

    LOAD_TABLEVIEW("unmodified.qml");
    tableView->setPointerNavigationEnabled(pointerNavigationEnabled);

    const auto item = tableView->itemAtIndex(tableView->index(0, 0));
    QVERIFY(item);

    QSignalSpy clickedSpy(item, SIGNAL(clicked()));

    // Click on the label
    const QPoint localPos = QPoint(item->width() / 2, item->height() / 2);
    const QPoint pos = item->window()->contentItem()->mapFromItem(item, localPos).toPoint();
    QTest::mouseClick(item->window(), Qt::LeftButton, Qt::NoModifier, pos);
    QCOMPARE(clickedSpy.size(), 1);
    clickedSpy.clear();
}

void tst_qquicktableviewdelegate::clearSelectionOnClick()
{
    LOAD_TABLEVIEW("unmodified.qml");

    // Select root item
    const auto index = tableView->selectionModel()->model()->index(0, 0);
    tableView->selectionModel()->select(index, QItemSelectionModel::Select);
    QCOMPARE(tableView->selectionModel()->selectedIndexes().size(), 1);

    // Click on a cell. This should remove the selection
    const auto item = getDelegateItem(0, 0);
    QVERIFY(item);
    const QPoint localPos = QPoint(item->width() / 2, item->height() / 2);
    const QPoint pos = item->window()->contentItem()->mapFromItem(item, localPos).toPoint();
    QTest::mouseClick(item->window(), Qt::LeftButton, Qt::NoModifier, pos);
    QCOMPARE(tableView->selectionModel()->selectedIndexes().size(), 0);
}

void tst_qquicktableviewdelegate::dragToSelect()
{
    // Check that the delegate is not blocking the user from
    // being able to select cells using Drag.
    LOAD_TABLEVIEW("unmodified.qml");

    // When TableView is not interactive, SelectionRectangle
    // will use Drag by default.
    tableView->setInteractive(false);

    QVERIFY(!tableView->selectionModel()->hasSelection());
    QCOMPARE(tableView->selectionBehavior(), QQuickTableView::SelectCells);

    model->appendColumn();
    model->setData(0, 1, "0,1", Qt::DisplayRole);
    QCOMPARE(model->columnCount(), 2);
    QCOMPARE(model->rowCount(), 1);

    QVERIFY(QQuickTest::qWaitForPolish(tableView));

    // Drag on from cell 0,0 to 0,1
    const auto item0_0 = getDelegateItem(0, 0);
    QVERIFY(item0_0);
    const auto item0_1 = getDelegateItem(0, 1);
    QVERIFY(item0_1);

    QQuickWindow *window = tableView->window();
    const QPoint localPos0_0(item0_0->width() / 2, item0_0->height() / 2);
    const QPoint windowPos0_0 = window->contentItem()->mapFromItem(item0_0, localPos0_0).toPoint();
    const QPoint localPos0_1(item0_1->width() / 2, item0_1->height() / 2);
    const QPoint windowPos0_1 = window->contentItem()->mapFromItem(item0_1, localPos0_1).toPoint();

    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, windowPos0_0);
    QTest::mouseMove(window, windowPos0_1);
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, windowPos0_1);

    QCOMPARE(tableView->selectionModel()->selectedIndexes().size(), 2);
    QVERIFY(item0_0->selected());
    QVERIFY(item0_1->selected());
}

void tst_qquicktableviewdelegate::pressAndHoldToSelect()
{
    // Check that the delegate is not blocking the user from
    // being able to select cells using PressAndHold
    LOAD_TABLEVIEW("unmodified.qml");

    // When TableView is interactive, SelectionRectangle
    // will use PressAndHold by default.
    tableView->setInteractive(true);

    QVERIFY(!tableView->selectionModel()->hasSelection());
    QCOMPARE(tableView->selectionBehavior(), QQuickTableView::SelectCells);

    // PressAndHold on cell 0,0
    const auto item0_0 = getDelegateItem(0, 0);
    QVERIFY(item0_0);

    QQuickWindow *window = tableView->window();
    const QPoint localPos0_0(item0_0->width() / 2, item0_0->height() / 2);
    const QPoint windowPos0_0 = window->contentItem()->mapFromItem(item0_0, localPos0_0).toPoint();

    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, windowPos0_0);
    QTRY_VERIFY(tableView->selectionModel()->hasSelection());
    QCOMPARE(tableView->selectionModel()->selectedIndexes().size(), 1);
    QVERIFY(item0_0->selected());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, windowPos0_0);
}

QQuickTableView *tst_qquicktableviewdelegate::getTableView()
{
    return view->rootObject()->property("tableView").value<QQuickTableView *>();
}

QQuickTableViewDelegate *tst_qquicktableviewdelegate::getDelegateItem(int row, int column)
{
    auto tableView = getTableView();
    if (!tableView)
        return nullptr;
    return qobject_cast<QQuickTableViewDelegate *>(tableView->itemAtCell({column, row}));
}

QTEST_MAIN(tst_qquicktableviewdelegate)

#include "tst_qquicktableviewdelegate.moc"
