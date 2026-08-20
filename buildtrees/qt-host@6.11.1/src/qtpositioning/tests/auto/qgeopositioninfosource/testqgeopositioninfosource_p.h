// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTQGEOPOSITIONINFOSOURCE_P_H
#define TESTQGEOPOSITIONINFOSOURCE_P_H

#ifdef TST_GEOCLUEMOCK_ENABLED
#include "geocluemock.h"
#include <QThread>
#endif

#include <QTest>
#include <QtTest/private/qpropertytesthelper_p.h>
#include <QObject>

QT_BEGIN_NAMESPACE
class QGeoPositionInfoSource;
QT_END_NAMESPACE

class TestQGeoPositionInfoSource : public QObject
{
    Q_OBJECT

public:
    TestQGeoPositionInfoSource(QObject *parent = 0);

    static TestQGeoPositionInfoSource *createDefaultSourceTest();

public slots:
    void test_slot1();
    void test_slot2();

protected:
    virtual QGeoPositionInfoSource *createTestSource() = 0;

    // MUST be called by subclasses if they override respective test slots
    void base_initTestCase();
    void base_init();
    void base_cleanup();
    void base_cleanupTestCase();

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void constructor_withParent();

    void constructor_noParent();

    void updateInterval();

    void setPreferredPositioningMethods();
    void setPreferredPositioningMethods_data();

    void preferredPositioningMethods();

    void createDefaultSource();

    void setUpdateInterval();
    void setUpdateInterval_data();

    void lastKnownPosition();
    void lastKnownPosition_data();

    void minimumUpdateInterval();

    void startUpdates_testIntervals();
    void startUpdates_testIntervalChangesWhileRunning();
    void startUpdates_testDefaultInterval();
    void startUpdates_testZeroInterval();
    void startUpdates_moreThanOnce();

    void stopUpdates();
    void stopUpdates_withoutStart();

    void requestUpdate();
    void requestUpdate_data();

    void requestUpdate_validTimeout();
    void requestUpdate_defaultTimeout();
    void requestUpdate_timeoutLessThanMinimumInterval();
    void requestUpdate_repeatedCalls();
    void requestUpdate_overlappingCalls();

    void requestUpdateAfterStartUpdates_ZeroInterval();
    void requestUpdateAfterStartUpdates_SmallInterval();
    void requestUpdateBeforeStartUpdates_ZeroInterval();
    void requestUpdateBeforeStartUpdates_SmallInterval();

    void removeSlotForRequestTimeout();
    void removeSlotForPositionUpdated();

    void updateIntervalBinding();
    void preferredMethodsBinding();

private:
    QGeoPositionInfoSource *m_source;
    bool m_testingDefaultSource;
    bool m_testSlot2Called;
};

#endif
