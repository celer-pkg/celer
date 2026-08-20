// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtPositioning/QGeoSatelliteInfo>
#include <QTest>

class tst_QGeoSatelliteInfoBenchmark : public QObject
{
    Q_OBJECT
private slots:
    void constructDefault();
    void constructCopy();
    void constructMove();

    void assign();
    void assignMove();

    void checkEquality();

    void setSatelliteSystem();
    void querySatelliteSystem();

    void setSatelliteIdentifier();
    void querySatelliteIdentifier();

    void setSignalStrength();
    void querySignalStrength();

    void setAttribute();
    void queryAttributeExisting();
    void queryAttributeNonExisting();
    void removeAttributeExisting();
    void removeAttributeNonExisting();
    void hasAttributeExisting();
    void hasAttributeNonExisting();
};

void tst_QGeoSatelliteInfoBenchmark::constructDefault()
{
    QBENCHMARK {
        QGeoSatelliteInfo info;
        Q_UNUSED(info)
    }
}

static QGeoSatelliteInfo createSatelliteInfo()
{
    QGeoSatelliteInfo info;
    info.setSatelliteSystem(QGeoSatelliteInfo::GLONASS);
    info.setSatelliteIdentifier(1);
    info.setSignalStrength(-30);
    info.setAttribute(QGeoSatelliteInfo::Elevation, 10.0);
    return info;
}

void tst_QGeoSatelliteInfoBenchmark::constructCopy()
{
    const auto info = createSatelliteInfo();
    QBENCHMARK {
        QGeoSatelliteInfo newInfo(info);
        Q_UNUSED(newInfo)
    }
}

void tst_QGeoSatelliteInfoBenchmark::constructMove()
{
    QBENCHMARK {
        // We need to create a new object at each iteration, so that we don't
        // end up moving an already moved-from object. So the real value for
        // move can be calculated using the results of constructDefault()
        // benchmark.
        QGeoSatelliteInfo info;
        QGeoSatelliteInfo newInfo(std::move(info));
        Q_UNUSED(newInfo)
    }
}

void tst_QGeoSatelliteInfoBenchmark::assign()
{
    const auto info = createSatelliteInfo();
    QGeoSatelliteInfo newInfo;
    QBENCHMARK {
        newInfo = info;
    }
}

void tst_QGeoSatelliteInfoBenchmark::assignMove()
{
    QGeoSatelliteInfo newInfo;
    QBENCHMARK {
        // We need to create a new object at each iteration, so that we don't
        // end up moving an already moved-from object. So the real value for
        // move can be calculated using the results of constructDefault()
        // benchmark.
        QGeoSatelliteInfo info;
        newInfo = std::move(info);
    }
}

void tst_QGeoSatelliteInfoBenchmark::checkEquality()
{
    const auto info1 = createSatelliteInfo();
    const auto info2 = createSatelliteInfo();
    QBENCHMARK {
        const bool equal = info1 == info2;
        Q_UNUSED(equal)
    }
}

void tst_QGeoSatelliteInfoBenchmark::setSatelliteSystem()
{
    QGeoSatelliteInfo info;
    QBENCHMARK {
        info.setSatelliteSystem(QGeoSatelliteInfo::GPS);
    }
}

void tst_QGeoSatelliteInfoBenchmark::querySatelliteSystem()
{
    const auto info = createSatelliteInfo();
    QBENCHMARK {
        const auto val = info.satelliteSystem();
        Q_UNUSED(val)
    }
}

void tst_QGeoSatelliteInfoBenchmark::setSatelliteIdentifier()
{
    QGeoSatelliteInfo info;
    QBENCHMARK {
        info.setSatelliteIdentifier(10);
    }
}

void tst_QGeoSatelliteInfoBenchmark::querySatelliteIdentifier()
{
    const auto info = createSatelliteInfo();
    QBENCHMARK {
        const auto val = info.satelliteIdentifier();
        Q_UNUSED(val)
    }
}

void tst_QGeoSatelliteInfoBenchmark::setSignalStrength()
{
    QGeoSatelliteInfo info;
    QBENCHMARK {
        info.setSignalStrength(-50);
    }
}

void tst_QGeoSatelliteInfoBenchmark::querySignalStrength()
{
    const auto info = createSatelliteInfo();
    QBENCHMARK {
        const auto val = info.signalStrength();
        Q_UNUSED(val)
    }
}

void tst_QGeoSatelliteInfoBenchmark::setAttribute()
{
    QGeoSatelliteInfo info;
    QBENCHMARK {
        info.setAttribute(QGeoSatelliteInfo::Elevation, 10.0);
    }
}

void tst_QGeoSatelliteInfoBenchmark::queryAttributeExisting()
{
    const auto info = createSatelliteInfo();
    QBENCHMARK {
        const auto val = info.attribute(QGeoSatelliteInfo::Elevation);
        Q_UNUSED(val)
    }
}

void tst_QGeoSatelliteInfoBenchmark::queryAttributeNonExisting()
{
    const auto info = createSatelliteInfo();
    QBENCHMARK {
        const auto val = info.attribute(QGeoSatelliteInfo::Azimuth);
        Q_UNUSED(val)
    }
}

void tst_QGeoSatelliteInfoBenchmark::removeAttributeExisting()
{
    auto info = createSatelliteInfo();
    QBENCHMARK {
        info.removeAttribute(QGeoSatelliteInfo::Elevation);
    }
}

void tst_QGeoSatelliteInfoBenchmark::removeAttributeNonExisting()
{
    auto info = createSatelliteInfo();
    QBENCHMARK {
        info.removeAttribute(QGeoSatelliteInfo::Azimuth);
    }
}

void tst_QGeoSatelliteInfoBenchmark::hasAttributeExisting()
{
    auto info = createSatelliteInfo();
    QBENCHMARK {
        const auto val = info.hasAttribute(QGeoSatelliteInfo::Elevation);
        Q_UNUSED(val)
    }
}

void tst_QGeoSatelliteInfoBenchmark::hasAttributeNonExisting()
{
    auto info = createSatelliteInfo();
    QBENCHMARK {
        const auto val = info.hasAttribute(QGeoSatelliteInfo::Azimuth);
        Q_UNUSED(val)
    }
}

QTEST_MAIN(tst_QGeoSatelliteInfoBenchmark)

#include "tst_bench_qgeosatelliteinfo.moc"
