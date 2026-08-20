// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSTATE_H
#define QSTATE_H

#include <QtCore/qlist.h>
#include <QtCore/qmetaobject.h>

#include <QtStateMachine/qabstractstate.h>

QT_BEGIN_NAMESPACE

class QAbstractTransition;
class QSignalTransition;

class QStatePrivate;
class Q_STATEMACHINE_EXPORT QState : public QAbstractState
{
    Q_OBJECT
    Q_PROPERTY(QAbstractState* initialState READ initialState WRITE setInitialState
               NOTIFY initialStateChanged BINDABLE bindableInitialState)
    Q_PROPERTY(QAbstractState* errorState READ errorState WRITE setErrorState
               NOTIFY errorStateChanged BINDABLE bindableErrorState)
    Q_PROPERTY(ChildMode childMode READ childMode WRITE setChildMode
               NOTIFY childModeChanged BINDABLE bindableChildMode)
public:
    enum ChildMode {
        ExclusiveStates,
        ParallelStates
    };
    Q_ENUM(ChildMode)

    enum RestorePolicy {
        DontRestoreProperties,
        RestoreProperties
    };
    Q_ENUM(RestorePolicy)

    QState(QState *parent = nullptr);
    QState(ChildMode childMode, QState *parent = nullptr);
    ~QState();

    QAbstractState *errorState() const;
    void setErrorState(QAbstractState *state);
    QBindable<QAbstractState*> bindableErrorState();

    void addTransition(QAbstractTransition *transition);
    QSignalTransition *addTransition(const QObject *sender, const char *signal, QAbstractState *target);
#ifdef Q_QDOC
    template<typename PointerToMemberFunction>
    QSignalTransition *addTransition(const QObject *sender, PointerToMemberFunction signal,
                       QAbstractState *target);
#else
    template <typename Func>
    QSignalTransition *addTransition(const typename QtPrivate::FunctionPointer<Func>::Object *obj,
                      Func signal, QAbstractState *target)
    {
        const QMetaMethod signalMetaMethod = QMetaMethod::fromSignal(signal);
        return addTransition(obj, signalMetaMethod.methodSignature().constData(), target);
    }
#endif // Q_QDOC
    QAbstractTransition *addTransition(QAbstractState *target);
    void removeTransition(QAbstractTransition *transition);
    QList<QAbstractTransition*> transitions() const;

    QAbstractState *initialState() const;
    void setInitialState(QAbstractState *state);
    QBindable<QAbstractState*> bindableInitialState();

    ChildMode childMode() const;
    void setChildMode(ChildMode mode);
    QBindable<QState::ChildMode> bindableChildMode();

#ifndef QT_NO_PROPERTIES
    void assignProperty(QObject *object, const char *name,
                        const QVariant &value);
#endif

Q_SIGNALS:
    void finished(QPrivateSignal);
    void propertiesAssigned(QPrivateSignal);
    void childModeChanged(QPrivateSignal);
    void initialStateChanged(QPrivateSignal);
    void errorStateChanged(QPrivateSignal);

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

    bool event(QEvent *e) override;

protected:
    QState(QStatePrivate &dd, QState *parent);

private:
    Q_DISABLE_COPY(QState)
    Q_DECLARE_PRIVATE(QState)
};

QT_END_NAMESPACE

#endif
