// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtPositioning/QGeoPositionInfo>
#include <QTest>

static const QGeoCoordinate coordinate(1.0, 2.0);
static const QDateTime dateTime = QDateTime::currentDateTimeUtc();

class tst_QGeoPositionInfoBenchmark : public QObject
{
    Q_OBJECT
private slots:
    void constructDefault();
    void constructWithParameters();
    void constructCopy();
    void constructMove();

    void assign();
    void assignMove();

    void checkEquality();

    void isValid();

    void setTimestamp();
    void queryTimestamp();

    void setCoordinate();
    void queryCoordinate();

    void setAttribute();
    void queryAttributeExisting();
    void queryAttributeNonExisting();
    void removeAttributeExisting();
    void removeAttributeNonExisting();
    void hasAttributeExisting();
    void hasAttributeNonExisting();
};

void tst_QGeoPositionInfoBenchmark::constructDefault()
{
    QBENCHMARK {
        QGeoPositionInfo info;
        Q_UNUSED(info)
    }
}

void tst_QGeoPositionInfoBenchmark::constructWithParameters()
{
    QBENCHMARK {
        QGeoPositionInfo info(coordinate, dateTime);
        Q_UNUSED(info)
    }
}

void tst_QGeoPositionInfoBenchmark::constructCopy()
{
    const QGeoPositionInfo info(coordinate, dateTime);
    QBENCHMARK {
        QGeoPositionInfo newInfo(info);
        Q_UNUSED(newInfo)
    }
}

void tst_QGeoPositionInfoBenchmark::constructMove()
{
    QBENCHMARK {
        // We need to create a new object at each iteration, so that we don't
        // end up moving an already moved-from object. So the real value for
        // move can be calculated using the results of constructDefault()
        // benchmark.
        QGeoPositionInfo info;
        QGeoPositionInfo newInfo(std::move(info));
        Q_UNUSED(newInfo)
    }
}

void tst_QGeoPositionInfoBenchmark::assign()
{
    const QGeoPositionInfo info(coordinate, dateTime);
    QGeoPositionInfo newInfo;
    QBENCHMARK {
        newInfo = info;
    }
}

void tst_QGeoPositionInfoBenchmark::assignMove()
{
    QGeoPositionInfo newInfo;
    QBENCHMARK {
        // We need to create a new object at each iteration, so that we don't
        // end up moving an already moved-from object. So the real value for
        // move can be calculated using the results of constructDefault()
        // benchmark.
        QGeoPositionInfo info;
        newInfo = std::move(info);
    }
}

void tst_QGeoPositionInfoBenchmark::checkEquality()
{
    // We will benchmark equal objects, because unequal objects will normally
    // take less time to compare (as the comparison will fail at some stage).
    const QGeoPositionInfo info1(coordinate, dateTime);
    const QGeoPositionInfo info2(coordinate, dateTime);
    QBENCHMARK {
        const bool equal = info1 == info2;
        Q_UNUSED(equal)
    }
}

void tst_QGeoPositionInfoBenchmark::isValid()
{
    const QGeoPositionInfo info(coordinate, dateTime);
    QBENCHMARK {
        const bool valid = info.isValid();
        Q_UNUSED(valid)
    }
}

void tst_QGeoPositionInfoBenchmark::setTimestamp()
{
    QGeoPositionInfo info;
    QBENCHMARK {
        info.setTimestamp(dateTime);
    }
}

void tst_QGeoPositionInfoBenchmark::queryTimestamp()
{
    const QGeoPositionInfo info(coordinate, dateTime);
    QBENCHMARK {
        const auto dt = info.timestamp();
        Q_UNUSED(dt)
    }
}

void tst_QGeoPositionInfoBenchmark::setCoordinate()
{
    QGeoPositionInfo info;
    QBENCHMARK {
        info.setCoordinate(coordinate);
    }
}

void tst_QGeoPositionInfoBenchmark::queryCoordinate()
{
    const QGeoPositionInfo info(coordinate, dateTime);
    QBENCHMARK {
        const auto coord = info.coordinate();
        Q_UNUSED(coord)
    }
}

void tst_QGeoPositionInfoBenchmark::setAttribute()
{
    QGeoPositionInfo info;
    QBENCHMARK {
        info.setAttribute(QGeoPositionInfo::Direction, 1.0);
    }
}

static QGeoPositionInfo generateInfoWithAttributes()
{
    QGeoPositionInfo info;
    info.setAttribute(QGeoPositionInfo::Direction, 1.0);
    info.setAttribute(QGeoPositionInfo::GroundSpeed, 2.0);
    info.setAttribute(QGeoPositionInfo::VerticalSpeed, 3.0);
    info.setAttribute(QGeoPositionInfo::MagneticVariation, 4.0);
    return info;
}

void tst_QGeoPositionInfoBenchmark::queryAttributeExisting()
{
    QGeoPositionInfo info = generateInfoWithAttributes();
    QBENCHMARK {
        const auto value = info.attribute(QGeoPositionInfo::GroundSpeed);
        Q_UNUSED(value)
    }
}

void tst_QGeoPositionInfoBenchmark::queryAttributeNonExisting()
{
    QGeoPositionInfo info = generateInfoWithAttributes();
    QBENCHMARK {
        const auto value = info.attribute(QGeoPositionInfo::HorizontalAccuracy);
        Q_UNUSED(value)
    }
}

void tst_QGeoPositionInfoBenchmark::removeAttributeExisting()
{
    QGeoPositionInfo info = generateInfoWithAttributes();
    QBENCHMARK {
        info.removeAttribute(QGeoPositionInfo::GroundSpeed);
    }
}

void tst_QGeoPositionInfoBenchmark::removeAttributeNonExisting()
{
    QGeoPositionInfo info = generateInfoWithAttributes();
    QBENCHMARK {
        info.removeAttribute(QGeoPositionInfo::HorizontalAccuracy);
    }
}

void tst_QGeoPositionInfoBenchmark::hasAttributeExisting()
{
    QGeoPositionInfo info = generateInfoWithAttributes();
    QBENCHMARK {
        const auto value = info.hasAttribute(QGeoPositionInfo::GroundSpeed);
        Q_UNUSED(value)
    }
}

void tst_QGeoPositionInfoBenchmark::hasAttributeNonExisting()
{
    QGeoPositionInfo info = generateInfoWithAttributes();
    QBENCHMARK {
        const auto value = info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy);
        Q_UNUSED(value)
    }
}

QTEST_MAIN(tst_QGeoPositionInfoBenchmark)

#include "tst_bench_qgeopositioninfo.moc"
