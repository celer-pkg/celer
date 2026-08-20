// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QBuffer>
#include <QSharedPointer>
#include <QTemporaryFile>
#include <QVariantMap>

#include <QtQuick3DRuntimeRender/private/qssglightmapio_p.h>

class tst_QQuick3DLightmapIO : public QObject
{
    Q_OBJECT

private slots:
    void testWriteAndRead();
    void testSaveToInvalidPath();
};

// Test writing and reading metadata and image
void tst_QQuick3DLightmapIO::testWriteAndRead()
{
    QByteArray bufferData;

    QByteArray f32Image(16, 'F');
    QByteArray u32Image(16, 'U');

    QByteArray finalImage("FINAL");
    QByteArray directImage("DIRECT");

    const QString longName = QStringLiteral("THIS-IS-A-VERY-LONG-NAME-THIS-IS-A-VERY-LONG-NAME-THIS-IS-A-VERY-LONG-"
                                            "NAME-THIS-IS-A-VERY-LONG-NAME-THIS-IS-A-VERY-LONG-NAME_mask");

    { // Save data to buffer
        QSharedPointer<QIODevice> buffer(new class QBuffer(&bufferData));
        QSharedPointer<QSSGLightmapWriter> io = QSSGLightmapWriter::open(buffer);

        QVariantMap metadata;
        metadata["author"] = "QtCreator";
        metadata["version"] = 1.0;
        metadata["width"] = 100;
        metadata["height"] = 50;

        io->writeMap("metadata", QSSGLightmapIODataTag::Metadata, metadata);
        io->writeF32Image("image", QSSGLightmapIODataTag::Unset, f32Image);

        QVariantMap mapA;
        QVariantMap mapB;
        QVariantMap mapC;

        mapA["value"] = "a";
        mapB["value"] = "b";
        mapC["value"] = "c";

        io->writeMap("a", QSSGLightmapIODataTag::Metadata, mapA);
        io->writeMap("b", QSSGLightmapIODataTag::Metadata, mapB);
        io->writeMap("c", QSSGLightmapIODataTag::SceneMetadata, mapC);

        io->writeF32Image("a", QSSGLightmapIODataTag::Texture_Final, finalImage);
        io->writeU32Image("a", QSSGLightmapIODataTag::Texture_Direct, directImage);

        io->writeU32Image(longName, QSSGLightmapIODataTag::Unset, u32Image);

        io->close();
    }

    QSharedPointer<QIODevice> buffer(new class QBuffer(&bufferData));
    QSharedPointer<QSSGLightmapLoader> io = QSSGLightmapLoader::open(buffer);
    QVariantMap metadata = io->readMap("metadata", QSSGLightmapIODataTag::Metadata);
    QVariantMap mapA = io->readMap("a", QSSGLightmapIODataTag::Metadata);
    QVariantMap mapB = io->readMap("b", QSSGLightmapIODataTag::Metadata);
    QVariantMap mapC = io->readMap("c", QSSGLightmapIODataTag::SceneMetadata);
    QCOMPARE(metadata["author"].toString(), QString("QtCreator"));
    QCOMPARE(metadata["version"].toDouble(), 1.0);
    QCOMPARE(metadata["width"].toUInt(), 100);
    QCOMPARE(metadata["height"].toUInt(), 50);
    QCOMPARE(mapA["value"].toString(), "a");
    QCOMPARE(mapB["value"].toString(), "b");
    QCOMPARE(mapC["value"].toString(), "c");
    QCOMPARE(io->readF32Image("image", QSSGLightmapIODataTag::Unset), f32Image);
    QCOMPARE(io->readU32Image(longName, QSSGLightmapIODataTag::Unset), u32Image);
    QCOMPARE(io->readF32Image("a", QSSGLightmapIODataTag::Texture_Final), finalImage);
    QCOMPARE(io->readU32Image("a", QSSGLightmapIODataTag::Texture_Direct), directImage);
}

// Test saving to an invalid path
void tst_QQuick3DLightmapIO::testSaveToInvalidPath()
{
    QSharedPointer<QSSGLightmapLoader> io = QSSGLightmapLoader::open("/nonexistent/path/file.lightmap");
    QVERIFY(!io); // We expect it to fail
}

QTEST_APPLESS_MAIN(tst_QQuick3DLightmapIO)

#include "tst_qquick3dlightmapio.moc"
