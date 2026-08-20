// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SHADOWINGSINGLETON_H
#define SHADOWINGSINGLETON_H

#include <QtCore/qobject.h>
#include <QtQml/qqmlengine.h>
#include <QtQmlIntegration/qqmlintegration.h>

class ShadowingSingletonBase : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    Q_INVOKABLE virtual QString aFunction() const = 0;

signals:
    void aSignal(int);
};


class ShadowingSingleton : public ShadowingSingletonBase
{
    Q_OBJECT
public:

    explicit ShadowingSingleton(QObject *parent = nullptr)
        : ShadowingSingletonBase(parent)
    {}

    Q_INVOKABLE QString aFunction() const override
    {
        return QStringLiteral("Hej");
    }
};

class ShadowingSingletonForeign
{
    Q_GADGET
    QML_FOREIGN(ShadowingSingletonBase)
    QML_SINGLETON
    QML_NAMED_ELEMENT(ShadowingSingleton)
public:
    static ShadowingSingletonBase *create(QQmlEngine *engine, QJSEngine *scriptEngine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine)
        return new ShadowingSingleton;
    }
};

#endif // SHADOWINGSINGLETON_H
