// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>

#include <QtCore/qjsonarray.h>
#include <QVariantMap>
#include <qjsondocument.h>
#include <qjsonobject.h>

class BenchmarkQtJson: public QObject
{
    Q_OBJECT
public:
    BenchmarkQtJson(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void parseNumbers();
    void parseJson();
    void parseJsonToVariant();

    void jsonObjectInsert();
    void variantMapInsert();

    void iteratorKey()
    {
        iteratorKeyImpl([](auto it) { return it.key(); },
                        [](auto it) { return it->toString(); });
    }
    void iteratorKeyView()
    {
        iteratorKeyImpl([](auto it) { return it.keyView(); },
                        [](auto it) { return it->toString(); });
    }

    void iteratorKeyValueView()
    {
        iteratorKeyImpl([](auto it) { return it.key(); },
                        [](auto it) { return it->toStringView(); });
    }
    void iteratorKeyViewValueView()
    {
        iteratorKeyImpl([](auto it) { return it.keyView(); },
                        [](auto it) { return it->toStringView(); });
    }

private:
    template <typename KeyFunc, typename ValueFunc>
    void iteratorKeyImpl(KeyFunc key, ValueFunc val);
};

BenchmarkQtJson::BenchmarkQtJson(QObject *parent) : QObject(parent)
{

}

void BenchmarkQtJson::initTestCase()
{

}

void BenchmarkQtJson::cleanupTestCase()
{

}

void BenchmarkQtJson::init()
{

}

void BenchmarkQtJson::cleanup()
{

}

void BenchmarkQtJson::parseNumbers()
{
    QString testFile = QFINDTESTDATA("numbers.json");
    QVERIFY2(!testFile.isEmpty(), "cannot find test file numbers.json!");
    QFile file(testFile);
    QVERIFY2(file.open(QFile::ReadOnly), qPrintable(file.errorString()));
    QByteArray testJson = file.readAll();

    QBENCHMARK {
        QJsonDocument doc = QJsonDocument::fromJson(testJson);
        QJsonObject object = doc.object();
    }
}

void BenchmarkQtJson::parseJson()
{
    QString testFile = QFINDTESTDATA("test.json");
    QVERIFY2(!testFile.isEmpty(), "cannot find test file test.json!");
    QFile file(testFile);
    QVERIFY2(file.open(QFile::ReadOnly), qPrintable(file.errorString()));
    QByteArray testJson = file.readAll();

    QBENCHMARK {
        QJsonDocument doc = QJsonDocument::fromJson(testJson);
        QJsonObject object = doc.object();
    }
}

void BenchmarkQtJson::parseJsonToVariant()
{
    QString testFile = QFINDTESTDATA("test.json");
    QVERIFY2(!testFile.isEmpty(), "cannot find test file test.json!");
    QFile file(testFile);
    QVERIFY2(file.open(QFile::ReadOnly), qPrintable(file.errorString()));
    QByteArray testJson = file.readAll();

    QBENCHMARK {
        QJsonDocument doc = QJsonDocument::fromJson(testJson);
        QVariant v = doc.toVariant();
    }
}

void BenchmarkQtJson::jsonObjectInsert()
{
    QJsonObject object;
    QString test(QStringLiteral("testString"));
    QJsonValue value(1.5);

    QBENCHMARK {
        for (int i = 0; i < 1000; i++)
            object.insert("testkey_" + QString::number(i), value);
    }
}

void BenchmarkQtJson::variantMapInsert()
{
    QVariantMap object;
    QString test(QStringLiteral("testString"));
    QVariant variantValue(1.5);

    QBENCHMARK {
        for (int i = 0; i < 1000; i++)
            object.insert("testkey_" + QString::number(i), variantValue);
    }
}

namespace Iteration {
template <typename KeyFunc, typename ValueFunc>
void visitObject(const QJsonObject &o, KeyFunc key, ValueFunc val);
template <typename KeyFunc, typename ValueFunc>
void visitArray(const QJsonArray &a, KeyFunc key, ValueFunc val) {
    for (const auto &value : a) {
        if (value.isObject())
            visitObject(value.toObject(), key, val);
        else if (value.isArray())
            visitArray(value.toArray(), key, val);
    }
}
template <typename KeyFunc, typename ValueFunc>
void visitObject(const QJsonObject &o, KeyFunc key, ValueFunc val) {
    for (auto it = o.begin(), end = o.end(); it != end; ++it) {
        [[maybe_unused]] const auto k = key(it);
        [[maybe_unused]] const auto v = val(it);
        if (it->isObject())
            visitObject(it->toObject(), key, val);
        else if (it->isArray())
            visitArray(it->toArray(), key, val);
    }
}
} // namespace Iteration

template <typename KeyFunc, typename ValueFunc>
void BenchmarkQtJson::iteratorKeyImpl(KeyFunc key, ValueFunc val)
{
    const QString testFile = QFINDTESTDATA("test.json");
    QVERIFY2(!testFile.isEmpty(), "cannot find test file test.json!");
    QFile file(testFile);
    QVERIFY2(file.open(QFile::ReadOnly), qPrintable(file.errorString()));
    const QByteArray content = file.readAll();
    QVERIFY(!content.isEmpty());
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(content, &error);
    QVERIFY2(!error.error, qPrintable(QString::asprintf("at offset %d: %ls",
                                                        error.offset,
                                                        qUtf16Printable(error.errorString()))));
    QVERIFY(!doc.isNull());
    QVERIFY(doc.isArray());
    const auto array = doc.array();
    QCOMPARE_GT(array.size(), 0);

    QBENCHMARK {
        Iteration::visitArray(array, key, val);
    }
}

QTEST_MAIN(BenchmarkQtJson)
#include "tst_bench_qtjson.moc"

