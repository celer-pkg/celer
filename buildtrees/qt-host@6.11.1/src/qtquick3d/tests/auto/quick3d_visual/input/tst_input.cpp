// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QSignalSpy>
#include <QTest>
#include <QtGui/QPointingDevice>
#include <QtQuick/QQuickView>
#include <QtQuick/private/qquickdraghandler_p.h>
#include <QtQuick/private/qquickmousearea_p.h>
#include <QtQuick/private/qquicktaphandler_p.h>
#include <QtQuick3D/private/qquick3ditem2d_p.h>
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQml/private/qqmlglobal_p.h>

#include "../shared/util.h"

Q_LOGGING_CATEGORY(lcTests, "qt.quick3d.tests")

// On one hand, uncommenting this will make troubleshooting easier (avoid the 60FPS hover events).
// On the other hand, if anything actually breaks when hover events are enabled, that's also a bug.
//#define DISABLE_HOVER_IN_IRRELEVANT_TESTS

class tst_Input : public QQuick3DDataTest
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void singleTap2D_data();
    void singleTap2D();
    void dualTouchTap2D_data();
    void dualTouchTap2D();
    void fallthrough_data();
    void fallthrough();
    void singleDrag2D_data();
    void singleDrag2D();

private:
    QQuickItem *find2DChildIn3DNode(QQuickView *view, const QString &objectName, const QString &itemOrHandlerName);

    QScopedPointer<QPointingDevice> touchscreen = QScopedPointer<QPointingDevice>(QTest::createTouchDevice());
};

void tst_Input::initTestCase()
{
    // Overrides the internal input handling in QQuick3DViewport so that it's
    // always true instead of conditionally based on if there are item2Ds in the scene.
    qputenv("QT_QUICK3D_FORCE_INPUT_HANDLING", "1");

    QQuick3DDataTest::initTestCase();
    if (!initialized())
        return;

    // Move the system cursor out of the way of test
    // If this isn't possible, don't bother running the test

    const auto *screen = QGuiApplication::primaryScreen();
    const QPoint topLeft = screen ? screen->geometry().topLeft() : QPoint();

    QCursor::setPos(topLeft);
    if (QCursor::pos() != topLeft)
        QSKIP("It's not possible to move the system cursor, possible test instability");
}

QQuickItem *tst_Input::find2DChildIn3DNode(QQuickView *view, const QString &objectName, const QString &itemOrHandlerName)
{
    QQuick3DNode *obj = view->rootObject()->findChild<QQuick3DNode*>(objectName);
    QQuickItem *subsceneRoot = obj->findChild<QQuickItem *>();
    if (!subsceneRoot)
        subsceneRoot = obj->findChild<QQuick3DItem2D *>()->contentItem();
    QObject *child = subsceneRoot->findChild<QObject *>(itemOrHandlerName);
    if (!child) {
        qCWarning(lcTests) << "failed to find" << itemOrHandlerName << "in" << subsceneRoot << subsceneRoot->findChildren<QObject*>();
        return nullptr;
    }
    qCDebug(lcTests) << "found" << child << "in" << subsceneRoot << "in" << obj;
    auto handler = qmlobject_cast<QQuickPointerHandler *>(child);
    return (handler ? handler->parentItem() : static_cast<QQuickItem*>(child));
}

