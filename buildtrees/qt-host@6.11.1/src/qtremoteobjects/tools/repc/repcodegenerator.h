// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef REPCODEGENERATOR_H
#define REPCODEGENERATOR_H

#include "repparser.h"

#include <QList>
#include <QSet>
#include <QString>
#include <QTextStream>

QT_BEGIN_NAMESPACE

class QIODevice;

class RepCodeGenerator
{
public:
    enum Mode
    {
        REPLICA,
        SOURCE,
        SIMPLE_SOURCE,
        MERGED
    };

    RepCodeGenerator(QIODevice *outputDevice, const AST &ast);

    void generate(Mode mode, QString fileName);

    QByteArray classSignature(const ASTClass &ac);
private:
    void generateHeader(Mode mode);
    QString generateMetaTypeRegistration(const QSet<QString> &metaTypes);
    QString generateMetaTypeRegistrationForPending(const QSet<QString> &metaTypes);

    void generateSimpleSetter(const ASTProperty &property, bool generateOverride = true);
    void generatePOD(const POD &pod);
    void generateEnumGadget(const ASTEnum &en, const QString &className);
    void generateDeclarationsForEnums(const QList<ASTEnum> &enums, bool generateQENUM=true);
    QString formatQPropertyDeclarations(const POD &pod);
    QString formatConstructors(const POD &pod);
    QString formatPropertyGettersAndSetters(const POD &pod);
    QString formatSignals(const POD &pod);
    QString formatDataMembers(const POD &pod);
    QString formatDebugOperator(const POD &pod);
    QString formatMarshallingOperators(const POD &pod);
    QString typeForMode(const ASTProperty &property, Mode mode);

    void generateClass(Mode mode, const ASTClass &astClasses,
                       const QString &metaTypeRegistrationCode);
    void generateSourceAPI(const ASTClass &astClass);

private:
    QTextStream m_stream;
    AST m_ast;
};

QT_END_NAMESPACE

#endif
