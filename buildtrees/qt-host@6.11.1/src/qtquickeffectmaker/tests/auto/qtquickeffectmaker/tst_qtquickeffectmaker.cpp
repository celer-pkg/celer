// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>

class Tst_QtQuickEffectMaker : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void dummyTest();
};

void Tst_QtQuickEffectMaker::dummyTest()
{
    QSKIP("Dummy test, skipping.");
}

QTEST_MAIN(Tst_QtQuickEffectMaker)

#include "tst_qtquickeffectmaker.moc"
