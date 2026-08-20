// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQuickTest>

#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>

class Setup : public QObject
{
    Q_OBJECT

public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        bool testDeprecatedSslOptionsProperty = false;
#if QT_REMOVAL_QT7_DEPRECATED_SINCE(6, 11)
        testDeprecatedSslOptionsProperty = true;
#endif
        engine->rootContext()->setContextProperty("testDeprecatedSslOptionsProperty",
                                                  testDeprecatedSslOptionsProperty);
    }
};

QUICK_TEST_MAIN_WITH_SETUP(tst_sslconfiguration_qml, Setup)

#include "tst_sslconfiguration_qml.moc"
