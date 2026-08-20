// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testqgeopositioninfosource_p.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::unique_ptr<TestQGeoPositionInfoSource> test(
            TestQGeoPositionInfoSource::createDefaultSourceTest());
    return QTest::qExec(test.get(), argc, argv);
}