void tst_Input::singleTap2D_data()
{
    QTest::addColumn<QString>("qmlSource");
    QTest::addColumn<QString>("objectName");
    QTest::addColumn<QString>("tapObjectName");
    QTest::addColumn<QPointingDevice::DeviceType>("deviceType");
    QTest::addColumn<QPoint>("overridePos");

    QTest::newRow("item2d left mousearea: mouse") << "item2d.qml" << "left object" << "left busybox button mousearea"
        << QPointingDevice::DeviceType::Mouse << QPoint(250, 250);
    QTest::newRow("item2d left taphandler: mouse") << "item2d.qml" << "left object" << "left busybox upper TapHandler"
        << QPointingDevice::DeviceType::Mouse << QPoint(100, 70);
    QTest::newRow("item2d right mousearea: mouse") << "item2d.qml" << "right object" << "right busybox button mousearea"
        << QPointingDevice::DeviceType::Mouse << QPoint(650, 250);
    QTest::newRow("shared source mousearea: mouse") << "sharedSource.qml" << "left object" << "shared busybox button mousearea"
        << QPointingDevice::DeviceType::Mouse << QPoint(150, 225);
    QTest::newRow("material mousearea: mouse") << "defaultMaterial.qml" << "left object" << "left busybox button mousearea"
        << QPointingDevice::DeviceType::Mouse << QPoint(100, 250);

    QTest::newRow("item2d left mousearea: touch") << "item2d.qml" << "left object" << "left busybox button mousearea"
        << QPointingDevice::DeviceType::TouchScreen << QPoint(250, 250);
    QTest::newRow("item2d left taphandler: touch") << "item2d.qml" << "left object" << "left busybox upper TapHandler"
        << QPointingDevice::DeviceType::TouchScreen << QPoint(100, 70);
    QTest::newRow("item2d right taphandler: touch") << "item2d.qml" << "right object" << "right busybox upper TapHandler"
        << QPointingDevice::DeviceType::TouchScreen << QPoint(650, 100);
    QTest::newRow("shared source mousearea: touch") << "sharedSource.qml" << "left object" << "shared busybox button mousearea"
        << QPointingDevice::DeviceType::TouchScreen << QPoint(100, 250);
    QTest::newRow("material mousearea: touch") << "defaultMaterial.qml" << "left object" << "left busybox button mousearea"
        << QPointingDevice::DeviceType::TouchScreen << QPoint(100, 250);
}

void tst_Input::singleTap2D()
{
    QFETCH(QString, qmlSource);
    QFETCH(QString, objectName);
    QFETCH(QString, tapObjectName);
    QFETCH(QPoint, overridePos);
    QFETCH(QPointingDevice::DeviceType, deviceType);

    QScopedPointer<QQuickView> view(createView(qmlSource, QSize(1024, 480)));
    QVERIFY(view);
#ifdef DISABLE_HOVER_IN_IRRELEVANT_TESTS
    QQuickWindowPrivate::get(view.data())->deliveryAgentPrivate()->frameSynchronousHoverEnabled = false;
#endif
    QVERIFY(QTest::qWaitForWindowActive(view.data()));

    QQuickItem *tapItem = find2DChildIn3DNode(view.data(), objectName, tapObjectName);
    QVERIFY(tapItem);

    // tapItem1->mapToItem(view->contentItem(), mouseArea->boundingRect().center()).toPoint()
    // would generate subscene coordinates; transform to the outer window scene is ambiguous,
    // because actually the button can exist on multiple faces of the 3D object.
    auto tapPos = tapItem->mapToScene(tapItem->boundingRect().center()).toPoint();
    qCDebug(lcTests) << "found destination for a tap:" << tapPos << "in" << tapItem;
    if (!overridePos.isNull())
        tapPos = overridePos; // TODO remove when there's a good mapping technique

    switch (static_cast<QPointingDevice::DeviceType>(deviceType)) {
    case QPointingDevice::DeviceType::Mouse:
        QTest::mouseClick(view.data(), Qt::LeftButton, Qt::NoModifier, tapPos);
        break;
    case QPointingDevice::DeviceType::TouchScreen:
        QTest::touchEvent(view.data(), touchscreen.data()).press(0, tapPos, view.data());
        QTRY_VERIFY(tapItem->property("pressed").toBool());
        QTest::touchEvent(view.data(), touchscreen.data()).release(0, tapPos, view.data());
        break;
    default:
        break;
    }
    QTRY_COMPARE(tapItem->property("clickedCount").toInt(), 1);

    QPoint pressPos = tapItem->property("lastPressPos").toPoint();
    QPoint releasePos = tapItem->property("lastReleasePos").toPoint();
    qCDebug(lcTests) << "pressed @" << pressPos << "released @" << releasePos;
    QCOMPARE(releasePos, pressPos);
    // TODO QCOMPARE(pressPos, tapItem->boundingRect().center());
}

