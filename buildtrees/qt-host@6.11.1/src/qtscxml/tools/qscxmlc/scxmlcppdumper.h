// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SCXMLCPPDUMPER_H
#define SCXMLCPPDUMPER_H

#include <QtScxml/qscxmlglobals.h>
#include <QtScxml/private/qscxmlcompiler_p.h>
#include <QtScxml/private/qscxmltabledata_p.h>

#include <QTextStream>

QT_BEGIN_NAMESPACE

struct TranslationUnit
{
    TranslationUnit()
        : stateMethods(false)
        , mainDocument(nullptr)
    {}

    QString scxmlFileName;
    QString outHFileName, outCppFileName;
    QString namespaceName;
    bool stateMethods;
    DocumentModel::ScxmlDocument *mainDocument;
    QList<DocumentModel::ScxmlDocument *> allDocuments;
    QHash<DocumentModel::ScxmlDocument *, QString> classnameForDocument;
    QList<TranslationUnit *> dependencies;
};

class CppDumper
{
public:
    CppDumper(QTextStream &headerStream, QTextStream &cppStream)
        : h(headerStream)
        , cpp(cppStream)
    {}

    void dump(TranslationUnit *unit);

private:
    void writeHeaderStart(const QString &headerGuard, const QStringList &forwardDecls);
    void writeClass(const QString &className,
                    const QScxmlInternal::GeneratedTableData::MetaDataInfo &info);
    void writeHeaderEnd(const QString &headerGuard, const QStringList &metatypeDecls);
    void writeImplStart();
    void writeImplBody(const QScxmlInternal::GeneratedTableData &table,
                       const QString &className,
                       DocumentModel::ScxmlDocument *doc,
                       const QStringList &factory,
                       const QScxmlInternal::GeneratedTableData::MetaDataInfo &info);
    void writeImplEnd();
    QString mangleIdentifier(const QString &str);

private:
    QString generatePropertyDecls(const QScxmlInternal::GeneratedTableData::MetaDataInfo &info);
    QString generateAccessorDecls(const QScxmlInternal::GeneratedTableData::MetaDataInfo &info);
    QString generateSignalDecls(const QScxmlInternal::GeneratedTableData::MetaDataInfo &info);
    QString generateMetaObject(const QString &className,
                               const QScxmlInternal::GeneratedTableData::MetaDataInfo &info);

    QTextStream &h;
    QTextStream &cpp;

    static QByteArray b(const char *str) { return QByteArray(str); }
    static QLatin1String l (const char *str) { return QLatin1String(str); }

    TranslationUnit *m_translationUnit;

    mutable QHash<QString, QString> m_mangledToOriginal;
};

QT_END_NAMESPACE

#endif // SCXMLCPPDUMPER_H
