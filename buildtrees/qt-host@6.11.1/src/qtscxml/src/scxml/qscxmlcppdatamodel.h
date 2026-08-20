// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLCPPDATAMODEL_H
#define QSCXMLCPPDATAMODEL_H

#include <QtScxml/qscxmldatamodel.h>

#define Q_SCXML_DATAMODEL \
    public: \
        QString evaluateToString(QScxmlExecutableContent::EvaluatorId id, bool *ok) override final; \
        bool evaluateToBool(QScxmlExecutableContent::EvaluatorId id, bool *ok) override final; \
        QVariant evaluateToVariant(QScxmlExecutableContent::EvaluatorId id, bool *ok) override final; \
        void evaluateToVoid(QScxmlExecutableContent::EvaluatorId id, bool *ok) override final; \
    private:

QT_BEGIN_NAMESPACE

class QScxmlCppDataModelPrivate;
class Q_SCXML_EXPORT QScxmlCppDataModel: public QScxmlDataModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScxmlCppDataModel)
public:
    explicit QScxmlCppDataModel(QObject *parent = nullptr);

    Q_INVOKABLE bool setup(const QVariantMap &initialDataValues) override;

    void evaluateAssignment(QScxmlExecutableContent::EvaluatorId id, bool *ok) override;
    void evaluateInitialization(QScxmlExecutableContent::EvaluatorId id, bool *ok) override;
    void evaluateForeach(QScxmlExecutableContent::EvaluatorId id, bool *ok, ForeachLoopBody *body) override;

    void setScxmlEvent(const QScxmlEvent &scxmlEvent) override final;
    const QScxmlEvent &scxmlEvent() const;

    QVariant scxmlProperty(const QString &name) const override;
    bool hasScxmlProperty(const QString &name) const override;
    bool setScxmlProperty(const QString &name, const QVariant &value, const QString &context) override;

    bool inState(const QString &stateName) const;
};

QT_END_NAMESPACE

#endif // QSCXMLCPPDATAMODEL_H