void tst_Input::dualTouchTap2D_data()
{
    QTest::addColumn<QString>("qmlSource");
    QTest::addColumn<QString>("objectName1");
    QTest::addColumn<QString>("objectName2");
    QTest::addColumn<QString>("tapObjectName1");
    QTest::addColumn<QString>("tapObjectName2");
    QTest::addColumn<QPoint>("overridePos1");
    QTest::addColumn<QPoint>("overridePos2");

    QTest::newRow("item2d: two MouseAreas") << "item2d.qml" << "left object" << "right object"
        << "left busybox button mousearea" << "right busybox button mousearea" << QPoint(250, 250) << QPoint(650, 250);
    QTest::newRow("texture source: MouseArea and TapHandler") << "sharedSource.qml" << "left object" << "left object"
        << "shared busybox upper TapHandler" << "shared busybox button mousearea" << QPoint(150, 225) << QPoint(150, 130);
    // doesn't pass: QTBUG-96324
//    QTest::newRow("shared source: MouseArea and TapHandler") << "sharedSource.qml" << "left object" << "left object"
//        << "shared busybox upper TapHandler" << "shared busybox button mousearea" << QPoint(150, 130) << QPoint(800, 175);
    QTest::newRow("material: two MouseAreas") << "defaultMaterial.qml" << "left object" << "right object"
        << "left busybox button mousearea" << "right busybox button mousearea" << QPoint(100, 250) << QPoint(800, 325);
}

void tst_Input::dualTouchTap2D()
{
    QFETCH(QString, qmlSource);
    QFETCH(QString, objectName1);
    QFETCH(QString, objectName2);
    QFETCH(QString, tapObjectName1);
    QFETCH(QString, tapObjectName2);
    QFETCH(QPoint, overridePos1);
    QFETCH(QPoint, overridePos2);

    QScopedPointer<QQuickView> view(createView(qmlSource, QSize(1024, 480)));
    QVERIFY(view);
#ifdef DISABLE_HOVER_IN_IRRELEVANT_TESTS
    QQuickWindowPrivate::get(view.data())->deliveryAgentPrivate()->frameSynchronousHoverEnabled = false;
#endif
    QVERIFY(QTest::qWaitForWindowActive(view.data()));

    QQuickItem *tapItem1 = find2DChildIn3DNode(view.data(), objectName1, tapObjectName1);
    QVERIFY(tapItem1);

    QQuickItem *tapItem2 = find2DChildIn3DNode(view.data(), objectName2, tapObjectName2);
    QVERIFY(tapItem2);

    // tapItem1->mapToItem(view->contentItem(), mouseArea->boundingRect().center()).toPoint()
    // would generate subscene coordinates; transform to the outer window scene is ambiguous,
    // because actually the button can exist on multiple faces of the 3D object.
    auto tapPos1 = tapItem1->mapToScene(tapItem1->boundingRect().center()).toPoint();
    qCDebug(lcTests) << "found destination 1 for a tap:" << tapPos1 << "in" << tapItem1;
    if (!overridePos1.isNull())
        tapPos1 = overridePos1; // TODO remove when there's a good mapping technique
    qCDebug(lcTests) << "overridden:" << tapPos1;

    auto tapPos2 = tapItem2->mapToScene(tapItem2->boundingRect().center()).toPoint();
    qCDebug(lcTests) << "found destination 2 for a tap:" << tapPos2 << "in" << tapItem2;
    if (!overridePos2.isNull())
        tapPos2 = overridePos2; // TODO remove when there's a good mapping technique
    qCDebug(lcTests) << "overridden:" << tapPos2;

    QTest::touchEvent(view.data(), touchscreen.data()).press(1, tapPos1, view.data()).press(2, tapPos2, view.data());
    QTRY_VERIFY(tapItem1->property("pressed").toBool());
    QTRY_VERIFY(tapItem2->property("pressed").toBool());
    QTest::touchEvent(view.data(), touchscreen.data()).release(1, tapPos1, view.data()).release(2, tapPos2, view.data());

    QTRY_COMPARE(tapItem1->property("pressed").toBool(), false);
    QTRY_COMPARE(tapItem2->property("pressed").toBool(), false);
    QCOMPARE(tapItem1->property("clickedCount").toInt(), 1);
    QCOMPARE(tapItem2->property("clickedCount").toInt(), 1);

    QPoint pressPos = tapItem1->property("lastPressPos").toPoint();
    QPoint releasePos = tapItem1->property("lastReleasePos").toPoint();
    qCDebug(lcTests) << tapItem1->objectName() << "pressed @" << pressPos << "released @" << releasePos;
    QCOMPARE(releasePos, pressPos);
    // TODO QCOMPARE(pressPos, tapItem1->boundingRect().center());

    pressPos = tapItem2->property("lastPressPos").toPoint();
    releasePos = tapItem2->property("lastReleasePos").toPoint();
    qCDebug(lcTests) << tapItem2->objectName() << "pressed @" << pressPos << "released @" << releasePos;
    QCOMPARE(releasePos, pressPos);
    // TODO QCOMPARE(pressPos, tapItem1->boundingRect().center());
}

