// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLINVOKABLESERVICE_P_H
#define QSCXMLINVOKABLESERVICE_P_H

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

#include "qscxmlinvokableservice.h"
#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QScxmlInvokableServicePrivate : public QObjectPrivate
{
public:
    QScxmlInvokableServicePrivate(QScxmlStateMachine *parentStateMachine);

    QString calculateId(QScxmlStateMachine *parent,
                        const QScxmlExecutableContent::InvokeInfo &invokeInfo, bool *ok) const;
    QVariantMap calculateData(QScxmlStateMachine *parent,
                              const QList<QScxmlExecutableContent::ParameterInfo> &parameters,
                              const QList<QScxmlExecutableContent::StringId> &names,
                              bool *ok) const;

    QScxmlStateMachine *parentStateMachine;
};

class QScxmlInvokableServiceFactoryPrivate : public QObjectPrivate
{
public:
    QScxmlInvokableServiceFactoryPrivate(
            const QScxmlExecutableContent::InvokeInfo &invokeInfo,
            const QList<QScxmlExecutableContent::StringId> &names,
            const QList<QScxmlExecutableContent::ParameterInfo> &parameters);

    QScxmlExecutableContent::InvokeInfo invokeInfo;
    QList<QScxmlExecutableContent::StringId> names;
    QList<QScxmlExecutableContent::ParameterInfo> parameters;
};

class Q_SCXML_EXPORT QScxmlScxmlService: public QScxmlInvokableService
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScxmlInvokableService)
    Q_PROPERTY(QScxmlStateMachine *stateMachine READ stateMachine CONSTANT)
public:
    QScxmlScxmlService(QScxmlStateMachine *stateMachine,
                       QScxmlStateMachine *parentStateMachine,
                       QScxmlInvokableServiceFactory *parent);
    ~QScxmlScxmlService();

    bool start() override;
    QString id() const override;
    QString name() const override;
    void postEvent(QScxmlEvent *event) override;
    QScxmlStateMachine *stateMachine() const;

private:
    QScxmlStateMachine *m_stateMachine;
};

class QScxmlStaticScxmlServiceFactoryPrivate : public QScxmlInvokableServiceFactoryPrivate
{
public:
    QScxmlStaticScxmlServiceFactoryPrivate(
            const QMetaObject *metaObject,
            const QScxmlExecutableContent::InvokeInfo &invokeInfo,
            const QList<QScxmlExecutableContent::StringId> &names,
            const QList<QScxmlExecutableContent::ParameterInfo> &parameters);

    const QMetaObject *metaObject;
};

QScxmlScxmlService *invokeDynamicScxmlService(const QString &sourceUrl,
                                              QScxmlStateMachine *parentStateMachine,
                                              QScxmlInvokableServiceFactory *factory);
QScxmlScxmlService *invokeStaticScxmlService(QScxmlStateMachine *childStateMachine,
                                             QScxmlStateMachine *parentStateMachine,
                                             QScxmlInvokableServiceFactory *factory);
QString calculateSrcexpr(QScxmlStateMachine *parent, QScxmlExecutableContent::EvaluatorId srcexpr,
                         bool *ok);

QT_END_NAMESPACE

#endif // QSCXMLINVOKABLESERVICE_P_H
