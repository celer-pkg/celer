// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include "qbinaryjson.h"

#include "qjsonarray.h"
#include "qjsonobject.h"
#include "qjsonvalue.h"
#include "qjsondocument.h"
#include "qregularexpression.h"
#include <limits>

#define INVALID_UNICODE "\xCE\xBA\xE1"
#define UNICODE_NON_CHARACTER "\xEF\xBF\xBF"
#define UNICODE_DJE "\320\202" // Character from the Serbian Cyrillic alphabet

class tst_QtJson: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void fromBinary();
    void toAndFromBinary_data();
    void toAndFromBinary();
    void invalidBinaryData();
    void compactArray();
    void compactObject();
    void validation();
    void testCompactionError();
private:
    QString testDataDir;
};

void tst_QtJson::initTestCase()
{
    testDataDir = QFileInfo(QFINDTESTDATA("test.json")).absolutePath();
    if (testDataDir.isEmpty())
        testDataDir = QCoreApplication::applicationDirPath();
}

void tst_QtJson::fromBinary()
{
    QFile file(testDataDir + "/test.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray testJson = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(testJson);
    QJsonDocument outdoc = QBinaryJson::fromBinaryData(QBinaryJson::toBinaryData(doc));
    QVERIFY(!outdoc.isNull());
    QCOMPARE(doc, outdoc);

    QFile bfile(testDataDir + "/test.bjson");
    QVERIFY(bfile.open(QFile::ReadOnly));
    QByteArray binary = bfile.readAll();

    QJsonDocument bdoc = QBinaryJson::fromBinaryData(binary);
    QVERIFY(!bdoc.isNull());
    QCOMPARE(doc.toVariant(), bdoc.toVariant());
    QCOMPARE(doc, bdoc);
}

void tst_QtJson::toAndFromBinary_data()
{
    QTest::addColumn<QString>("filename");
    QTest::newRow("test.json") << (testDataDir + "/test.json");
    QTest::newRow("test2.json") << (testDataDir + "/test2.json");
}

void tst_QtJson::toAndFromBinary()
{
    QFETCH(QString, filename);
    QFile file(filename);
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray data = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QVERIFY(!doc.isNull());
    {
        QJsonDocument outdoc = QBinaryJson::fromBinaryData(QBinaryJson::toBinaryData(doc));
        QVERIFY(!outdoc.isNull());
        QCOMPARE(doc, outdoc);
    }
    {
        int size = -1;
        auto rawData = QBinaryJson::toRawData(doc, &size);
        QVERIFY(size > 0);
        QJsonDocument outdoc = QBinaryJson::fromRawData(rawData, size);
        QVERIFY(!outdoc.isNull());
        QCOMPARE(doc, outdoc);
    }
}

void tst_QtJson::invalidBinaryData()
{
    QDir dir(testDataDir + "/invalidBinaryData");
    QFileInfoList files = dir.entryInfoList();
    for (int i = 0; i < files.size(); ++i) {
        if (!files.at(i).isFile())
            continue;
        QFile file(files.at(i).filePath());
        QVERIFY(file.open(QIODevice::ReadOnly));
        QByteArray bytes = file.readAll();
        bytes.squeeze();
        QJsonDocument document = QBinaryJson::fromRawData(bytes.constData(), bytes.size());
        QVERIFY(document.isNull());
    }
}

void tst_QtJson::compactArray()
{
    QJsonArray array;
    array.append(QLatin1String("First Entry"));
    array.append(QLatin1String("Second Entry"));
    array.append(QLatin1String("Third Entry"));
    QJsonDocument doc(array);
    int s =  QBinaryJson::toBinaryData(doc).size();
    array.removeAt(1);
    doc.setArray(array);
    QVERIFY(s > QBinaryJson::toBinaryData(doc).size());
    s = QBinaryJson::toBinaryData(doc).size();
    QCOMPARE(doc.toJson(),
             QByteArray("[\n"
                        "    \"First Entry\",\n"
                        "    \"Third Entry\"\n"
                        "]\n"));

    array.removeAt(0);
    doc.setArray(array);
    QVERIFY(s > QBinaryJson::toBinaryData(doc).size());
    s = QBinaryJson::toBinaryData(doc).size();
    QCOMPARE(doc.toJson(),
             QByteArray("[\n"
                        "    \"Third Entry\"\n"
                        "]\n"));

    array.removeAt(0);
    doc.setArray(array);
    QVERIFY(s > QBinaryJson::toBinaryData(doc).size());
    s = QBinaryJson::toBinaryData(doc).size();
    QCOMPARE(doc.toJson(),
             QByteArray("[\n"
                        "]\n"));

}

void tst_QtJson::compactObject()
{
    QJsonObject object;
    object.insert(QLatin1String("Key1"), QLatin1String("First Entry"));
    object.insert(QLatin1String("Key2"), QLatin1String("Second Entry"));
    object.insert(QLatin1String("Key3"), QLatin1String("Third Entry"));
    QJsonDocument doc(object);
    int s =  QBinaryJson::toBinaryData(doc).size();
    object.remove(QLatin1String("Key2"));
    doc.setObject(object);
    QVERIFY(s > QBinaryJson::toBinaryData(doc).size());
    s = QBinaryJson::toBinaryData(doc).size();
    QCOMPARE(doc.toJson(),
             QByteArray("{\n"
                        "    \"Key1\": \"First Entry\",\n"
                        "    \"Key3\": \"Third Entry\"\n"
                        "}\n"));

    object.remove(QLatin1String("Key1"));
    doc.setObject(object);
    QVERIFY(s > QBinaryJson::toBinaryData(doc).size());
    s = QBinaryJson::toBinaryData(doc).size();
    QCOMPARE(doc.toJson(),
             QByteArray("{\n"
                        "    \"Key3\": \"Third Entry\"\n"
                        "}\n"));

    object.remove(QLatin1String("Key3"));
    doc.setObject(object);
    QVERIFY(s > QBinaryJson::toBinaryData(doc).size());
    s = QBinaryJson::toBinaryData(doc).size();
    QCOMPARE(doc.toJson(),
             QByteArray("{\n"
                        "}\n"));

}

void tst_QtJson::validation()
{
    // this basically tests that we don't crash on corrupt data
    QFile file(testDataDir + "/test.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray testJson = file.readAll();
    QVERIFY(!testJson.isEmpty());

    QJsonDocument doc = QJsonDocument::fromJson(testJson);
    QVERIFY(!doc.isNull());

    QByteArray binary = QBinaryJson::toBinaryData(doc);

    // only test the first 1000 bytes. Testing the full file takes too long
    for (int i = 0; i < 1000; ++i) {
        QByteArray corrupted = binary;
        corrupted[i] = char(0xff);
        QJsonDocument doc = QBinaryJson::fromBinaryData(corrupted);
        if (doc.isNull())
            continue;
        QByteArray json = doc.toJson();
    }


    QFile file2(testDataDir + "/test3.json");
    QVERIFY(file2.open(QFile::ReadOnly));
    testJson = file2.readAll();
    QVERIFY(!testJson.isEmpty());

    doc = QJsonDocument::fromJson(testJson);
    QVERIFY(!doc.isNull());

    binary = QBinaryJson::toBinaryData(doc);

    for (int i = 0; i < binary.size(); ++i) {
        QByteArray corrupted = binary;
        corrupted[i] = char(0xff);
        QJsonDocument doc = QBinaryJson::fromBinaryData(corrupted);
        if (doc.isNull())
            continue;
        QByteArray json = doc.toJson();

        corrupted = binary;
        corrupted[i] = 0x00;
        doc = QBinaryJson::fromBinaryData(corrupted);
        if (doc.isNull())
            continue;
        json = doc.toJson();
    }
}

void tst_QtJson::testCompactionError()
{
    QJsonObject schemaObject;
    schemaObject.insert("_Type", QLatin1String("_SchemaType"));
    schemaObject.insert("name", QLatin1String("Address"));
    schemaObject.insert("schema", QJsonObject());
    {
        QJsonObject content(schemaObject);
        QJsonDocument doc(content);
        QVERIFY(!doc.isNull());
        QByteArray hash = QCryptographicHash::hash(QBinaryJson::toBinaryData(doc), QCryptographicHash::Md5).toHex();
        schemaObject.insert("_Version", QString::fromLatin1(hash.constData(), hash.size()));
    }

    QJsonObject schema;
    schema.insert("streetNumber", schema.value("number").toObject());
    schemaObject.insert("schema", schema);
    {
        QJsonObject content(schemaObject);
        content.remove("_Uuid");
        content.remove("_Version");
        QJsonDocument doc(content);
        QVERIFY(!doc.isNull());
        QByteArray hash = QCryptographicHash::hash(QBinaryJson::toBinaryData(doc), QCryptographicHash::Md5).toHex();
        schemaObject.insert("_Version", QString::fromLatin1(hash.constData(), hash.size()));
    }
}

QTEST_MAIN(tst_QtJson)
#include "tst_qtjson.moc"
