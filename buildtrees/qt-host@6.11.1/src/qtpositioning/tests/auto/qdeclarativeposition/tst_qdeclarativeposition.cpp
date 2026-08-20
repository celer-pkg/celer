// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtTest/private/qpropertytesthelper_p.h>
#include <QtPositioningQuick/private/qdeclarativeposition_p.h>

QT_USE_NAMESPACE

class tst_QDeclarativePosition : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void latitudeValidBinding();
    void longitudeValidBinding();
    void altitudeValidBinding();
    void coordinateBinding();
    void timestampBinding();
    void speedBinding();
    void speedValidBinding();
    void horizontalAccuracyValidBinding();
    void horizontalAccuracyBinding();
    void verticalAccuracyValidBinding();
    void verticalAccuracyBinding();
    void directionValidBinding();
    void directionBinding();
    void verticalSpeedValidBinding();
    void verticalSpeedBinding();
    void magneticVariationValidBinding();
    void magneticVariationBinding();
    void directionAccuracyBinding();
    void directionAccuracyValidBinding();

private:
    QDeclarativePosition m_declarativePosition;
    QGeoPositionInfo m_positionInfo;
    std::function<void()> m_mutatorFunc = nullptr;
    std::function<bool(const double &, const double &)> m_doubleComparator = nullptr;
};

void tst_QDeclarativePosition::initTestCase()
{
    m_mutatorFunc = [&]() { m_declarativePosition.setPosition(m_positionInfo); };
    m_doubleComparator = [](const double &lhs, const double &rhs) {
        return (qIsNaN(lhs) && qIsNaN(rhs)) || QtPrivate::fuzzyCompare(lhs, rhs);
    };
}

void tst_QDeclarativePosition::init()
{
    // reset position before each test
    m_declarativePosition.setPosition(QGeoPositionInfo());
}

void tst_QDeclarativePosition::latitudeValidBinding()
{
    QCOMPARE(m_declarativePosition.isLatitudeValid(), false);
    m_positionInfo.setCoordinate(QGeoCoordinate(1.0, 2.0));
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, bool>(
            m_declarativePosition, false, true, "latitudeValid", m_mutatorFunc);
}

void tst_QDeclarativePosition::longitudeValidBinding()
{
    QCOMPARE(m_declarativePosition.isLongitudeValid(), false);
    m_positionInfo.setCoordinate(QGeoCoordinate(1.0, 2.0));
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, bool>(
            m_declarativePosition, false, true, "longitudeValid", m_mutatorFunc);
}

void tst_QDeclarativePosition::altitudeValidBinding()
{
    QCOMPARE(m_declarativePosition.isAltitudeValid(), false);
    m_positionInfo.setCoordinate(QGeoCoordinate(1.0, 2.0, 3.0));
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, bool>(
            m_declarativePosition, false, true, "altitudeValid", m_mutatorFunc);
}

void tst_QDeclarativePosition::coordinateBinding()
{
    QCOMPARE(m_declarativePosition.coordinate(), QGeoCoordinate());
    m_positionInfo.setCoordinate(QGeoCoordinate(1.0, 2.0, 3.0));
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, QGeoCoordinate>(
            m_declarativePosition, QGeoCoordinate(), QGeoCoordinate(1.0, 2.0, 3.0), "coordinate",
            m_mutatorFunc);
}

void tst_QDeclarativePosition::timestampBinding()
{
    QCOMPARE(m_declarativePosition.timestamp(), QDateTime());
    const auto timestamp = QDateTime::currentDateTimeUtc();
    m_positionInfo.setTimestamp(timestamp);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, QDateTime>(
            m_declarativePosition, QDateTime(), timestamp, "timestamp", m_mutatorFunc);
}

