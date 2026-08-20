// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef EVENTCONNECTION_P_H
#define EVENTCONNECTION_P_H

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
#include <QtScxml/qscxmlevent.h>
#include <QtCore/qobject.h>
#include <QtQml/qqmlparserstatus.h>
#include <QtQml/qqml.h>

#include "QtCore/qproperty.h"
#include <QtCore/qvarlengtharray.h>
#include <private/qproperty_p.h>

QT_BEGIN_NAMESPACE

// QScxmlEvent is used as signal parameter, and defined in the cpp lib
struct Q_SCXMLQML_EXPORT QScxmlEventForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QScxmlEvent)
    QML_ADDED_IN_VERSION(5,8)
};

class Q_SCXMLQML_EXPORT QScxmlEventConnection : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QStringList events READ events WRITE setEvents NOTIFY eventsChanged
               BINDABLE bindableEvents)
    Q_PROPERTY(QScxmlStateMachine *stateMachine READ stateMachine WRITE setStateMachine
               NOTIFY stateMachineChanged BINDABLE bindableStateMachine)
    Q_INTERFACES(QQmlParserStatus)
    QML_NAMED_ELEMENT(EventConnection)
    QML_ADDED_IN_VERSION(5,8)

public:
    QScxmlEventConnection(QObject *parent = nullptr);
    ~QScxmlEventConnection() override;

    QStringList events() const;
    void setEvents(const QStringList &events);
    QBindable<QStringList> bindableEvents();

    QScxmlStateMachine *stateMachine() const;
    void setStateMachine(QScxmlStateMachine *stateMachine);
    QBindable<QScxmlStateMachine*> bindableStateMachine();

Q_SIGNALS:
    void eventsChanged();
    void stateMachineChanged();

    void occurred(const QScxmlEvent &event);

private:
    Q_OBJECT_COMPAT_PROPERTY_WITH_ARGS(QScxmlEventConnection, QScxmlStateMachine*, m_stateMachine,
                                      &QScxmlEventConnection::setStateMachine,
                                      &QScxmlEventConnection::stateMachineChanged, nullptr);
    Q_OBJECT_COMPAT_PROPERTY(QScxmlEventConnection, QStringList, m_events,
                             &QScxmlEventConnection::setEvents,
                             &QScxmlEventConnection::eventsChanged);

    QVarLengthArray<QMetaObject::Connection, 4> m_connections;

    void doConnect();
    void classBegin() override;
    void componentComplete() override;
};

QT_END_NAMESPACE

#endif // EVENTCONNECTION_P_H
