// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSTATEMACHINE_H
#define QSTATEMACHINE_H

#include <QtCore/qcoreevent.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qset.h>
#include <QtCore/qvariant.h>

#include <QtStateMachine/qstate.h>

#if __has_include(<chrono>)
#    include <chrono>
#endif

QT_BEGIN_NAMESPACE

class QStateMachinePrivate;
class QAbstractAnimation;
class Q_STATEMACHINE_EXPORT QStateMachine : public QState
{
    Q_OBJECT
    Q_PROPERTY(QString errorString READ errorString BINDABLE bindableErrorString)
    Q_PROPERTY(QState::RestorePolicy globalRestorePolicy READ globalRestorePolicy
               WRITE setGlobalRestorePolicy BINDABLE bindableGlobalRestorePolicy)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
#if QT_CONFIG(animation)
    Q_PROPERTY(bool animated READ isAnimated WRITE setAnimated BINDABLE bindableAnimated)
#endif
public:
    class Q_STATEMACHINE_EXPORT SignalEvent : public QEvent
    {
    public:
        SignalEvent(QObject *sender, int signalIndex,
                     const QList<QVariant> &arguments);
        ~SignalEvent();

        inline QObject *sender() const { return m_sender; }
        inline int signalIndex() const { return m_signalIndex; }
        inline QList<QVariant> arguments() const { return m_arguments; }

    private:
        QObject *m_sender;
        int m_signalIndex;
        QList<QVariant> m_arguments;

        friend class QSignalTransitionPrivate;
    };

    class Q_STATEMACHINE_EXPORT WrappedEvent : public QEvent
    {
    public:
        WrappedEvent(QObject *object, QEvent *event);
        ~WrappedEvent();

        inline QObject *object() const { return m_object; }
        inline QEvent *event() const { return m_event; }

    private:
        QObject *m_object;
        QEvent *m_event;
    };

    enum EventPriority {
        NormalPriority,
        HighPriority
    };

    enum Error {
        NoError,
        NoInitialStateError,
        NoDefaultStateInHistoryStateError,
        NoCommonAncestorForTransitionError,
        StateMachineChildModeSetToParallelError
    };

    explicit QStateMachine(QObject *parent = nullptr);
    explicit QStateMachine(QState::ChildMode childMode, QObject *parent = nullptr);
    ~QStateMachine();

    void addState(QAbstractState *state);
    void removeState(QAbstractState *state);

    Error error() const;

    QString errorString() const;
    void clearError();
    QBindable<QString> bindableErrorString() const;

    bool isRunning() const;

#if QT_CONFIG(animation)
    bool isAnimated() const;
    void setAnimated(bool enabled);
    QBindable<bool> bindableAnimated();

    void addDefaultAnimation(QAbstractAnimation *animation);
    QList<QAbstractAnimation *> defaultAnimations() const;
    void removeDefaultAnimation(QAbstractAnimation *animation);
#endif // animation

    QState::RestorePolicy globalRestorePolicy() const;
    void setGlobalRestorePolicy(QState::RestorePolicy restorePolicy);
    QBindable<QState::RestorePolicy> bindableGlobalRestorePolicy();

    void postEvent(QEvent *event, EventPriority priority = NormalPriority);
    int postDelayedEvent(QEvent *event, int delay);
    bool cancelDelayedEvent(int id);

    QSet<QAbstractState*> configuration() const;

    bool eventFilter(QObject *watched, QEvent *event) override;

#if __has_include(<chrono>) || defined(Q_QDOC)
    int postDelayedEvent(QEvent *event, std::chrono::milliseconds delay)
    {
        return postDelayedEvent(event, int(delay.count()));
    }
#endif

public Q_SLOTS:
    void start();
    void stop();
    void setRunning(bool running);

Q_SIGNALS:
    void started(QPrivateSignal);
    void stopped(QPrivateSignal);
    void runningChanged(bool running);


protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

    virtual void beginSelectTransitions(QEvent *event);
    virtual void endSelectTransitions(QEvent *event);

    virtual void beginMicrostep(QEvent *event);
    virtual void endMicrostep(QEvent *event);

    bool event(QEvent *e) override;

protected:
    QStateMachine(QStateMachinePrivate &dd, QObject *parent);

private:
    Q_DISABLE_COPY(QStateMachine)
    Q_DECLARE_PRIVATE(QStateMachine)
    Q_PRIVATE_SLOT(d_func(), void _q_start())
    Q_PRIVATE_SLOT(d_func(), void _q_process())
#if QT_CONFIG(animation)
    Q_PRIVATE_SLOT(d_func(), void _q_animationFinished())
#endif
    Q_PRIVATE_SLOT(d_func(), void _q_startDelayedEventTimer(int, int))
    Q_PRIVATE_SLOT(d_func(), void _q_killDelayedEventTimer(int, int))
};

QT_END_NAMESPACE

#endif
