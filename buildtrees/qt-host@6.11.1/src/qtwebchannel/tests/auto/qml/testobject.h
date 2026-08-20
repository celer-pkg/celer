// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#ifndef TESTOBJECT_H
#define TESTOBJECT_H

#include <QObject>
#include <QProperty>
#include <QVariantMap>

QT_BEGIN_NAMESPACE

class TestObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap objectMap READ objectMap CONSTANT)
    Q_PROPERTY(QString stringProperty READ stringProperty WRITE setStringProperty BINDABLE bindableStringProperty)
public:
    explicit TestObject(QObject *parent = nullptr);
    ~TestObject();

    QVariantMap objectMap() const;
    QString stringProperty() const;
    QBindable<QString> bindableStringProperty() { return &m_stringProperty; }

    Q_INVOKABLE QObject* createQObject();

public slots:
    void triggerSignals();

    int testOverload(int i);
    QString testOverload(const QString &str);
    QString testOverload(const QString &str, int i);
    int testVariantType(const QVariant &val);
    bool testEmbeddedObjects(const QVariantList &list);
    void setStringProperty(const QString &stringProperty);

signals:
    void testSignalBool(bool testBool);
    void testSignalInt(int testInt);

    void testOverloadSignal(int i);
    void testOverloadSignal(const QString &str);
    void testOverloadSignal(const QString &str, int i);

private:
    QObject *embeddedObject;
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TestObject, QString, m_stringProperty, "foo")
};

QT_END_NAMESPACE

#endif // TESTOBJECT_H
