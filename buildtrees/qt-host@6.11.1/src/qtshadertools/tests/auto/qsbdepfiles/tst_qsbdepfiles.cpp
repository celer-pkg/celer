// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QTest>
#include <QtCore/qstring.h>

using namespace Qt::StringLiterals;

class tst_qsbdepfiles : public QObject
{
    Q_OBJECT
private:
    void init();
private slots:
    void Depfiles_data();
    void Depfiles();
};

void tst_qsbdepfiles::Depfiles_data()
{
    QTest::addColumn<QLatin1String>("filename");

    QTest::newRow("WithoutInclude") << "tst.frag.qsb.d"_L1;
    QTest::newRow("WithInclude") << "tstinclude.frag.qsb.d"_L1;
    QTest::newRow("WithWhitespaces") << "tstincludewhitespaces.frag.qsb.d"_L1;
    QTest::newRow("WithRelativeInclude") << "tstincluderelative.frag.qsb.d"_L1;
    QTest::newRow("WithComment") << "tstincludecomment.frag.qsb.d"_L1;
}

void tst_qsbdepfiles::Depfiles()
{
    QFETCH(const QLatin1String, filename);
    QString resultFilePath = QFINDTESTDATA(QString(".qsb/shaders/%1").arg(filename));
    QString expectedFilePath = QString(":/data/%1").arg(filename);

    QFile resultFile(resultFilePath);
    QFile expectedFile(expectedFilePath);

    QVERIFY(resultFile.open(QFile::ReadOnly));
    QVERIFY(expectedFile.open(QFile::ReadOnly));

    // We chop the one byte from expectedFile because of CMake issue.
    // See https://gitlab.kitware.com/cmake/cmake/-/issues/25164
    QVERIFY(resultFile.readAll() == expectedFile.readAll().chopped(1));
}

QTEST_MAIN(tst_qsbdepfiles)

#include "tst_qsbdepfiles.moc"
