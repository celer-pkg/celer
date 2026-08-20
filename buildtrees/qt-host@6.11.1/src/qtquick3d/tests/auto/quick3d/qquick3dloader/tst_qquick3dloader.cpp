// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <private/qmlutils_p.h>

#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>

class tst_QQuick3DLoader : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_QQuick3DLoader();

private slots:
    void boundComponent();
};

tst_QQuick3DLoader::tst_QQuick3DLoader()
    : QQmlDataTest(QT_QMLTEST_DATADIR, FailOnWarningsPolicy::FailOnWarnings)
{
}

void tst_QQuick3DLoader::boundComponent()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("boundComponent.qml"));
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    QScopedPointer<QObject> o(component.create());
    QCOMPARE(o->objectName(), QStringLiteral("loaded"));
}

QTEST_MAIN(tst_QQuick3DLoader)

#include "tst_qquick3dloader.moc"
