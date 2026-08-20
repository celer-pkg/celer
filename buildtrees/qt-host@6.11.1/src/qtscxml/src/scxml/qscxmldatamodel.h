// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLDATAMODEL_H
#define QSCXMLDATAMODEL_H

#include <QtScxml/qscxmlexecutablecontent.h>

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>

Q_MOC_INCLUDE(qscxmlstatemachine.h)

QT_BEGIN_NAMESPACE

class QScxmlEvent;

class QScxmlStateMachine;

class QScxmlDataModelPrivate;
class Q_SCXML_EXPORT QScxmlDataModel : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScxmlDataModel)
    Q_PROPERTY(QScxmlStateMachine *stateMachine READ stateMachine WRITE setStateMachine
               NOTIFY stateMachineChanged BINDABLE bindableStateMachine)

public:
    class Q_SCXML_EXPORT ForeachLoopBody
    {
        Q_DISABLE_COPY(ForeachLoopBody)
    public:
        ForeachLoopBody();
        virtual ~ForeachLoopBody();
        virtual void run(bool *ok) = 0;
    };

public:
    explicit QScxmlDataModel(QObject *parent = nullptr);

    static QScxmlDataModel *createScxmlDataModel(const QString& pluginKey);

    void setStateMachine(QScxmlStateMachine *stateMachine);
    QScxmlStateMachine *stateMachine() const;
    QBindable<QScxmlStateMachine*> bindableStateMachine();

    Q_INVOKABLE virtual bool setup(const QVariantMap &initialDataValues) = 0;

    virtual QString evaluateToString(QScxmlExecutableContent::EvaluatorId id, bool *ok) = 0;
    virtual bool evaluateToBool(QScxmlExecutableContent::EvaluatorId id, bool *ok) = 0;
    virtual QVariant evaluateToVariant(QScxmlExecutableContent::EvaluatorId id, bool *ok) = 0;
    virtual void evaluateToVoid(QScxmlExecutableContent::EvaluatorId id, bool *ok) = 0;
    virtual void evaluateAssignment(QScxmlExecutableContent::EvaluatorId id, bool *ok) = 0;
    virtual void evaluateInitialization(QScxmlExecutableContent::EvaluatorId id, bool *ok) = 0;
    virtual void evaluateForeach(QScxmlExecutableContent::EvaluatorId id, bool *ok, ForeachLoopBody *body) = 0;

    virtual void setScxmlEvent(const QScxmlEvent &event) = 0;

    virtual QVariant scxmlProperty(const QString &name) const = 0;
    virtual bool hasScxmlProperty(const QString &name) const = 0;
    virtual bool setScxmlProperty(const QString &name, const QVariant &value, const QString &context) = 0;

Q_SIGNALS:
    void stateMachineChanged(QScxmlStateMachine *stateMachine);

protected:
    explicit QScxmlDataModel(QScxmlDataModelPrivate &dd, QObject *parent = nullptr);
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QScxmlDataModel *)

#endif // QSCXMLDATAMODEL_H
