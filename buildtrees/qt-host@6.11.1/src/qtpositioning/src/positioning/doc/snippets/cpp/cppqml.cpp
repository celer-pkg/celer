// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <QtCore/QVariant>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoLocation>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoAreaMonitorSource>

void cppQmlInterface(QObject *qmlObject)
{
   //! [Address get]
    QGeoAddress geoAddress = qmlObject->property("address").value<QGeoAddress>();
    //! [Address get]

    //! [Address set]
    qmlObject->setProperty("address", QVariant::fromValue(geoAddress));
    //! [Address set]

    //! [Location get]
    QGeoLocation geoLocation = qmlObject->property("location").value<QGeoLocation>();
    //! [Location get]

    //! [Location set]
    qmlObject->setProperty("location", QVariant::fromValue(geoLocation));
    //! [Location set]
}

class MyClass : public QObject
{
    Q_OBJECT
//! [BigBen]
public:
    MyClass() : QObject()
    {
        QGeoAreaMonitorSource *monitor = QGeoAreaMonitorSource::createDefaultSource(this);
        if (monitor) {
            connect(monitor, SIGNAL(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo)),
                    this, SLOT(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo)));
            connect(monitor, SIGNAL(areaExited(QGeoAreaMonitorInfo,QGeoPositionInfo)),
                    this, SLOT(areaExited(QGeoAreaMonitorInfo,QGeoPositionInfo)));

            QGeoAreaMonitorInfo bigBen("Big Ben");
            QGeoCoordinate position(51.50104, -0.124632);
            bigBen.setArea(QGeoCircle(position, 100));

            monitor->startMonitoring(bigBen);

        } else {
            qDebug() << "Could not create default area monitor";
        }
    }

public Q_SLOTS:
    void areaEntered(const QGeoAreaMonitorInfo &mon, const QGeoPositionInfo &update)
    {
        Q_UNUSED(mon);

        qDebug() << "Now within 100 meters, current position is" << update.coordinate();
    }

    void areaExited(const QGeoAreaMonitorInfo &mon, const QGeoPositionInfo &update)
    {
        Q_UNUSED(mon);

        qDebug() << "No longer within 100 meters, current position is" << update.coordinate();
    }
//! [BigBen]
};
