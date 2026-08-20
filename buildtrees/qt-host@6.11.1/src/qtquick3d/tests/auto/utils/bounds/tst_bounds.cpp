// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtQuick3DUtils/private/qssgbounds3_p.h>

class bounds : public QObject
{
    Q_OBJECT

private slots:
    void test_constructors();
};

void bounds::test_constructors()
{
    const auto boundsDefault = QSSGBounds3();
    const auto boundsUninitialized = QSSGBounds3(Qt::Uninitialized);
    auto boundsEmpty = QSSGBounds3();
    boundsEmpty.setEmpty();

    QVERIFY2(boundsDefault.minimum == boundsEmpty.minimum, "Empty equals default constructor");
    QVERIFY2(boundsDefault.maximum == boundsEmpty.maximum, "Empty equals default constructor");

    QVERIFY2(boundsDefault.minimum != boundsUninitialized.minimum, "Uninitialized is not equal to default");

    QVERIFY2(boundsEmpty.isEmpty(), "Check empty");
    QVERIFY2(boundsDefault.isEmpty(), "Check empty");
}

QTEST_APPLESS_MAIN(bounds)

#include "tst_bounds.moc"
