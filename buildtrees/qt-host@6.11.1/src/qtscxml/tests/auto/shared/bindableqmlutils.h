// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BINDABLEQMLUTILS_H
#define BINDABLEQMLUTILS_H

#include <QtQml/QQmlListReference>
#include <QtQml/QQmlProperty>
#include <QObject>
#include <QtTest>

// This is a helper function to test basics of typical bindable
// and manipulable QQmlListProperty
// "TestedClass" is the class type we are testing
// "TestedData" is the data type that the list contains
// "testedClass" is an instance of the class we are interested testing
// "data1" is a value that can be set and removed from the list
// "data2" is a value that can be set and removed from the list
// "propertyName" is the name of the property we are interested in testing
template<typename TestedClass, typename TestedData>
void testManipulableQmlListBasics(TestedClass& testedClass,
                                TestedData data1, TestedData data2,
                                const char* propertyName)
{
    // Get the property we are testing
    const QMetaObject *metaObject = testedClass.metaObject();
    QMetaProperty metaProperty = metaObject->property(metaObject->indexOfProperty(propertyName));

    // Generate a string to help identify failures (as this is a generic template)
    QString id(metaObject->className());
    id.append(QStringLiteral("::"));
    id.append(propertyName);

    // Fail gracefully if preconditions to use this helper function are not met:
    QQmlListReference listRef(&testedClass, propertyName);
    QVERIFY2(metaProperty.isBindable(), qPrintable(id));
    QVERIFY2(listRef.isManipulable(), qPrintable(id));
    QVERIFY2(data1 != data2, qPrintable(id));

    // Create a signal spy for the property changed -signal if such exists
    QScopedPointer<QSignalSpy> changedSpy(nullptr);
    if (metaProperty.hasNotifySignal())
        changedSpy.reset(new QSignalSpy(&testedClass, metaProperty.notifySignal()));

    // Modify values without binding (verify property basics still work)
    QVERIFY2(listRef.count() == 0, qPrintable(id));
    listRef.append(data1);
    QVERIFY2(listRef.count() == 1, qPrintable(id));
    if (changedSpy)
        QVERIFY2(changedSpy->size() == 1, qPrintable(id + ", actual: " + QString::number(changedSpy->size())));
    QVERIFY2(listRef.at(0) == data1, qPrintable(id));
    listRef.clear();
    QVERIFY2(listRef.count() == 0, qPrintable(id));
    if (changedSpy)
        QVERIFY2(changedSpy->size() == 2, qPrintable(id + ", actual: " + QString::number(changedSpy->size())));

    // Bind to the property and verify that the bindings reflect the listproperty changes
    QProperty<bool> data1InList([&](){
        QQmlListReference list(&testedClass, propertyName);
        for (int i = 0; i < list.count(); i++) {
            if (list.at(i) == data1)
                return true;
        }
        return false;
    });
    QProperty<bool> data2InList([&](){
        QQmlListReference list(&testedClass, propertyName);
        for (int i = 0; i < list.count(); i++) {
            if (list.at(i) == data2)
                return true;
        }
        return false;
    });

    // initial state
    QVERIFY2(!data1InList, qPrintable(id));
    QVERIFY2(!data2InList, qPrintable(id));

    listRef.append(data1);
    if (changedSpy)
        QVERIFY2(changedSpy->size() == 3, qPrintable(id + ", actual: " + QString::number(changedSpy->size())));
    QVERIFY2(data1InList, qPrintable(id));
    QVERIFY2(!data2InList, qPrintable(id));

    listRef.append(data2);
    if (changedSpy)
        QVERIFY2(changedSpy->size() == 4, qPrintable(id + ", actual: " + QString::number(changedSpy->size())));
    QVERIFY2(data1InList, qPrintable(id));
    QVERIFY2(data2InList, qPrintable(id));
    QVERIFY2(listRef.count() == 2, qPrintable(id + ", actual: " + QString::number(changedSpy->size())));

    listRef.clear();
    if (changedSpy)
        QVERIFY2(changedSpy->size() == 5, qPrintable(id + ", actual: " + QString::number(changedSpy->size())));
    QVERIFY2(!data1InList, qPrintable(id));
    QVERIFY2(!data2InList, qPrintable(id));
    QVERIFY2(listRef.count() == 0, qPrintable(id + ", actual: " + QString::number(changedSpy->size())));

    listRef.append(data1);
    listRef.replace(0, data2);
    if (changedSpy)
        QVERIFY2(changedSpy->size() == 7, qPrintable(id + ", actual: " + QString::number(changedSpy->size())));
    QVERIFY2(!data1InList, qPrintable(id));
    QVERIFY2(data2InList, qPrintable(id));
    QVERIFY2(listRef.count() == 1, qPrintable(id + ", actual: " + QString::number(changedSpy->size())));

    listRef.removeLast();
    if (changedSpy)
        QVERIFY2(changedSpy->size() == 8, qPrintable(id + ", actual: " + QString::number(changedSpy->size())));
    QVERIFY2(!data1InList, qPrintable(id));
    QVERIFY2(!data2InList, qPrintable(id));

    QVERIFY2(listRef.count() == 0, qPrintable(id + ", actual: " + QString::number(listRef.count())));
}

#endif // BINDABLEQMLUTILS_H
