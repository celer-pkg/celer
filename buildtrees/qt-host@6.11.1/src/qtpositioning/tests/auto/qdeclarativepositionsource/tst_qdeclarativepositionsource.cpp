// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QtTest>
#include <QtTest/private/qpropertytesthelper_p.h>
#include <QtPositioningQuick/private/qdeclarativepositionsource_p.h>

QT_USE_NAMESPACE

class tst_DeclarativePositionSource : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void nameBinding();
    void supportedMethodsBinding();
    void sourceErrorBinding();
    void validBinding();
    void positionBinding();
    void activeBinding();
    void activeBindingAutomated();
    void startBreaksActiveBinding();
    void activeBindingBreak();

    void intervalOnSourceDependency();
    void preferredMethodsOnSourceDependency();

    void updateAfterStart();
    void startAfterUpdate();
    void stopAfterUpdate();
    void startStopAfterUpdate();
    void updateTimedOut();
    void updateWithStartTimedOut();
    void startUpdateStopWithNoIntervals();

private:
    std::unique_ptr<QDeclarativePositionSource> m_positionSource = nullptr;
};

void tst_DeclarativePositionSource::init()
{
    // create a fresh instance of QDeclarativePositionSource before each test
    m_positionSource.reset(new QDeclarativePositionSource);
    m_positionSource->componentComplete(); // simulate QML loading
}

void tst_DeclarativePositionSource::nameBinding()
{
    m_positionSource->setName("test.source");
    QTestPrivate::testReadWritePropertyBasics<QDeclarativePositionSource, QString>(
            *m_positionSource.get(), "invalid source", "test.source", "name");
}

void tst_DeclarativePositionSource::supportedMethodsBinding()
{
    // Invalid source has no positioning methods.
    // "test.source" has all positioning methods.
    m_positionSource->setName("invalid name");
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePositionSource,
                                             QDeclarativePositionSource::PositioningMethods>(
            *m_positionSource.get(), QDeclarativePositionSource::NoPositioningMethods,
            QDeclarativePositionSource::AllPositioningMethods, "supportedPositioningMethods",
            [&]() { m_positionSource->setName("test.source"); });
}

void tst_DeclarativePositionSource::sourceErrorBinding()
{
    // "dummy.source" has a minimum update interval of 100, and calling
    // update() with a smaller timeout immediately result in a timeout error
    m_positionSource->setName("dummy.source");
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePositionSource,
                                             QDeclarativePositionSource::SourceError>(
            *m_positionSource.get(), QDeclarativePositionSource::NoError,
            QDeclarativePositionSource::UpdateTimeoutError, "sourceError",
            [&]() { m_positionSource->update(10); });
    if (QTest::currentTestFailed())
        return;

    // Test that we can't bind sourceError to smth, as it's read-only
    QProperty<QDeclarativePositionSource::SourceError> errorSetter;
    m_positionSource->bindableSourceError().setBinding(Qt::makePropertyBinding(errorSetter));
    QCOMPARE(m_positionSource->bindableSourceError().hasBinding(), false);
}

void tst_DeclarativePositionSource::validBinding()
{
    // Invalid source name results in no position source -> invalid object.
    // Setting the name to "test.source" results in creating a new position
    // source -> valid object
    m_positionSource->setName("invalid name");
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePositionSource, bool>(
            *m_positionSource.get(), false, true, "valid",
            [&]() { m_positionSource->setName("test.source"); });
}

static char *printPosition(const QDeclarativePosition *position)
{
    // For this test we need to print only coordinate, so that we get a nice
    // error message if the below test fails.
    return qstrdup(QDebug::toString(position->coordinate()).toLatin1().constData());
}

