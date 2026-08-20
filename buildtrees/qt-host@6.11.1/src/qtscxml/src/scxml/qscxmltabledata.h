// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLTABLEDATA_H
#define QSCXMLTABLEDATA_H

#include <QtScxml/qscxmlexecutablecontent.h>
#include <QtCore/qstring.h>

#ifndef Q_QSCXMLC_OUTPUT_REVISION
#define Q_QSCXMLC_OUTPUT_REVISION 2
#endif

QT_BEGIN_NAMESPACE

class QScxmlInvokableServiceFactory;

class Q_SCXML_EXPORT QScxmlTableData
{
public:
    virtual ~QScxmlTableData();

    virtual QString string(QScxmlExecutableContent::StringId id) const = 0;
    virtual QScxmlExecutableContent::InstructionId *instructions() const = 0;
    virtual QScxmlExecutableContent::EvaluatorInfo evaluatorInfo(QScxmlExecutableContent::EvaluatorId evaluatorId) const = 0;
    virtual QScxmlExecutableContent::AssignmentInfo assignmentInfo(QScxmlExecutableContent::EvaluatorId assignmentId) const = 0;
    virtual QScxmlExecutableContent::ForeachInfo foreachInfo(QScxmlExecutableContent::EvaluatorId foreachId) const = 0;
    virtual QScxmlExecutableContent::StringId *dataNames(int *count) const = 0;

    virtual QScxmlExecutableContent::ContainerId initialSetup() const = 0;
    virtual QString name() const = 0;

    virtual const qint32 *stateMachineTable() const = 0;
    virtual QScxmlInvokableServiceFactory *serviceFactory(int id) const = 0;
};

QT_END_NAMESPACE

#endif // QSCXMLTABLEDATA_H
