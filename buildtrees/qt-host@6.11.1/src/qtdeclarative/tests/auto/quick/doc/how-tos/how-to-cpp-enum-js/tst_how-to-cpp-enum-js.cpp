// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/qregularexpression.h>
#include <QtTest/QTest>
#include <QtQml/qjsengine.h>

#include "backend.h"

QT_BEGIN_NAMESPACE

class tst_HowToCppEnumJs : public QObject
{
    Q_OBJECT

public:
    tst_HowToCppEnumJs();

private slots:
    void example();
};

tst_HowToCppEnumJs::tst_HowToCppEnumJs()
{
}

void tst_HowToCppEnumJs::example()
{
    QTest::failOnWarning();

    QJSEngine engine;
    engine.installExtensions(QJSEngine::AllExtensions);

    QJSValue metaObjects = engine.newObject();
    const QJSValue backendJsMetaObject = engine.newQMetaObject(&Backend::staticMetaObject);
    metaObjects.setProperty("Backend", backendJsMetaObject);
    // Repeat the two lines above for other types as needed.
    engine.registerModule("MyApp", metaObjects);

    QTest::ignoreMessage(QtDebugMsg, "Backend loaded successfully");
    Backend backend(&engine);
    const bool loaded = backend.load();
    QVERIFY(loaded);
}

QT_END_NAMESPACE

QTEST_MAIN(tst_HowToCppEnumJs)

#include "tst_how-to-cpp-enum-js.moc"
