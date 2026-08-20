// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qscxmlecmascriptplatformproperties_p.h"
#include "qscxmlstatemachine.h"

#include <qjsengine.h>

QT_BEGIN_NAMESPACE
class QScxmlPlatformProperties::Data
{
public:
    Data()
        : m_stateMachine(nullptr)
    {}

    QScxmlStateMachine *m_stateMachine;
    QJSValue m_jsValue;
};

QScxmlPlatformProperties::QScxmlPlatformProperties(QObject *parent)
    : QObject(parent)
    , data(new Data)
{}

QScxmlPlatformProperties *QScxmlPlatformProperties::create(QJSEngine *engine, QScxmlStateMachine *stateMachine)
{
    QScxmlPlatformProperties *pp = new QScxmlPlatformProperties(engine);
    pp->data->m_stateMachine = stateMachine;
    pp->data->m_jsValue = engine->newQObject(pp);
    return pp;
}

QScxmlPlatformProperties::~QScxmlPlatformProperties()
{
    delete data;
}

QJSEngine *QScxmlPlatformProperties::engine() const
{
    return qobject_cast<QJSEngine *>(parent());
}

QScxmlStateMachine *QScxmlPlatformProperties::stateMachine() const
{
    return data->m_stateMachine;
}

QJSValue QScxmlPlatformProperties::jsValue() const
{
    return data->m_jsValue;
}

/// _x.marks === "the spot"
QString QScxmlPlatformProperties::marks() const
{
    return QStringLiteral("the spot");
}

bool QScxmlPlatformProperties::inState(const QString &stateName)
{
    return stateMachine()->isActive(stateName);
}

QT_END_NAMESPACE
