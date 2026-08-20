// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "testtypes.h"

QList<Padding::LogEntry> Padding::log;
int FromJSValue::constructorCalls = 0;
int FromJSPrimitive::constructorCalls = 0;
int FromJSManaged::constructorCalls = 0;

void registerTypes()
{
    qmlRegisterType<MyTypeObject>("Test", 1, 0, "MyTypeObject");
    qmlRegisterTypesAndRevisions<ConstructibleValueType>("Test", 1);
    qmlRegisterTypesAndRevisions<ConstructibleFromQReal>("Test", 1);
    qmlRegisterTypesAndRevisions<StructuredValueType>("Test", 1);
    qmlRegisterTypesAndRevisions<ForeignAnonymousStructuredValueType>("Test", 1);
    qmlRegisterTypesAndRevisions<Padding>("Test", 1);
    qmlRegisterTypesAndRevisions<MyItem>("Test", 1);
    qmlRegisterTypesAndRevisions<FromJSValue>("Test", 1);
    qmlRegisterTypesAndRevisions<FromJSPrimitive>("Test", 1);
    qmlRegisterTypesAndRevisions<FromJSManaged>("Test", 1);
}
