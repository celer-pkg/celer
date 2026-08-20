// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLINVOKABLESERVICE_H
#define QSCXMLINVOKABLESERVICE_H

#include <QtScxml/qscxmldatamodel.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QScxmlEvent;
class QScxmlStateMachine;

class QScxmlInvokableServiceFactory;
class QScxmlInvokableServicePrivate;
class Q_SCXML_EXPORT QScxmlInvokableService : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScxmlInvokableService)
    Q_PROPERTY(QScxmlStateMachine *parentStateMachine READ parentStateMachine CONSTANT)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    QScxmlInvokableService(QScxmlStateMachine *parentStateMachine,
                           QScxmlInvokableServiceFactory *parent);

    QScxmlStateMachine *parentStateMachine() const;

    virtual bool start() = 0;
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual void postEvent(QScxmlEvent *event) = 0;
};

class QScxmlInvokableServiceFactoryPrivate;
class Q_SCXML_EXPORT QScxmlInvokableServiceFactory : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScxmlInvokableServiceFactory)
    Q_PROPERTY(QScxmlExecutableContent::InvokeInfo invokeInfo READ invokeInfo CONSTANT)
    Q_PROPERTY(QList<QScxmlExecutableContent::ParameterInfo> parameters READ parameters CONSTANT)
    Q_PROPERTY(QList<QScxmlExecutableContent::StringId> names READ names CONSTANT)

public:
    QScxmlInvokableServiceFactory(
            const QScxmlExecutableContent::InvokeInfo &invokeInfo,
            const QList<QScxmlExecutableContent::StringId> &names,
            const QList<QScxmlExecutableContent::ParameterInfo> &parameters,
            QObject *parent = nullptr);

    virtual QScxmlInvokableService *invoke(QScxmlStateMachine *parentStateMachine) = 0;
    const QScxmlExecutableContent::InvokeInfo &invokeInfo() const;
    const QList<QScxmlExecutableContent::ParameterInfo> &parameters() const;
    const QList<QScxmlExecutableContent::StringId> &names() const;

protected:
    QScxmlInvokableServiceFactory(QScxmlInvokableServiceFactoryPrivate &dd, QObject *parent);
};

class QScxmlStaticScxmlServiceFactoryPrivate;
class Q_SCXML_EXPORT QScxmlStaticScxmlServiceFactory: public QScxmlInvokableServiceFactory
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScxmlStaticScxmlServiceFactory)
public:
    QScxmlStaticScxmlServiceFactory(
            const QMetaObject *metaObject,
            const QScxmlExecutableContent::InvokeInfo &invokeInfo,
            const QList<QScxmlExecutableContent::StringId> &nameList,
            const QList<QScxmlExecutableContent::ParameterInfo> &parameters,
            QObject *parent = nullptr);

    QScxmlInvokableService *invoke(QScxmlStateMachine *parentStateMachine) override;
};

class Q_SCXML_EXPORT QScxmlDynamicScxmlServiceFactory: public QScxmlInvokableServiceFactory
{
    Q_OBJECT
public:
    QScxmlDynamicScxmlServiceFactory(
            const QScxmlExecutableContent::InvokeInfo &invokeInfo,
            const QList<QScxmlExecutableContent::StringId> &names,
            const QList<QScxmlExecutableContent::ParameterInfo> &parameters,
            QObject *parent = nullptr);

    QScxmlInvokableService *invoke(QScxmlStateMachine *parentStateMachine) override;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QScxmlInvokableService *)

#endif // QSCXMLINVOKABLESERVICE_H
