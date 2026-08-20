// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef TST_TYPEDJSON_H
#define TST_TYPEDJSON_H

#include <QtJsonRpc/private/qtypedjson_p.h>
#include <QtTest/QtTest>
#include <QCborValue>
#include <QDebug>
#include <QLibraryInfo>
#include <QByteArray>

#include <memory>

namespace TestSpec {

class Position
{
public:
    int line = {};
    int character = {};

    template<typename W>
    void walk(W &w)
    {
        field(w, "line", line);
        field(w, "character", character);
    }
};

class Range
{
public:
    Position start = {};
    Position end = {};

    template<typename W>
    void walk(W &w)
    {
        field(w, "start", start);
        field(w, "end", end);
    }
};

using ProgressToken = std::variant<int, QByteArray>;

class ReferenceContext
{
public:
    bool includeDeclaration = {};

    template<typename W>
    void walk(W &w)
    {
        field(w, "includeDeclaration", includeDeclaration);
    }
};

class WorkDoneProgressParams
{
public:
    std::optional<ProgressToken> workDoneToken = {};

    template<typename W>
    void walk(W &w)
    {
        field(w, "workDoneToken", workDoneToken);
    }
};

class TextDocumentIdentifier
{
public:
    QByteArray uri = {};

    template<typename W>
    void walk(W &w)
    {
        field(w, "uri", uri);
    }
};

class TextDocumentPositionParams
{
public:
    TextDocumentIdentifier textDocument = {};
    Position position = {};

    template<typename W>
    void walk(W &w)
    {
        field(w, "textDocument", textDocument);
        field(w, "position", position);
    }
};

class PartialResultParams
{
public:
    std::optional<ProgressToken> partialResultToken = {};

    template<typename W>
    void walk(W &w)
    {
        field(w, "partialResultToken", partialResultToken);
    }
};

class ReferenceParams : public TextDocumentPositionParams,
                        WorkDoneProgressParams,
                        PartialResultParams
{
public:
    ReferenceContext context = {};

    template<typename W>
    void walk(W &w)
    {
        TextDocumentPositionParams::walk(w);
        WorkDoneProgressParams::walk(w);
        PartialResultParams::walk(w);
        field(w, "context", context);
    }
};

class TextDocumentEdit
{
public:
    TextDocumentIdentifier textDocument = {};

    template<typename W>
    void walk(W &w)
    {
        field(w, "textDocument", textDocument);
    }
};
class WorkspaceEdit
{
public:
    std::optional<
            std::variant<QList<TextDocumentEdit>, QList<std::variant<TextDocumentEdit, Position>>>>
            documentChanges = {};

    template<typename W>
    void walk(W &w)
    {
        field(w, "documentChanges", documentChanges);
    }
};
class PatchedWorkspaceEdit
{
public:
    std::optional<QList<std::variant<TextDocumentEdit, Position>>> documentChanges = {};

    template<typename W>
    void walk(W &w)
    {
        field(w, "documentChanges", documentChanges);
    }
};
} // namespace TestSpec

QT_BEGIN_NAMESPACE
namespace QTypedJson {
using namespace Qt::StringLiterals;

class TestTypedJson : public QObject
{
    Q_OBJECT
    QJsonObject loadJson(QString filePath)
    {
        QFile f(filePath);
        if (!f.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed opening" << filePath << "due to" << f.errorString();
            Q_ASSERT(false);
        }
        QByteArray data = f.readAll();
        QJsonParseError error;
        QJsonDocument json = QJsonDocument::fromJson(data, &error);
        if (json.isNull()) {
            qWarning() << "Error reading json from" << filePath << ": " << error.errorString();
            Q_ASSERT(false);
        }
        return json.object();
    }
    template<typename T>
    void testT(QString file, T &value, bool checkJson = true)
    {
        QJsonObject obj = loadJson(file);
        QTypedJson::Reader r(obj);
        QTypedJson::doWalk(r, value);
        QJsonObject obj2B = toJsonValue(value).toObject();
        QByteArray json2B = QJsonDocument(obj2B).toJson();
        if (checkJson) {
            QByteArray json1 = QJsonDocument(obj).toJson();
            QCOMPARE(json1, json2B);
        }
        QTypedJson::Reader r2B(obj2B);
        T value2B;
        QTypedJson::doWalk(r2B, value2B);
        QJsonObject obj3 = toJsonValue(value2B).toObject();
        QByteArray json3 = QJsonDocument(obj3).toJson();
        QCOMPARE(json2B, json3);
    }
private slots:

    void testJson()
    {
        QString baseDir = QLatin1String(QT_TYPEDJSON_DATADIR);
        QString f1 = baseDir + QLatin1String("/Range.json");
        TestSpec::Range value;
        testT(f1, value);
        QCOMPARE(value.start.line, 5);
        QCOMPARE(value.start.character, 23);
        QCOMPARE(value.end.line, 6);
        QCOMPARE(value.end.character, 0);
        QString f2 = baseDir + QLatin1String("/ReferenceParams.json");
        TestSpec::ReferenceParams value2;
        testT(f2, value2);
        QCOMPARE(value2.textDocument.uri, QByteArray("file:///folder/file.ts"));
        QCOMPARE(value2.position.line, 9);
        QCOMPARE(value2.position.character, 5);
    }

