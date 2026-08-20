// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>

#include <QtTest/qsignalspy.h>

#include <QtQmlModels/private/qqmldelegatemodel_p.h>
#include <QtQuick3D/private/qquick3drepeater_p.h>

#include <QtQuickTestUtils/private/qmlutils_p.h>

class tst_qquick3drepeater : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_qquick3drepeater();

private slots:
    void setDelegateAfterModel();
    void delegateModelAccess();
    void delegateModelAccess_data();
};

tst_qquick3drepeater::tst_qquick3drepeater()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
}

void tst_qquick3drepeater::setDelegateAfterModel()
{
    QQmlEngine engine;
    const QUrl url = testFileUrl("setDelegateAfterModel.qml");
    QQmlComponent component(&engine, url);
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));

    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());

    // If the model was lost by setting the delegate, the count would be 0.
    QCOMPARE(object->property("count").toInt(), 3);

    QTest::ignoreMessage(
            QtWarningMsg,
            qPrintable(url.toString() + ":18:5: QML Repeater3D: Cannot retain explicitly set "
                                        "delegate on non-DelegateModel"));
    object->setProperty("useObjectModel", QVariant::fromValue<bool>(true));
    QCOMPARE(object->property("count").toInt(), 2);

    // The old model must not mess with the view anymore.
    QTest::failOnWarning(qPrintable(url.toString() + ":18:5: QML Repeater3D: Explicitly set delegate "
                                                     "is externally overridden"));
    QMetaObject::invokeMethod(object.data(), "plantDelegate");
    QCOMPARE(object->property("count").toInt(), 4);
}


namespace Model {
Q_NAMESPACE
QML_ELEMENT
enum Kind : qint8
{
    None = -1,
    Singular,
    List,
    Array,
    Object
};
Q_ENUM_NS(Kind)
}

namespace Delegate {
Q_NAMESPACE
QML_ELEMENT
enum Kind : qint8
{
    None = -1,
    Untyped,
    Typed
};
Q_ENUM_NS(Kind)
}

template<typename Enum>
const char *enumKey(Enum value) {
    const QMetaObject *mo = qt_getEnumMetaObject(value);
    const QMetaEnum metaEnum = mo->enumerator(mo->indexOfEnumerator(qt_getEnumName(value)));
    return metaEnum.valueToKey(value);
}

void tst_qquick3drepeater::delegateModelAccess_data()
{
    QTest::addColumn<QQmlDelegateModel::DelegateModelAccess>("access");
    QTest::addColumn<Model::Kind>("modelKind");
    QTest::addColumn<Delegate::Kind>("delegateKind");

    using Access = QQmlDelegateModel::DelegateModelAccess;
    for (auto access : { Access::Qt5ReadWrite, Access::ReadOnly, Access::ReadWrite }) {
        for (auto model : { Model::Singular, Model::List, Model::Array, Model::Object }) {
            for (auto delegate : { Delegate::Untyped, Delegate::Typed }) {
                QTest::addRow("%s-%s-%s", enumKey(access), enumKey(model), enumKey(delegate))
                << access << model << delegate;
            }
        }
    }
}

void tst_qquick3drepeater::delegateModelAccess()
{
    static const bool initialized = []() {
        qmlRegisterNamespaceAndRevisions(&Model::staticMetaObject, "Test", 1);
        qmlRegisterNamespaceAndRevisions(&Delegate::staticMetaObject, "Test", 1);
        return true;
    }();
    QVERIFY(initialized);

    QFETCH(QQmlDelegateModel::DelegateModelAccess, access);
    QFETCH(Model::Kind, modelKind);
    QFETCH(Delegate::Kind, delegateKind);

    QQmlEngine engine;
    const QUrl url = testFileUrl("delegateModelAccess.qml");
    QQmlComponent c(&engine, url);
    QVERIFY2(c.isReady(), qPrintable(c.errorString()));
    QScopedPointer<QObject> object(c.create());

    QQuick3DRepeater *repeater = qvariant_cast<QQuick3DRepeater *>(object->property("repeater"));
    QVERIFY(repeater);

    QSignalSpy modelChangedSpy(repeater, &QQuick3DRepeater::modelChanged);

    if (delegateKind == Delegate::Untyped && modelKind == Model::Array)
        QSKIP("Properties of objects in arrays are not exposed as context properties");

    if (access == QQmlDelegateModel::ReadOnly) {
        const QRegularExpression message(
                url.toString() + ":[0-9]+: TypeError: Cannot assign to read-only property \"a\"");

        QTest::ignoreMessage(QtWarningMsg, message);
        if (delegateKind == Delegate::Untyped)
            QTest::ignoreMessage(QtWarningMsg, message);
    }

    object->setProperty("delegateModelAccess", access);
    object->setProperty("modelIndex", modelKind);
    object->setProperty("delegateIndex", delegateKind);

    QObject *delegate = repeater->objectAt(0);
    QVERIFY(delegate);

    const bool modelWritable = access != QQmlDelegateModel::ReadOnly;
    const bool immediateWritable = (delegateKind == Delegate::Untyped)
            ? access != QQmlDelegateModel::ReadOnly
            : access == QQmlDelegateModel::ReadWrite;

    // Only the array is actually updated itself. The other models are pointers
    const bool writeShouldSignal = modelKind == Model::Kind::Array;

    double expected = 11;

    // Initial setting of the model, signals one update
    int expectedModelUpdates = 1;
    QCOMPARE(modelChangedSpy.count(), expectedModelUpdates);

    QCOMPARE(delegate->property("immediateX").toDouble(), expected);
    QCOMPARE(delegate->property("modelX").toDouble(), expected);

    if (modelWritable) {
        expected = 3;
        if (writeShouldSignal)
            ++expectedModelUpdates;
    }

    QMetaObject::invokeMethod(delegate, "writeThroughModel");
    QCOMPARE(delegate->property("immediateX").toDouble(), expected);
    QCOMPARE(delegate->property("modelX").toDouble(), expected);
    QCOMPARE(modelChangedSpy.count(), expectedModelUpdates);

    if (immediateWritable) {
        expected = 1;
        if (writeShouldSignal)
            ++expectedModelUpdates;
    }

    QMetaObject::invokeMethod(delegate, "writeImmediate");

    // Writes to required properties always succeed, but might not be propagated to the model
    QCOMPARE(delegate->property("immediateX").toDouble(),
             delegateKind == Delegate::Untyped ? expected : 1);

    QCOMPARE(delegate->property("modelX").toDouble(), expected);
    QCOMPARE(modelChangedSpy.count(), expectedModelUpdates);
}

QTEST_MAIN(tst_qquick3drepeater)

#include "tst_qquick3drepeater.moc"
