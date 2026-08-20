// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testobject.h"


QT_BEGIN_NAMESPACE

TestObject::TestObject(QObject* parent)
    : QObject(parent)
    , embeddedObject(new QObject(this))
{
    embeddedObject->setObjectName("embedded");
}

TestObject::~TestObject()
{
}

QVariantMap TestObject::objectMap() const
{
    QVariantMap map;
    map.insert("subObject", QVariant::fromValue(embeddedObject));
    return map;
}

QString TestObject::stringProperty() const
{
    return m_stringProperty;
}

void TestObject::triggerSignals()
{
    emit testSignalBool(true);
    emit testSignalBool(false);

    emit testSignalInt(42);
    emit testSignalInt(1);
    emit testSignalInt(0);
}

int TestObject::testOverload(int i)
{
    emit testOverloadSignal(i);
    return i + 1;
}

QString TestObject::testOverload(const QString &str)
{
    emit testOverloadSignal(str);
    return str.toUpper();
}

QString TestObject::testOverload(const QString &str, int i)
{
    emit testOverloadSignal(str, i);
    return str.toUpper() + QString::number(i + 1);
}

int TestObject::testVariantType(const QVariant &val)
{
    return val.metaType().id();
}

bool TestObject::testEmbeddedObjects(const QVariantList &list)
{
    return list.size() == 2 &&
            list[0].metaType().id() == QMetaType::QObjectStar &&
            list[1].metaType().id() == QMetaType::QVariantMap &&
            list[1].toMap()["obj"].metaType().id() == QMetaType::QObjectStar;
}

void TestObject::setStringProperty(const QString &stringProperty)
{
    m_stringProperty = stringProperty;
}

QObject* TestObject::createQObject() {
     return new QObject(this);
}

QT_END_NAMESPACE
