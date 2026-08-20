// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTQGEOSATELLITEINFOSOURCE_H
#define TESTQGEOSATELLITEINFOSOURCE_H

#include <QObject>
#include <QtTest/QtTest>
#include <QtTest/private/qpropertytesthelper_p.h>

QT_BEGIN_NAMESPACE
class QGeoSatelliteInfoSource;
QT_END_NAMESPACE


class TestQGeoSatelliteInfoSource : public QObject
{
    Q_OBJECT

public:
    TestQGeoSatelliteInfoSource(QObject *parent = 0);

    static TestQGeoSatelliteInfoSource *createDefaultSourceTest();

protected:
    virtual QGeoSatelliteInfoSource *createTestSource() = 0;

    // MUST be called by subclasses if they override respective test slots
    void base_initTestCase();
    void base_init();
    void base_cleanup();
    void base_cleanupTestCase();

public slots:
    void test_slot1();
    void test_slot2();

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void constructor_withParent();
    void constructor_noParent();

    void updateInterval();

    void setUpdateInterval();
    void setUpdateInterval_data();

    void minimumUpdateInterval();

    void createDefaultSource();
    void createDefaultSource_noParent();

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
    void requestUpdate_overlappingCallsWithTimeout();

    void requestUpdateAfterStartUpdates_ZeroInterval();
    void requestUpdateAfterStartUpdates_SmallInterval();
    void requestUpdateBeforeStartUpdates_ZeroInterval();
    void requestUpdateBeforeStartUpdates_SmallInterval();

    void removeSlotForRequestTimeout();
    void removeSlotForSatellitesInUseUpdated();
    void removeSlotForSatellitesInViewUpdated();

    void bindings();

private:
    QGeoSatelliteInfoSource *m_source;
    bool m_testingDefaultSource;
    bool m_testSlot2Called;
};

#endif // #ifndef TESTQGEOSATELLITEINFOSOURCE_H
