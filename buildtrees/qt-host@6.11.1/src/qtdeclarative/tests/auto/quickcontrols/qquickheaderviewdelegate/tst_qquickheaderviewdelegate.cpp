// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQuick/qquickview.h>
#include <QtQuickTemplates2/private/qquickheaderview_p.h>
#include <QtQuickTemplates2/private/qquickheaderviewdelegate_p.h>

#include <QtQuickTest/quicktest.h>

#include <QtQuickTestUtils/private/viewtestutils_p.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>

#include <QtTest/QTest>

using namespace QQuickViewTestUtils;
using namespace QQuickVisualTestUtils;

// ########################################################

class tst_qquickheaderviewdelegate : public QQmlDataTest
{
    Q_OBJECT

public:
    tst_qquickheaderviewdelegate();

private:
    void loadHeaderView(Qt::Orientation orientation);
    QQuickHeaderViewBase* getHeaderView();
    QQuickHeaderViewDelegate* getDelegate(int row, int column);
    template<typename T>
    T getProperty(QQuickHeaderViewDelegate* delegate, const char* property);

private slots:
    void initTestCase() override;
    void defaults_data();
    void defaults();
    void checkProperties_data();
    void checkProperties();

private:
    std::unique_ptr<QQuickView> view = nullptr;
};

tst_qquickheaderviewdelegate::tst_qquickheaderviewdelegate()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
}

void tst_qquickheaderviewdelegate::loadHeaderView(Qt::Orientation orientation)
{
    switch (orientation) {
    case Qt::Horizontal:
        view->setSource(testFileUrl("HorizontalHeaderView.qml"));
        break;
    case Qt::Vertical:
        view->setSource(testFileUrl("VerticalHeaderView.qml"));
        break;
    }

    view->show();
}

QQuickHeaderViewBase* tst_qquickheaderviewdelegate::getHeaderView()
{
    return view->rootObject()->property("headerView").value<QQuickHeaderViewBase*>();
}

QQuickHeaderViewDelegate* tst_qquickheaderviewdelegate::getDelegate(int row, int column)
{
    QQuickHeaderViewBase* headerView = getHeaderView();
    QQuickItem* item = headerView->itemAtCell({column, row});
    return qobject_cast<QQuickHeaderViewDelegate*>(item);
}

template<typename T>
T tst_qquickheaderviewdelegate::getProperty(QQuickHeaderViewDelegate* delegate,
                                            const char* property)
{
    return delegate->property(property).value<T>();
}

void tst_qquickheaderviewdelegate::initTestCase()
{
    QQmlDataTest::initTestCase();
    view.reset(createView());
}

void tst_qquickheaderviewdelegate::defaults_data()
{
    QTest::addColumn<Qt::Orientation>("orientation");
    QTest::newRow("Horizontal") << Qt::Horizontal;
    QTest::newRow("Vertical") << Qt::Vertical;
}

void tst_qquickheaderviewdelegate::defaults()
{
    QFETCH(Qt::Orientation, orientation);

    QTest::failOnWarning(QRegularExpression(".*"));
    loadHeaderView(orientation);
}

void tst_qquickheaderviewdelegate::checkProperties_data()
{
    defaults_data();
}

void tst_qquickheaderviewdelegate::checkProperties()
{
    QFETCH(Qt::Orientation, orientation);

    loadHeaderView(orientation);
    QVERIFY(QTest::qWaitForWindowActive(view.get()));

    QQuickHeaderViewBase* headerView = getHeaderView();
    QVERIFY(headerView);

    QQuickHeaderViewDelegate* delegate = getDelegate(0, 0);
    QVERIFY(delegate);

    const auto delegateHeaderView =
            getProperty<QQuickHeaderViewBase*>(delegate, "headerView");
    QCOMPARE(delegateHeaderView, headerView);

    const auto delegateOrientation =
            getProperty<Qt::Orientation>(delegate, "orientation");
    QCOMPARE(delegateOrientation, orientation);
}

QTEST_MAIN(tst_qquickheaderviewdelegate)

#include "tst_qquickheaderviewdelegate.moc"
