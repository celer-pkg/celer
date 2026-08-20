// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QtPositioning/private/qwebmercator_p.h>
#include <qtest.h>

QT_USE_NAMESPACE

class tst_qwebmercator : public QObject
{
    Q_OBJECT

private:
    void transformForwardAndBackward(double lat, double lon)
    {
        QGeoCoordinate c(lat, lon);
        QDoubleVector2D cmap = QWebMercator().coordToMercator(c);
        QGeoCoordinate c1 = QWebMercator().mercatorToCoord(cmap);
        QCOMPARE(c1.latitude(), c.latitude());
        QCOMPARE(c1.longitude(), c.longitude());
    }

private slots:
    void transformForwardAndBackward()
    {
        transformForwardAndBackward(0, 0);
        // transformForwardAndBackward(42,180); //Will fail.

        for (double lon=-180; lon<181; lon += 22/3.) {
            for (double lat=-90; lat<91; lat += 22/3.)
                transformForwardAndBackward(lat, lon);
        }
        for (double eps = 1.0; eps > 1e-12; eps /= 10.0) {
            transformForwardAndBackward(-90 + eps, 0.0);
            transformForwardAndBackward(90 - eps, 0.0);
        }
    }
};

QTEST_GUILESS_MAIN(tst_qwebmercator)
#include "tst_qwebmercator.moc"
