// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testqgeosatelliteinfosource_p.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::unique_ptr<TestQGeoSatelliteInfoSource> test(TestQGeoSatelliteInfoSource::createDefaultSourceTest());
    return QTest::qExec(test.get(), argc, argv);
}
