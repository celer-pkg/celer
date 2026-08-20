// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtTest>
#include <QDebug>
#include <QtQuick3DAssetImport/private/qssgassetimportmanager_p.h>
#include <QDir>
#include <QByteArray>

// add necessary includes here

class tst_assetimport : public QObject
{
    Q_OBJECT

public:
    tst_assetimport();
    ~tst_assetimport();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void importFile_data();
    void importFile();

};

tst_assetimport::tst_assetimport()
{

}

tst_assetimport::~tst_assetimport()
{

}

void tst_assetimport::initTestCase()
{

}

void tst_assetimport::cleanupTestCase()
{

}

void tst_assetimport::importFile_data()
{
    QTest::addColumn<QString>("extension");
    QTest::addColumn<bool>("result");

    QTest::newRow("fbx") << QString("fbx") << true;
    QTest::newRow("dae") << QString("dae") << true;
    QTest::newRow("obj") << QString("obj") << true;
    QTest::newRow("gltf") << QString("gltf") << true;
    QTest::newRow("glb") << QString("glb") << true;
    QTest::newRow("stl") << QString("stl") << true;
    QTest::newRow("ply") << QString("ply") << true;
}

void tst_assetimport::importFile()
{
    QFETCH(QString, extension);
    QFETCH(bool, result);

    QSSGAssetImportManager importManager;
    QString file = "resources/cube_scene." + extension;
    QString error;

    // Should return "true" if there were no errors opening the source or creating the exported object.
    auto importState = importManager.importFile(QFINDTESTDATA(file), QDir("./"), &error);
    const bool realResult = (importState == QSSGAssetImportManager::ImportState::Success);
    if(!error.isEmpty()) {
        if (importState == QSSGAssetImportManager::ImportState::Unsupported) {
            QEXPECT_FAIL("", "Unsupported format!", Continue);
        } else {
            qDebug() << "Error message:" << error;
            QFAIL(error.toStdString().c_str());
        }
    }

    QCOMPARE(realResult, result);
}

QTEST_APPLESS_MAIN(tst_assetimport)

#include "tst_assetimport.moc"
