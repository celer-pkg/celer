// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtPositioning/QGeoCoordinate>
#include <QtCore/QJsonDocument>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtLocation/private/qgeojson_p.h>

QT_USE_NAMESPACE

class tst_QGeoJson : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testGeojson();

private:
    QString testDataDir;
};

void tst_QGeoJson::testGeojson()
{
    QJsonDocument originalDocument;
    QJsonDocument processedDocument;
    QVariantList modelList;

    QString exampleGeometries[] = {
        QStringLiteral("01-point.json"),
        QStringLiteral("02-linestring.json"),
        QStringLiteral("03-multipoint.json"),
        QStringLiteral("04-polygon.json"),
        QStringLiteral("05-multilinestring.json"),
        QStringLiteral("06-multipolygon.json"),
        QStringLiteral("07-geometrycollection.json"),
        QStringLiteral("08-feature.json"),
        QStringLiteral("09-featurecollection.json"),
        QStringLiteral("10-countries.json"),
        QStringLiteral("11-full.json")
    };

    testDataDir = QFileInfo(QFINDTESTDATA("01-point.json")).absolutePath();
    if (testDataDir.isEmpty())
        testDataDir = QCoreApplication::applicationDirPath();
    for (int i = 0; i < 11; ++i) {
        modelList.clear();

        QFile file(testDataDir +  QStringLiteral("/") + exampleGeometries[i]);
        QVERIFY2(file.open(QFile::ReadOnly), qPrintable(file.errorString()));

        QByteArray testJson = file.readAll();
        originalDocument = QJsonDocument::fromJson(testJson);

        modelList = QGeoJson::importGeoJson(originalDocument);
        processedDocument = QGeoJson::exportGeoJson(modelList);

        QVERIFY2(processedDocument == originalDocument, qPrintable("Test failed in file: " + exampleGeometries[i]));
    }
}

QTEST_MAIN(tst_QGeoJson)
#include "tst_qgeojson.moc"
