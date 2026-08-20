// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <storeloader.h>

#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

#include <QtCore/QVariant>

using namespace Qt::StringLiterals;

class QtDynamicFeatureTest : public QObject
{
    Q_OBJECT
public:
    QtDynamicFeatureTest(){}

private Q_SLOTS:
    void loadResourcesFeature();
};

void QtDynamicFeatureTest::loadResourcesFeature()
{
    QVERIFY(!QFile::exists(":/dynamic_resources/qtlogo.png"));

    auto handler = StoreLoader::loadModule("tst_android_dynamic_feature_resources"_L1);

    QSignalSpy spy(handler.get(), &StoreLoaderHandler::finished);

    QVERIFY(spy.wait(20000));
    QVERIFY(QFile::exists(":/dynamic_resources/qtlogo.png"));
}

QTEST_MAIN(QtDynamicFeatureTest)

#include "tst_android_dynamic_feature.moc"
