// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QSCXMLECMASCRIPTDATAMODEL_P_H
#define QSCXMLECMASCRIPTDATAMODEL_P_H

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

#include <QObject>
#include <QtScxml/qscxmlglobals.h>
#include <QtScxml/qscxmldatamodel.h>

QT_BEGIN_NAMESPACE


class QScxmlEcmaScriptDataModelPrivate;
class QScxmlEcmaScriptDataModel: public QScxmlDataModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScxmlEcmaScriptDataModel)
public:
    explicit QScxmlEcmaScriptDataModel(QObject *parent = nullptr);

    Q_INVOKABLE bool setup(const QVariantMap &initialDataValues) override;

    QString evaluateToString(QScxmlExecutableContent::EvaluatorId id, bool *ok) override final;
    bool evaluateToBool(QScxmlExecutableContent::EvaluatorId id, bool *ok) override final;
    QVariant evaluateToVariant(QScxmlExecutableContent::EvaluatorId id, bool *ok) override final;
    void evaluateToVoid(QScxmlExecutableContent::EvaluatorId id, bool *ok) override final;
    void evaluateAssignment(QScxmlExecutableContent::EvaluatorId id, bool *ok) override final;
    void evaluateInitialization(QScxmlExecutableContent::EvaluatorId id, bool *ok) override final;
    void evaluateForeach(QScxmlExecutableContent::EvaluatorId id, bool *ok, ForeachLoopBody *body) override final;

    void setScxmlEvent(const QScxmlEvent &event) override;

    QVariant scxmlProperty(const QString &name) const override;
    bool hasScxmlProperty(const QString &name) const override;
    bool setScxmlProperty(const QString &name, const QVariant &value, const QString &context) override;
};

QT_END_NAMESPACE

#endif // QSCXMLECMASCRIPTDATAMODEL_P_H