void tst_DeclarativePositionSource::positionBinding()
{
    // "test.source" udpates its position, starting from (0, 0) coordinate, and
    // adding 0.1 lat and 0.1 log at every step
    m_positionSource->setName("test.source");
    m_positionSource->setUpdateInterval(200);
    const QGeoCoordinate c1(0.1, 0.1, 0);
    const QGeoCoordinate c2(0.2, 0.2, 0);
    m_positionSource->update();
    QTRY_COMPARE_WITH_TIMEOUT(m_positionSource->position()->coordinate(), c1, 5000);

    QGeoPositionInfo posInfo;
    posInfo.setCoordinate(c1);
    QDeclarativePosition pos1;
    pos1.setPosition(posInfo);

    posInfo.setCoordinate(c2);
    QDeclarativePosition pos2;
    pos2.setPosition(posInfo);

    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePositionSource, QDeclarativePosition *>(
            *m_positionSource.get(), &pos1, &pos2, "position",
            [&]() {
                QSignalSpy spy(m_positionSource.get(),
                               &QDeclarativePositionSource::positionChanged);
                m_positionSource->update();
                // Wait for the update to happen. It should take around 200 ms,
                // but we want to be on a safe side in case of high CI load.
                QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 5000);
            },
            [](const QDeclarativePosition *p1, const QDeclarativePosition *p2) {
                return p1->coordinate() == p2->coordinate();
            },
            printPosition);
    if (QTest::currentTestFailed())
        return;

    QProperty<QDeclarativePosition *> prop(nullptr);
    m_positionSource->bindablePosition().setBinding(Qt::makePropertyBinding(prop));
    // We can't have bindings on read-only properties
    QCOMPARE(m_positionSource->bindablePosition().hasBinding(), false);
}

void tst_DeclarativePositionSource::activeBinding()
{
    m_positionSource->setName("test.source");

    QProperty<bool> activeObserver;
    activeObserver.setBinding(m_positionSource->bindableActive().makeBinding());

    QProperty<bool> activeLambdaObserver;
    activeLambdaObserver.setBinding([&]() { return m_positionSource->isActive(); });

    QSignalSpy activeSpy(m_positionSource.get(), &QDeclarativePositionSource::activeChanged);

    QCOMPARE(activeSpy.size(), 0);
    QCOMPARE(activeObserver.value(), false);
    QCOMPARE(activeLambdaObserver.value(), false);

    QProperty<bool> activeSetter(false);
    m_positionSource->bindableActive().setBinding(Qt::makePropertyBinding(activeSetter));

    activeSetter = true;
    QTest::qWait(0); // to trigger singleShot timer in setActive

    QCOMPARE(activeSpy.size(), 1);
    QCOMPARE(activeObserver.value(), true);
    QCOMPARE(activeLambdaObserver.value(), true);

    activeSetter = false;

    QCOMPARE(activeSpy.size(), 2);
    QCOMPARE(activeObserver.value(), false);
    QCOMPARE(activeLambdaObserver.value(), false);

    // calling update() does not break the binding
    m_positionSource->update(50);
    QCOMPARE(activeSpy.size(), 3);
    QCOMPARE(activeObserver.value(), true);
    QCOMPARE(activeLambdaObserver.value(), true);

    QTRY_COMPARE_WITH_TIMEOUT(m_positionSource->sourceError(),
                              QDeclarativePositionSource::UpdateTimeoutError, 5000);

    QCOMPARE(activeSpy.size(), 4);
    QCOMPARE(activeObserver.value(), false);
    QCOMPARE(activeLambdaObserver.value(), false);

    activeSetter = true;
    QTest::qWait(0); // to trigger singleShot timer in setActive

    QCOMPARE(activeSpy.size(), 5);
    QCOMPARE(activeObserver.value(), true);
    QCOMPARE(activeLambdaObserver.value(), true);

    // calling stop() will break the binding
    m_positionSource->stop();

    QCOMPARE(activeSpy.size(), 6);
    QCOMPARE(m_positionSource->isActive(), false);
    QCOMPARE(activeObserver.value(), false);
    QCOMPARE(activeLambdaObserver.value(), false);

    activeSetter = false;
    activeSetter = true;
    QTest::qWait(0); // to trigger singleShot timer in setActive

    // nothing changed, as the binding is broken
    QCOMPARE(activeSpy.size(), 6);
    QCOMPARE(m_positionSource->isActive(), false);
    QCOMPARE(activeObserver.value(), false);
    QCOMPARE(activeLambdaObserver.value(), false);
}