void tst_QDeclarativePosition::speedBinding()
{
    QCOMPARE(m_declarativePosition.speed(), qQNaN());
    m_positionInfo.setAttribute(QGeoPositionInfo::GroundSpeed, 10.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, double>(
            m_declarativePosition, qQNaN(), 10.0, "speed", m_mutatorFunc, m_doubleComparator);
}

void tst_QDeclarativePosition::speedValidBinding()
{
    QCOMPARE(m_declarativePosition.isSpeedValid(), false);
    m_positionInfo.setAttribute(QGeoPositionInfo::GroundSpeed, 10.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, bool>(
            m_declarativePosition, false, true, "speedValid", m_mutatorFunc);
}

void tst_QDeclarativePosition::horizontalAccuracyValidBinding()
{
    QCOMPARE(m_declarativePosition.isHorizontalAccuracyValid(), false);
    m_positionInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, 1.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, bool>(
            m_declarativePosition, false, true, "horizontalAccuracyValid", m_mutatorFunc);
}

void tst_QDeclarativePosition::horizontalAccuracyBinding()
{
    QCOMPARE(m_declarativePosition.horizontalAccuracy(), qQNaN());
    m_positionInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, 10.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, double>(
            m_declarativePosition, qQNaN(), 10.0, "horizontalAccuracy", m_mutatorFunc,
            m_doubleComparator);
}

void tst_QDeclarativePosition::verticalAccuracyValidBinding()
{
    QCOMPARE(m_declarativePosition.isVerticalAccuracyValid(), false);
    m_positionInfo.setAttribute(QGeoPositionInfo::VerticalAccuracy, 1.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, bool>(
            m_declarativePosition, false, true, "verticalAccuracyValid", m_mutatorFunc);
}

void tst_QDeclarativePosition::verticalAccuracyBinding()
{
    QCOMPARE(m_declarativePosition.verticalAccuracy(), qQNaN());
    m_positionInfo.setAttribute(QGeoPositionInfo::VerticalAccuracy, 10.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, double>(
            m_declarativePosition, qQNaN(), 10.0, "verticalAccuracy", m_mutatorFunc,
            m_doubleComparator);
}

void tst_QDeclarativePosition::directionValidBinding()
{
    QCOMPARE(m_declarativePosition.isDirectionValid(), false);
    m_positionInfo.setAttribute(QGeoPositionInfo::Direction, 1.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, bool>(
            m_declarativePosition, false, true, "directionValid", m_mutatorFunc);
}

void tst_QDeclarativePosition::directionBinding()
{
    QCOMPARE(m_declarativePosition.direction(), qQNaN());
    m_positionInfo.setAttribute(QGeoPositionInfo::Direction, 10.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, double>(
            m_declarativePosition, qQNaN(), 10.0, "direction", m_mutatorFunc, m_doubleComparator);
}

void tst_QDeclarativePosition::verticalSpeedValidBinding()
{
    QCOMPARE(m_declarativePosition.isVerticalSpeedValid(), false);
    m_positionInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, 1.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, bool>(
            m_declarativePosition, false, true, "verticalSpeedValid", m_mutatorFunc);
}

void tst_QDeclarativePosition::verticalSpeedBinding()
{
    QCOMPARE(m_declarativePosition.verticalSpeed(), qQNaN());
    m_positionInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, 10.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, double>(
            m_declarativePosition, qQNaN(), 10.0, "verticalSpeed", m_mutatorFunc,
            m_doubleComparator);
}

void tst_QDeclarativePosition::magneticVariationValidBinding()
{
    QCOMPARE(m_declarativePosition.isMagneticVariationValid(), false);
    m_positionInfo.setAttribute(QGeoPositionInfo::MagneticVariation, 1.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, bool>(
            m_declarativePosition, false, true, "magneticVariationValid", m_mutatorFunc);
}

void tst_QDeclarativePosition::magneticVariationBinding()
{
    QCOMPARE(m_declarativePosition.magneticVariation(), qQNaN());
    m_positionInfo.setAttribute(QGeoPositionInfo::MagneticVariation, 10.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, double>(
            m_declarativePosition, qQNaN(), 10.0, "magneticVariation", m_mutatorFunc,
                m_doubleComparator);
}

void tst_QDeclarativePosition::directionAccuracyBinding()
{
    QCOMPARE(m_declarativePosition.directionAccuracy(), qQNaN());
    m_positionInfo.setAttribute(QGeoPositionInfo::DirectionAccuracy, 10.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, double>(
            m_declarativePosition, qQNaN(), 10.0, "directionAccuracy", m_mutatorFunc,
            m_doubleComparator);
}

void tst_QDeclarativePosition::directionAccuracyValidBinding()
{
    QCOMPARE(m_declarativePosition.isDirectionAccuracyValid(), false);
    m_positionInfo.setAttribute(QGeoPositionInfo::DirectionAccuracy, 10.0);
    QTestPrivate::testReadOnlyPropertyBasics<QDeclarativePosition, bool>(
            m_declarativePosition, false, true, "directionAccuracyValid", m_mutatorFunc);
}

QTEST_GUILESS_MAIN(tst_QDeclarativePosition)
#include "tst_qdeclarativeposition.moc"
