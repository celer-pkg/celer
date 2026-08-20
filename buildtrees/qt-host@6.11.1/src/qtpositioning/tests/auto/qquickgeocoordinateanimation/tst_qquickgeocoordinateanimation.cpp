// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QtTest>
#include <QtTest/private/qpropertytesthelper_p.h>
#include <QtPositioningQuick/private/qquickgeocoordinateanimation_p.h>

QT_USE_NAMESPACE

class tst_QuickGeoCoordinateAnimation : public QObject
{
    Q_OBJECT

private slots:
    void bindings();
};

void tst_QuickGeoCoordinateAnimation::bindings()
{
    QQuickGeoCoordinateAnimation animation;
    QTestPrivate::testReadWritePropertyBasics<QQuickGeoCoordinateAnimation,
                                              QQuickGeoCoordinateAnimation::Direction>(
            animation, QQuickGeoCoordinateAnimation::East, QQuickGeoCoordinateAnimation::West,
            "direction");
}

QTEST_APPLESS_MAIN(tst_QuickGeoCoordinateAnimation)

#include "tst_qquickgeocoordinateanimation.moc"
