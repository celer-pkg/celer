// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../../shared/util.h"

#include <qtest.h>
#include <qqmlengine.h>
#include <qqmlcomponent.h>

#include <private/qqmlmetatype_p.h>
#include <private/qqmlengine_p.h>

class tst_qqmlstatemachinemetatype : public QQmlDataTest
{
    Q_OBJECT

private slots:
    void unregisterAttachedProperties()
    {
        qmlClearTypeRegistrations();

        QQmlEngine e;
        QQmlComponent c(&e, testFileUrl("unregisterAttachedProperties.qml"));
        QVERIFY2(c.isReady(), qPrintable(c.errorString()));

        const QQmlType attachedType = QQmlMetaType::qmlType("QtQuick/KeyNavigation",
                                                            QTypeRevision::fromVersion(2, 2));
        QCOMPARE(attachedType.attachedPropertiesType(e.handle()->typeLoader()),
                 attachedType.metaObject());

        QScopedPointer<QObject> obj(c.create());
        QVERIFY(obj);
    }
};

QTEST_MAIN(tst_qqmlstatemachinemetatype)

#include "tst_qqmlstatemachinemetatype.moc"
