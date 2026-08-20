// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "statemachineextended_p.h"

#include <QtScxml/qscxmlglobals.h>
#include <QtScxml/qscxmlstatemachine.h>

QT_BEGIN_NAMESPACE

QScxmlStateMachineExtended::QScxmlStateMachineExtended(QObject *extendee) :
    QObject(extendee)
{
}

QQmlListProperty<QObject> QScxmlStateMachineExtended::children()
{
    return QQmlListProperty<QObject>(this, &m_children);
}

QT_END_NAMESPACE