void tst_Input::fallthrough_data()
{
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<bool>("mouseAreaCanBlock");

    QTest::newRow("click on background") << QPoint(900, 240) << false;
    QTest::newRow("item2d") << QPoint(240, 240) << true;
    QTest::newRow("model without 2D subscene") << QPoint(512, 240) << false;
}

void tst_Input::fallthrough()
{
    QFETCH(QPoint, pos);
    QFETCH(bool, mouseAreaCanBlock);

    QScopedPointer<QQuickView> view(createView(QLatin1String("interactiveContentUnder.qml"), QSize(1024, 480)));
    QVERIFY(view);
#ifdef DISABLE_HOVER_IN_IRRELEVANT_TESTS
    QQuickWindowPrivate::get(view.data())->deliveryAgentPrivate()->frameSynchronousHoverEnabled = false;
#endif
    QVERIFY(QTest::qWaitForWindowActive(view.data()));
    QQuickTapHandler *tapHandlerUnderneath = view->contentItem()->findChild<QQuickTapHandler *>();
    QVERIFY(tapHandlerUnderneath);
    QSignalSpy pressedChangedSpy(tapHandlerUnderneath, SIGNAL(pressedChanged()));

    QTest::mouseClick(view.data(), Qt::LeftButton, Qt::NoModifier, pos);
    QTRY_COMPARE(pressedChangedSpy.size(), 2);

    if (mouseAreaCanBlock) {
        pressedChangedSpy.clear();
        QQuickMouseArea *mouseArea = qmlobject_cast<QQuickMouseArea *>(find2DChildIn3DNode(view.data(), "left object", "left mousearea"));
        QVERIFY(mouseArea);
        QSignalSpy maPressedChangedSpy(mouseArea, SIGNAL(pressedChanged()));
        mouseArea->setEnabled(true); // accepts the mouse events
        QTest::mouseClick(view.data(), Qt::LeftButton, Qt::NoModifier, pos);
        QTRY_COMPARE(maPressedChangedSpy.size(), 2);
        QCOMPARE(pressedChangedSpy.size(), 0);
    }
}