void tst_DeclarativePositionSource::activeBindingAutomated()
{
    QTestPrivate::testReadWritePropertyBasics(*m_positionSource.get(), true, false, "active");
}

void tst_DeclarativePositionSource::startBreaksActiveBinding()
{
    m_positionSource->setName("test.source");
    QSignalSpy activeSpy(m_positionSource.get(), &QDeclarativePositionSource::activeChanged);

    QProperty<bool> activeSetter(true);
    m_positionSource->bindableActive().setBinding(Qt::makePropertyBinding(activeSetter));

    QTest::qWait(0); // to trigger singleShot timer in setActive

    QCOMPARE(m_positionSource->isActive(), true);
    QCOMPARE(activeSpy.size(), 1);
    QVERIFY(m_positionSource->bindableActive().hasBinding());

    // Call start() explicitly, which should break the binding for 'active'
    // property.
    m_positionSource->start();
    activeSetter = false; // this should have no effect
    QTest::qWait(0); // to trigger singleShot timer in setActive

    QCOMPARE(m_positionSource->isActive(), true);
    QCOMPARE(activeSpy.size(), 1);
    QVERIFY(!m_positionSource->bindableActive().hasBinding());
}

void tst_DeclarativePositionSource::activeBindingBreak()
{
    // This test checks a tricky scenario of breaking the binding for
    // 'active' property.
    // 0. active has a binding to some property Prop.
    // 1. Prop is set to true => active = true => start getting position info.
    // 2. Calling update() for a single update => the binding of active is not
    //    broken.
    // 3. Calling stop() explicitly before the update() call is finished =>
    //    the active binding is supposed to be broken, but the active state
    //    should still be true, because we have a pending update.
    // 4. The pending update finishes => the active is set to false.

    m_positionSource->setName("test.source");

    QProperty<bool> setter(false);
    m_positionSource->bindableActive().setBinding(Qt::makePropertyBinding(setter));
    QVERIFY(m_positionSource->bindableActive().hasBinding());

    setter = true;
    QTest::qWait(0); // to trigger singleShot timer in setActive
    QCOMPARE(m_positionSource->isActive(), true);

    QSignalSpy posSpy(m_positionSource.get(), &QDeclarativePositionSource::positionChanged);
    QSignalSpy errSpy(m_positionSource.get(), &QDeclarativePositionSource::sourceErrorChanged);
    m_positionSource->update(210);

    m_positionSource->stop();
    QVERIFY(!m_positionSource->bindableActive().hasBinding());
    QCOMPARE(m_positionSource->isActive(), true);

    // Wait for the single update to complete. It can be either position update
    // or timeout.
    QTRY_VERIFY_WITH_TIMEOUT(posSpy.size() > 0 || errSpy.size() > 0, 5000);

    QCOMPARE(m_positionSource->isActive(), false);
    QVERIFY(!m_positionSource->bindableActive().hasBinding());
}

void tst_DeclarativePositionSource::intervalOnSourceDependency()
{
    // updateInterval can be modified if the new source does not support that
    // large one, or implements some calculation
    m_positionSource->setName("invalid_source"); // reset the source

    QSignalSpy intervalSpy(m_positionSource.get(),
                           &QDeclarativePositionSource::updateIntervalChanged);

    m_positionSource->setUpdateInterval(100);
    QCOMPARE(m_positionSource->updateInterval(), 100);
    QCOMPARE(intervalSpy.size(), 1);

    // "test.source" has a minimum update interval of 200
    m_positionSource->setName("test.source");
    QCOMPARE(m_positionSource->updateInterval(), 200);
    QCOMPARE(intervalSpy.size(), 2);

    // "dummy.source" has a minimum update interval of 100, so we expect our
    // desired interval to be applied
    m_positionSource->setName("dummy.source");
    QCOMPARE(m_positionSource->updateInterval(), 100);
    QCOMPARE(intervalSpy.size(), 3);
}

