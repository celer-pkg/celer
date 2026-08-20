// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtStateMachine/QAbstractTransition>
#include <QtStateMachine/QState>
#include <QtStateMachine/QStateMachine>

static constexpr QEvent::Type PingEventType = QEvent::Type(QEvent::User + 2);
static constexpr QEvent::Type PongEventType = QEvent::Type(QEvent::User + 3);

//! [0]
class PingEvent : public QEvent
{
public:
    PingEvent() : QEvent(PingEventType) { }
};

class PongEvent : public QEvent
{
public:
    PongEvent() : QEvent(PongEventType) { }
};
//! [0]

//! [1]
class Pinger : public QState
{
public:
    explicit Pinger(QState *parent) : QState(parent) { }

protected:
    void onEntry(QEvent *) override
    {
        machine()->postEvent(new PingEvent);
        qInfo() << "ping?";
    }
};
//! [1]

//! [3]
class PongTransition : public QAbstractTransition
{
public:
    PongTransition() {}

protected:
    bool eventTest(QEvent *e) override { return (e->type() == PingEventType); }

    void onTransition(QEvent *) override
    {
        machine()->postDelayedEvent(new PingEvent, 500);
        qInfo() << "ping?";
    }
};
//! [3]

//! [2]
class PingTransition : public QAbstractTransition
{
public:
    PingTransition() {}

protected:
    bool eventTest(QEvent *e) override { return e->type() == PingEventType; }

    void onTransition(QEvent *) override
    {
        machine()->postDelayedEvent(new PongEvent, 500);
        qInfo() << "pong!";
    }
};
//! [2]

//! [4]
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QStateMachine machine;
    auto group = new QState(QState::ParallelStates);
    group->setObjectName("group");
//! [4]

//! [5]
    auto pinger = new Pinger(group);
    pinger->setObjectName("pinger");
    pinger->addTransition(new PongTransition);

    auto ponger = new QState(group);
    ponger->setObjectName("ponger");
    ponger->addTransition(new PingTransition);
//! [5]

//! [6]
    machine.addState(group);
    machine.setInitialState(group);
    machine.start();

    return app.exec();
}
//! [6]