void tst_Input::singleDrag2D_data()
{
    QTest::addColumn<QString>("qmlSource");
    QTest::addColumn<QString>("objectName");
    QTest::addColumn<QString>("dragObjectName");
    QTest::addColumn<QPointingDevice::DeviceType>("deviceType");
    QTest::addColumn<QPoint>("overridePos");

    QTest::newRow("item2d left slider: mouse") << "item2d.qml" << "left object" << "left busybox slider draghandler"
        << QPointingDevice::DeviceType::Mouse << QPoint(400, 270);
    QTest::newRow("shared left slider: mouse") << "sharedSource.qml" << "left object" << "shared busybox slider draghandler"
        << QPointingDevice::DeviceType::Mouse << QPoint(250, 270);
    QTest::newRow("material left slider: mouse") << "defaultMaterial.qml" << "left object" << "left busybox slider draghandler"
        << QPointingDevice::DeviceType::Mouse << QPoint(250, 270);

    QTest::newRow("item2d left slider: touch") << "item2d.qml" << "left object" << "left busybox slider draghandler"
        << QPointingDevice::DeviceType::TouchScreen << QPoint(400, 270);
    QTest::newRow("shared left slider: touch") << "sharedSource.qml" << "left object" << "shared busybox slider draghandler"
        << QPointingDevice::DeviceType::TouchScreen << QPoint(250, 270);
    QTest::newRow("material left slider: touch") << "defaultMaterial.qml" << "left object" << "left busybox slider draghandler"
        << QPointingDevice::DeviceType::TouchScreen << QPoint(250, 270);
}

void tst_Input::singleDrag2D()
{
    QFETCH(QString, qmlSource);
    QFETCH(QString, objectName);
    QFETCH(QString, dragObjectName);
    QFETCH(QPoint, overridePos);
    QFETCH(QPointingDevice::DeviceType, deviceType);

    QScopedPointer<QQuickView> view(createView(qmlSource, QSize(1024, 480)));
    QVERIFY(view);
#ifdef DISABLE_HOVER_IN_IRRELEVANT_TESTS
    QQuickWindowPrivate::get(view.data())->deliveryAgentPrivate()->frameSynchronousHoverEnabled = false;
#endif
    QVERIFY(QTest::qWaitForWindowActive(view.data()));

    QQuickItem *dragItem = find2DChildIn3DNode(view.data(), objectName, dragObjectName);
    QVERIFY(dragItem);
    QQuickDragHandler *dragHandler = dragItem->findChild<QQuickDragHandler *>();
    QVERIFY(dragHandler);
    QCOMPARE(dragItem->property("value").toInt(), 50); // initial value

    // dragItem1->mapToItem(view->contentItem(), mouseArea->boundingRect().center()).toPoint()
    // would generate subscene coordinates; transform to the outer window scene is ambiguous,
    // because actually the slider can exist on multiple faces of the 3D object.
    auto dragPos = dragItem->mapToScene(dragItem->boundingRect().center()).toPoint();
    qCDebug(lcTests) << "found destination for drag begin:" << dragPos << ": center of" << dragItem->boundingRect() << dragItem->position() << "in" << dragItem;
    if (!overridePos.isNull())
        dragPos = overridePos; // TODO remove when there's a good mapping technique

    switch (static_cast<QPointingDevice::DeviceType>(deviceType)) {
    case QPointingDevice::DeviceType::Mouse:
        QTest::mousePress(view.data(), Qt::LeftButton, Qt::NoModifier, dragPos);
        dragPos += QPoint(0, 50);
        QTest::mouseMove(view.data(), dragPos);
        QTRY_VERIFY(dragHandler->active());
        qCDebug(lcTests) << "slider dragged from 50 to" << dragItem->property("value").toInt();
        QTest::mouseRelease(view.data(), Qt::LeftButton, Qt::NoModifier, dragPos);
        break;
    case QPointingDevice::DeviceType::TouchScreen:
        QTest::touchEvent(view.data(), touchscreen.data()).press(0, dragPos, view.data());
        dragPos += QPoint(0, 50);
        QTest::touchEvent(view.data(), touchscreen.data()).move(0, dragPos, view.data());
        QTRY_VERIFY(dragHandler->active());
        qCDebug(lcTests) << "slider dragged from 50 to" << dragItem->property("value").toInt();
        QTest::touchEvent(view.data(), touchscreen.data()).release(0, dragPos, view.data());
        break;
    default:
        break;
    }
    QCOMPARE_LT(dragItem->property("value").toInt(), 25); // 16 is likely in practice
}
QTEST_MAIN(tst_Input)
#include "tst_input.moc"