void tst_DeclarativePositionSource::preferredMethodsOnSourceDependency()
{
    m_positionSource->setName("invalid_source"); // reset the source

    QSignalSpy methodsSpy(m_positionSource.get(),
                          &QDeclarativePositionSource::preferredPositioningMethodsChanged);

    m_positionSource->setPreferredPositioningMethods(
            QDeclarativePositionSource::SatellitePositioningMethods);
    QCOMPARE(m_positionSource->preferredPositioningMethods(),
             QDeclarativePositionSource::SatellitePositioningMethods);
    QCOMPARE(methodsSpy.size(), 1);

    // "dummy.source" has only non-satellite methods, so they will be used
    m_positionSource->setName("dummy.source");
    QCOMPARE(m_positionSource->preferredPositioningMethods(),
             QDeclarativePositionSource::NonSatellitePositioningMethods);
    QCOMPARE(methodsSpy.size(), 2);

    // "test.source" has all positioning methods, so satellite will be used,
    // as we initially wanted
    m_positionSource->setName("test.source");
    QCOMPARE(m_positionSource->preferredPositioningMethods(),
             QDeclarativePositionSource::SatellitePositioningMethods);
    QCOMPARE(methodsSpy.size(), 3);
}

void tst_DeclarativePositionSource::updateAfterStart()
{
    // When update() is called after start(), it should not invalidate any
    // state. The active state must still be true when the single update() is
    // completed.

    m_positionSource->setName("test.source");

    QCOMPARE(m_positionSource->isActive(), false);

    m_positionSource->start();
    QCOMPARE(m_positionSource->isActive(), true);

    QSignalSpy posSpy(m_positionSource.get(), &QDeclarativePositionSource::positionChanged);
    QSignalSpy errSpy(m_positionSource.get(), &QDeclarativePositionSource::sourceErrorChanged);
    m_positionSource->update(210);
    // Wait for the single update to complete. It can be either position update
    // or timeout.
    QTRY_VERIFY_WITH_TIMEOUT(posSpy.size() > 0 || errSpy.size() > 0, 5000);
    QCOMPARE(m_positionSource->isActive(), true);

    m_positionSource->stop();
    QCOMPARE(m_positionSource->isActive(), false);
}

void tst_DeclarativePositionSource::startAfterUpdate()
{
    // When start() is called after update(), the position source should remain
    // active even when the signle update is completed.

    m_positionSource->setName("test.source");

    QCOMPARE(m_positionSource->isActive(), false);

    QSignalSpy posSpy(m_positionSource.get(), &QDeclarativePositionSource::positionChanged);
    QSignalSpy errSpy(m_positionSource.get(), &QDeclarativePositionSource::sourceErrorChanged);
    m_positionSource->update(210);
    QCOMPARE(m_positionSource->isActive(), true);

    m_positionSource->start();
    // Wait for the single update to complete. It can be either position update
    // or timeout.
    QTRY_VERIFY_WITH_TIMEOUT(posSpy.size() > 0 || errSpy.size() > 0, 5000);
    QCOMPARE(m_positionSource->isActive(), true);

    m_positionSource->stop();
    QCOMPARE(m_positionSource->isActive(), false);
}

void tst_DeclarativePositionSource::stopAfterUpdate()
{
    // When stop() is called after update(), and the update() is still in
    // progress, the position source should remain active until the update()
    // is completed.

    m_positionSource->setName("test.source");

    QCOMPARE(m_positionSource->isActive(), false);

    QSignalSpy posSpy(m_positionSource.get(), &QDeclarativePositionSource::positionChanged);
    QSignalSpy errSpy(m_positionSource.get(), &QDeclarativePositionSource::sourceErrorChanged);
    m_positionSource->update(210);
    QCOMPARE(m_positionSource->isActive(), true);

    m_positionSource->stop();
    QCOMPARE(m_positionSource->isActive(), true);

    // Wait for the single update to complete. It can be either position update
    // or timeout.
    QTRY_VERIFY_WITH_TIMEOUT(posSpy.size() > 0 || errSpy.size() > 0, 5000);
    QCOMPARE(m_positionSource->isActive(), false);
}

