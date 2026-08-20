// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtPositioning/QGeoAreaMonitorInfo>
#include <QtPositioning/QGeoCircle>
#include <QTest>

static const QDateTime expirationTime = QDateTime::currentDateTimeUtc().addSecs(60);
static const QGeoCircle area = QGeoCircle(QGeoCoordinate(1.0, 1.0), 100);

class tst_QGeoAreaMonitorInfoBenchmark : public QObject
{
    Q_OBJECT
private slots:
    void construct();
    void constructCopy();
    void constructMove();

    void assign();
    void assignMove();

    void checkEquality();

    void setName();
    void queryName();

    void queryIdentifier();
    void isValid();

    void setArea();
    void queryArea();

    void setExpiration();
    void queryExpiration();

    void setPersistent();
    void queryPersistent();

    void setNotificationParameters();
    void queryNotificationParameters();
};



void tst_QGeoAreaMonitorInfoBenchmark::construct()
{
    QBENCHMARK {
        QGeoAreaMonitorInfo info("test");
        Q_UNUSED(info)
    }
}

static QGeoAreaMonitorInfo createAreaMonitorInfo()
{
    QGeoAreaMonitorInfo info("test");
    info.setExpiration(expirationTime);
    info.setArea(area);
    QVariantMap parameters;
    parameters["key"] = "value";
    parameters["another_key"] = true;
    info.setNotificationParameters(parameters);
    return info;
}

void tst_QGeoAreaMonitorInfoBenchmark::constructCopy()
{
    const auto info = createAreaMonitorInfo();
    QBENCHMARK {
        QGeoAreaMonitorInfo newInfo(info);
        Q_UNUSED(newInfo)
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::constructMove()
{
    QBENCHMARK {
        // We need to create a new object at each iteration, so that we don't
        // end up moving an already moved-from object. So the real value for
        // move can be calculated using the results of construct()
        // benchmark.
        QGeoAreaMonitorInfo info("test");
        QGeoAreaMonitorInfo newInfo(std::move(info));
        Q_UNUSED(newInfo)
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::assign()
{
    const auto info = createAreaMonitorInfo();
    QGeoAreaMonitorInfo newInfo;
    QBENCHMARK {
        newInfo = info;
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::assignMove()
{
    QGeoAreaMonitorInfo newInfo;
    QBENCHMARK {
        // We need to create a new object at each iteration, so that we don't
        // end up moving an already moved-from object. So the real value for
        // move can be calculated using the results of construct()
        // benchmark.
        QGeoAreaMonitorInfo info("test");
        newInfo = std::move(info);
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::checkEquality()
{
    const auto info1 = createAreaMonitorInfo();
    const auto info2 = createAreaMonitorInfo();
    QBENCHMARK {
        const bool equal = info1 == info2;
        Q_UNUSED(equal)
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::setName()
{
    auto info = createAreaMonitorInfo();
    // Setting the name twice, as there is a check for same name.
    // Ideally need to divide the result of the benchmark by 2.
    QBENCHMARK {
        info.setName("name1");
        info.setName("name2");
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::queryName()
{
    const auto info = createAreaMonitorInfo();
    QBENCHMARK {
        const auto val = info.name();
        Q_UNUSED(val)
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::queryIdentifier()
{
    const auto info = createAreaMonitorInfo();
    QBENCHMARK {
        const auto val = info.identifier();
        Q_UNUSED(val)
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::isValid()
{
    const auto info = createAreaMonitorInfo();
    QBENCHMARK {
        const auto val = info.isValid();
        Q_UNUSED(val)
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::setArea()
{
    auto info = createAreaMonitorInfo();
    QBENCHMARK {
        info.setArea(area);
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::queryArea()
{
    const auto info = createAreaMonitorInfo();
    QBENCHMARK {
        const auto val = info.area();
        Q_UNUSED(val)
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::setExpiration()
{
    auto info = createAreaMonitorInfo();
    QBENCHMARK {
        info.setExpiration(expirationTime);
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::queryExpiration()
{
    const auto info = createAreaMonitorInfo();
    QBENCHMARK {
        const auto val = info.expiration();
        Q_UNUSED(val)
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::setPersistent()
{
    auto info = createAreaMonitorInfo();
    QBENCHMARK {
        info.setPersistent(true);
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::queryPersistent()
{
    const auto info = createAreaMonitorInfo();
    QBENCHMARK {
        const auto val = info.isPersistent();
        Q_UNUSED(val)
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::setNotificationParameters()
{
    auto info = createAreaMonitorInfo();
    QVariantMap newParameters;
    newParameters["key"] = "value1";
    newParameters["another_key"] = false;
    QBENCHMARK {
        info.setNotificationParameters(newParameters);
    }
}

void tst_QGeoAreaMonitorInfoBenchmark::queryNotificationParameters()
{
    const auto info = createAreaMonitorInfo();
    QBENCHMARK {
        const auto val = info.notificationParameters();
        Q_UNUSED(val)
    }
}

QTEST_MAIN(tst_QGeoAreaMonitorInfoBenchmark)

#include "tst_bench_qgeoareamonitorinfo.moc"
