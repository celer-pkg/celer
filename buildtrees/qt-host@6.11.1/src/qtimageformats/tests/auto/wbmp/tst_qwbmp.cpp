// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtGui/QtGui>

class tst_qwbmp: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void readImage_data();
    void readImage();
};

void tst_qwbmp::initTestCase()
{
    if (!QImageReader::supportedImageFormats().contains("wbmp"))
        QSKIP("The image format handler is not installed.");
}

void tst_qwbmp::readImage_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QSize>("size");

    QTest::newRow("qt-logo-small") << QString("qt-logo-small.wbmp") << QSize(123, 103);
    QTest::newRow("qt-logo-big") << QString("qt-logo-big.wbmp") << QSize(250, 250);
}

void tst_qwbmp::readImage()
{
    QFETCH(QString, fileName);
    QFETCH(QSize, size);

    QString path = QString(":/wbmp/") + fileName;
    QImageReader reader(path);
    QVERIFY(reader.canRead());
    QImage image = reader.read();
    QVERIFY(!image.isNull());
    QCOMPARE(image.size(), size);
}

QTEST_MAIN(tst_qwbmp)

#include "tst_qwbmp.moc"