void tst_DeclarativePositionSource::startStopAfterUpdate()
{
    // Quite artificial example. Calling start() and stop() after update(),
    // while still waiting for the update() to complete, should still result in
    // the position source to be active until the update() is completed.

    m_positionSource->setName("test.source");

    QCOMPARE(m_positionSource->isActive(), false);

    QSignalSpy posSpy(m_positionSource.get(), &QDeclarativePositionSource::positionChanged);
    QSignalSpy errSpy(m_positionSource.get(), &QDeclarativePositionSource::sourceErrorChanged);
    m_positionSource->update(210);
    QCOMPARE(m_positionSource->isActive(), true);

    m_positionSource->start();
    m_positionSource->stop();

    QCOMPARE(m_positionSource->isActive(), true);

    // Wait for the single update to complete. It can be either position update
    // or timeout.
    QTRY_VERIFY_WITH_TIMEOUT(posSpy.size() > 0 || errSpy.size() > 0, 5000);
    QCOMPARE(m_positionSource->isActive(), false);
}

void tst_DeclarativePositionSource::updateTimedOut()
{
    // This test checks that we reset to inactive state when the single update()
    // request times out without providing the position info

    m_positionSource->setName("test.source");

    QCOMPARE(m_positionSource->isActive(), false);

    m_positionSource->update(50); // too small timeout -> will return an error
    QCOMPARE(m_positionSource->isActive(), true);

    QTRY_COMPARE_WITH_TIMEOUT(m_positionSource->sourceError(),
                              QDeclarativePositionSource::UpdateTimeoutError, 5000);
    QCOMPARE(m_positionSource->isActive(), false);
}

void tst_DeclarativePositionSource::updateWithStartTimedOut()
{
    // This test checks that if single update() times out, but the regular
    // updates are running, we still remain in active state.

    m_positionSource->setName("test.source");

    QCOMPARE(m_positionSource->isActive(), false);

    m_positionSource->start();

    m_positionSource->update(50); // too small timeout -> will return an error
    QCOMPARE(m_positionSource->isActive(), true);

    QTRY_COMPARE_WITH_TIMEOUT(m_positionSource->sourceError(),
                              QDeclarativePositionSource::UpdateTimeoutError, 5000);
    QCOMPARE(m_positionSource->isActive(), true);

    m_positionSource->stop();
    QCOMPARE(m_positionSource->isActive(), false);
}

void tst_DeclarativePositionSource::startUpdateStopWithNoIntervals()
{
    // This test checks that a sequence of calls start() -> update() -> stop()
    // without any waits between them will result in expected behavior.
    // Specifically, the position source should remain active until it gets
    // the position response.

    m_positionSource->setName("test.source");

    QCOMPARE(m_positionSource->isActive(), false);

    m_positionSource->start();
    QSignalSpy posSpy(m_positionSource.get(), &QDeclarativePositionSource::positionChanged);
    QSignalSpy errSpy(m_positionSource.get(), &QDeclarativePositionSource::sourceErrorChanged);
    m_positionSource->update(210);
    QCOMPARE(m_positionSource->isActive(), true);
    m_positionSource->stop();
    QCOMPARE(m_positionSource->isActive(), true);

    // Wait for the single update to complete. It can be either position update
    // or timeout.
    QTRY_VERIFY_WITH_TIMEOUT(posSpy.size() > 0 || errSpy.size() > 0, 5000);
    QCOMPARE(m_positionSource->isActive(), false);
}

QTEST_MAIN(tst_DeclarativePositionSource)

#include "tst_qdeclarativepositionsource.moc"
