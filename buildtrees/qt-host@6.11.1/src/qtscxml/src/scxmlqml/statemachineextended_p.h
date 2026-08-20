// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef STATEMACHINEEXTENDED_P_H
#define STATEMACHINEEXTENDED_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qscxmlqmlglobals_p.h"

#include <QtScxml/qscxmlstatemachine.h>
#include <QtCore/qobject.h>
#include <QtQml/qqmllist.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

/* Allow State Machines created from QML to have children. */
class Q_SCXMLQML_EXPORT QScxmlStateMachineExtended : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> children READ children)
    Q_CLASSINFO("DefaultProperty", "children")
public:
    QScxmlStateMachineExtended(QObject *extendee);
    QQmlListProperty<QObject> children();

private:
    QObjectList m_children;
};

// The QScxmlStateMachine is defined in the scxml library
struct Q_SCXMLQML_EXPORT QScxmlStateMachineForeign
{
    Q_GADGET
    QML_UNCREATABLE("Only created through derived types")
    QML_NAMED_ELEMENT(StateMachine)
    QML_FOREIGN(QScxmlStateMachine)
    QML_EXTENDED(QScxmlStateMachineExtended)
    QML_ADDED_IN_VERSION(5,8)
};

QT_END_NAMESPACE

#endif // STATEMACHINEEXTENDED_P_H
