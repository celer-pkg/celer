// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_QGEOLOCATION_H
#define TST_QGEOLOCATION_H

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QMetaType>

#include "../utils/qlocationtestutils_p.h"
#include <QtPositioning/qgeoaddress.h>
#include <QtPositioning/qgeocoordinate.h>
#include <QtPositioning/qgeolocation.h>
#include <QtPositioning/QGeoRectangle>

QT_USE_NAMESPACE

class tst_QGeoLocation : public QObject
{
    Q_OBJECT

public:
    tst_QGeoLocation();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

   //Start Unit Tests for qgeolocation.h
    void constructor();
    void copy_constructor();
    void move_constructor();
    void move_assignment();
    void destructor();
    void address();
    void coordinate();
    void viewport();
    void extendedAttributes();
    void operators();
    void comparison();
    void comparison_data();
    void isEmpty();
    void hashing();
    void hashing_data();
    //End Unit Tests for qgeolocation.h

private:
    QGeoLocation m_location;

    QGeoAddress m_address;
    QGeoCoordinate m_coordinate;
    QGeoShape m_viewport;
    QVariantMap m_extendedAttributes;
};

Q_DECLARE_METATYPE( QGeoCoordinate::CoordinateFormat);
Q_DECLARE_METATYPE( QGeoCoordinate::CoordinateType);
Q_DECLARE_METATYPE( QList<double>);

#endif

