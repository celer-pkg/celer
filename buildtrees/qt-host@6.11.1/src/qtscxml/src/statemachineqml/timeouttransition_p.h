// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef TIMEOUTTRANSITION_H
#define TIMEOUTTRANSITION_H

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

#include "qstatemachineqmlglobals_p.h"

#include <QtStateMachine/QSignalTransition>
#include <QtQml/QQmlParserStatus>
#include <QtQml/qqml.h>
#include <QtCore/private/qproperty_p.h>

QT_BEGIN_NAMESPACE
class QTimer;

class Q_STATEMACHINEQML_EXPORT TimeoutTransition : public QSignalTransition, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout BINDABLE bindableTimeout)
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT
    QML_ADDED_IN_VERSION(1, 0)

public:
    TimeoutTransition(QState *parent = nullptr);
    ~TimeoutTransition();

    int timeout() const;
    void setTimeout(int timeout);
    QBindable<int> bindableTimeout();

    void classBegin() override {}
    void componentComplete() override;

private:
    QTimer *m_timer;
};

QT_END_NAMESPACE

#endif
