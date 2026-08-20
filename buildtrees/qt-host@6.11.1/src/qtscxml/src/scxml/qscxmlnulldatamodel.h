// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLNULLDATAMODEL_H
#define QSCXMLNULLDATAMODEL_H

#include <QtScxml/qscxmldatamodel.h>

QT_BEGIN_NAMESPACE

class QScxmlNullDataModelPrivate;
class Q_SCXML_EXPORT QScxmlNullDataModel: public QScxmlDataModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScxmlNullDataModel)
public:
    explicit QScxmlNullDataModel(QObject *parent = nullptr);
    ~QScxmlNullDataModel();

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

#endif // QSCXMLNULLDATAMODEL_H