    void qtbug124592()
    {
        const QString jsonList{ uR"({
"documentChanges" : [
    { "textDocument": { "uri": "a" } },
    { "textDocument": { "uri": "b" } },
    { "textDocument": { "uri": "c" } }
] })"_s };
        const QString jsonListOfVariants{ uR"({
"documentChanges" : [
    { "textDocument": { "uri": "a" } },
    { "textDocument": { "uri": "b" } },
    { "textDocument": { "uri": "c" } },
    { "line": 5, "character": 6 }
] })"_s };

        QJsonDocument list = QJsonDocument::fromJson(jsonList.toUtf8());
        QVERIFY(!list.isNull());
        QJsonDocument listOfVariants = QJsonDocument::fromJson(jsonListOfVariants.toUtf8());
        QVERIFY(!listOfVariants.isNull());

        {
            TestSpec::WorkspaceEdit edit;
            QTypedJson::Reader r(list.object());
            QTypedJson::doWalk(r, edit);
            QVERIFY(edit.documentChanges);
            QVERIFY(std::holds_alternative<QList<TestSpec::TextDocumentEdit>>(
                    *edit.documentChanges));
            auto list = std::get<QList<TestSpec::TextDocumentEdit>>(*edit.documentChanges);
            QCOMPARE(list.size(), 3);
            QCOMPARE(list[0].textDocument.uri, u"a"_s);
            QCOMPARE(list[1].textDocument.uri, u"b"_s);
            QCOMPARE(list[2].textDocument.uri, u"c"_s);
        }

        {
            TestSpec::WorkspaceEdit edit;
            QTypedJson::Reader r(listOfVariants.object());
            QTypedJson::doWalk(r, edit);
            QVERIFY(edit.documentChanges);
            auto isVariant = std::holds_alternative<
                    QList<std::variant<TestSpec::TextDocumentEdit, TestSpec::Position>>>(
                    *edit.documentChanges);
            QVERIFY(isVariant);

            auto list =
                    std::get<QList<std::variant<TestSpec::TextDocumentEdit, TestSpec::Position>>>(
                            *edit.documentChanges);
            QCOMPARE(list.size(), 4);
            QVERIFY(std::holds_alternative<TestSpec::TextDocumentEdit>(list[0]));
            QCOMPARE(std::get<TestSpec::TextDocumentEdit>(list[0]).textDocument.uri, u"a"_s);
            QVERIFY(std::holds_alternative<TestSpec::TextDocumentEdit>(list[1]));
            QCOMPARE(std::get<TestSpec::TextDocumentEdit>(list[1]).textDocument.uri, u"b"_s);
            QVERIFY(std::holds_alternative<TestSpec::TextDocumentEdit>(list[2]));
            QCOMPARE(std::get<TestSpec::TextDocumentEdit>(list[2]).textDocument.uri, u"c"_s);
            QVERIFY(std::holds_alternative<TestSpec::Position>(list[3]));
            QCOMPARE(std::get<TestSpec::Position>(list[3]).line, 5);
            QCOMPARE(std::get<TestSpec::Position>(list[3]).character, 6);
        }
    }

    void equivalenceListAndListOfVariants()
    {
        const QString jsonList{ uR"({
"documentChanges" : [
    { "textDocument": { "uri": "a" } },
    { "textDocument": { "uri": "b" } },
    { "textDocument": { "uri": "c" } }
] })"_s };

        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonList.toUtf8());
        QVERIFY(!jsonDocument.isNull());

        // show that the list of variant can serialize from the list of TextDocumentEdit
        TestSpec::PatchedWorkspaceEdit hasListOfVariant;
        QTypedJson::Reader r(jsonDocument.object());
        QTypedJson::doWalk(r, hasListOfVariant);
        QVERIFY(hasListOfVariant.documentChanges);
        auto list = *hasListOfVariant.documentChanges;
        QCOMPARE(list.size(), 3);
        QVERIFY(std::holds_alternative<TestSpec::TextDocumentEdit>(list[0]));
        QCOMPARE(std::get<TestSpec::TextDocumentEdit>(list[0]).textDocument.uri, u"a"_s);
        QVERIFY(std::holds_alternative<TestSpec::TextDocumentEdit>(list[1]));
        QCOMPARE(std::get<TestSpec::TextDocumentEdit>(list[1]).textDocument.uri, u"b"_s);
        QVERIFY(std::holds_alternative<TestSpec::TextDocumentEdit>(list[2]));
        QCOMPARE(std::get<TestSpec::TextDocumentEdit>(list[2]).textDocument.uri, u"c"_s);

        // show that the list of variant can deserialize into the list of TextDocumentEdit
        TestSpec::WorkspaceEdit hasListOfTextDocuments;
        QTypedJson::Reader r2(jsonDocument.object());
        QTypedJson::doWalk(r2, hasListOfTextDocuments);
        QCOMPARE(toJsonValue(hasListOfVariant), toJsonValue(hasListOfTextDocuments));
    }
};

} // namespace QTypedJson
QT_END_NAMESPACE

#endif // TST_TYPEDJSON_H
