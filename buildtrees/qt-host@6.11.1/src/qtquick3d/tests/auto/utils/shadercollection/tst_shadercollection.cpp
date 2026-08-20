// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtCore/qbytearray.h>
#include <QtCore/qfile.h>

#include <rhi/qshaderbaker.h>

#include <QtQuick3DUtils/private/qqsbcollection_p.h>

static const char *shaderDescription() { return "ShaderDescription"; }
static const char *tempOutFileName() { return "qsbctstfile.qsbc"; }

static const char *vertSource() {
    return "#version 310 es\n"
           "layout (location = 0) in vec3 pos;\n"
           "layout (location = 1) out vec4 vertColor;\n"
           "void main()\n"
           "{\n"
           "    gl_Position = vec4(pos, 1.0);\n"
           "    vertColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
           "}\n";
}
static const char *fragSource() {
    return "#version 310 es\n"
            "precision lowp float;\n"
            "layout (location = 0) out vec4 glColor;\n"
            "layout (location = 1) in vec4 vertColor;\n"
            "void main()\n"
            "{\n"
            "    glColor = vertColor;\n"
            "}\n";
}

class ShaderCollection : public QObject
{
    Q_OBJECT
public:
    ShaderCollection() = default;
    ~ShaderCollection() = default;

private Q_SLOTS:
    void initTestCase();
    void test_readWriteToFile();
    void test_readWriteToBuffer();
    void test_readWriteOpenDevice();
    void test_mapModes();
    void test_inMemoryCollection();

private:
    QShader vert;
    QShader frag;
    QQsbCollection::FeatureSet featureSet;
};

void ShaderCollection::initTestCase()
{
    QShaderBaker baker;
    baker.setGeneratedShaders({{ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) }});
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    baker.setSourceString(vertSource(), QShader::VertexStage);
    vert = baker.bake();
    QVERIFY2(vert.isValid(), qPrintable(baker.errorMessage()));

    baker.setSourceString(fragSource(), QShader::FragmentStage);
    frag = baker.bake();
    QVERIFY2(frag.isValid(), qPrintable(baker.errorMessage()));

    featureSet.insert("feature1", true);
    featureSet.insert("feature2", false);
}

void ShaderCollection::test_readWriteToFile()
{
    const QByteArray hkey = QByteArrayLiteral("12345");
    const auto tempFile = QDir::tempPath() + QDir::separator() + tempOutFileName();
    QQsbCollection::Entry entry;
    {
        QQsbIODeviceCollection qsbc(tempFile);
        QVERIFY(qsbc.map(QQsbIODeviceCollection::Write));
        entry = qsbc.addEntry(hkey, { QByteArray(shaderDescription()), featureSet, vert, frag });
        QVERIFY(entry.value != -1);
        QCOMPARE(entry.key, hkey);
        const auto entries = qsbc.availableEntries();
        QCOMPARE(entries.size(), 1);
        QCOMPARE(*entries.begin(), entry);
        qsbc.unmap();
    }

    {
        QQsbIODeviceCollection qsbc(tempFile);
        QVERIFY(qsbc.map(QQsbIODeviceCollection::Read));
        const auto entries = qsbc.availableEntries();
        QCOMPARE(entries.size(), 1);
        QCOMPARE(*entries.begin(), entry);
        QQsbCollection::EntryDesc entryDesc;
        QVERIFY(qsbc.extractEntry(entry, entryDesc));
        QCOMPARE(entryDesc.materialKey, QByteArray(shaderDescription()));
        QCOMPARE(entryDesc.vertShader, vert);
        QCOMPARE(entryDesc.fragShader, frag);
        QCOMPARE(entryDesc.featureSet, featureSet);
        qsbc.unmap();
    }
}

void ShaderCollection::test_readWriteToBuffer()
{
    QBuffer buffer;
    const QByteArray hkey = QByteArrayLiteral("12345");
    QQsbCollection::Entry entry;
    {
        QVERIFY(buffer.buffer().isEmpty());
        QQsbIODeviceCollection qsbc(buffer);
        QVERIFY(qsbc.map(QQsbIODeviceCollection::Write));
        entry = qsbc.addEntry(hkey, { QByteArray(shaderDescription()), featureSet, vert, frag });
        QVERIFY(entry.value != -1);
        QCOMPARE(entry.key, hkey);
        qsbc.unmap();
        QVERIFY(!buffer.buffer().isEmpty());
    }

    {
        QQsbIODeviceCollection qsbc(buffer);
        QVERIFY(qsbc.map(QQsbIODeviceCollection::Read));
        QQsbCollection::EntryDesc entryDesc;
        QVERIFY(qsbc.extractEntry(entry, entryDesc));
        QCOMPARE(entryDesc.materialKey, QByteArray(shaderDescription()));
        QCOMPARE(entryDesc.vertShader, vert);
        QCOMPARE(entryDesc.fragShader, frag);
        QCOMPARE(entryDesc.featureSet, featureSet);
        qsbc.unmap();
    }
}

void ShaderCollection::test_readWriteOpenDevice()
{
    QBuffer buffer;
    const QByteArray hkey = QByteArrayLiteral("12345");
    QQsbCollection::Entry entry;
    {
        buffer.open(QIODevice::WriteOnly | QIODevice::Truncate);
        QCOMPARE(buffer.isOpen(), true);
        QVERIFY(buffer.buffer().isEmpty());
        QQsbIODeviceCollection qsbc(buffer);
        QVERIFY(qsbc.map(QQsbIODeviceCollection::Write));
        entry = qsbc.addEntry(hkey, { QByteArray(shaderDescription()), featureSet, vert, frag });
        QVERIFY(entry.value != -1);
        QCOMPARE(entry.key, hkey);
        qsbc.unmap();
        QVERIFY(!buffer.buffer().isEmpty());
    }

    QCOMPARE(buffer.isOpen(), false);

    {
        buffer.open(QIODevice::ReadOnly);
        QCOMPARE(buffer.isOpen(), true);
        QQsbIODeviceCollection qsbc(buffer);
        QVERIFY(qsbc.map(QQsbIODeviceCollection::Read));
        QQsbCollection::EntryDesc entryDesc;
        QVERIFY(qsbc.extractEntry(entry, entryDesc));
        QCOMPARE(entryDesc.materialKey, QByteArray(shaderDescription()));
        QCOMPARE(entryDesc.vertShader, vert);
        QCOMPARE(entryDesc.fragShader, frag);
        QCOMPARE(entryDesc.featureSet, featureSet);
        qsbc.unmap();
    }
}

void ShaderCollection::test_mapModes()
{
    {
        QBuffer buffer;
        const QByteArray hkey = QByteArrayLiteral("12345");
        QQsbCollection::Entry entry;
        {
            buffer.open(QIODevice::ReadWrite | QIODevice::Truncate);
            QCOMPARE(buffer.isOpen(), true);
            QVERIFY(buffer.buffer().isEmpty());
            QQsbIODeviceCollection qsbc(buffer);
            QVERIFY(qsbc.map(QQsbIODeviceCollection::Write));
            entry = qsbc.addEntry(hkey, { QByteArray(shaderDescription()), featureSet, vert, frag });
            QVERIFY(entry.value != -1);
            QCOMPARE(entry.key, hkey);
            qsbc.unmap();
            QVERIFY(!buffer.buffer().isEmpty());
        }

        QCOMPARE(buffer.isOpen(), false);

        {
            buffer.open(QIODevice::ReadOnly);
            QCOMPARE(buffer.isOpen(), true);
            QQsbIODeviceCollection qsbc(buffer);
            QVERIFY(qsbc.map(QQsbIODeviceCollection::Read));
            QQsbCollection::EntryDesc entryDesc;
            QVERIFY(qsbc.extractEntry(entry, entryDesc));
            QCOMPARE(entryDesc.materialKey, QByteArray(shaderDescription()));
            QCOMPARE(entryDesc.vertShader, vert);
            QCOMPARE(entryDesc.fragShader, frag);
            QCOMPARE(entryDesc.featureSet, featureSet);
            qsbc.unmap();
        }
    }

    {
        QBuffer buffer;
        const QByteArray hkey = QByteArrayLiteral("12345");
        QQsbCollection::Entry entry;

        { // Test invalid input (missing Truncate)
            buffer.open(QIODevice::ReadWrite);
            QCOMPARE(buffer.isOpen(), true);
            QVERIFY(buffer.buffer().isEmpty());
            QQsbIODeviceCollection qsbc(buffer);
            QCOMPARE(qsbc.map(QQsbIODeviceCollection::Write), false);
            entry = qsbc.addEntry(hkey, { QByteArray(shaderDescription()), featureSet, vert, frag });
            QCOMPARE(entry.isValid(), false);
            qsbc.unmap();
            QVERIFY(buffer.buffer().isEmpty());
        }

        { // Test invalid input (Text)
            buffer.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
            QCOMPARE(buffer.isOpen(), true);
            QVERIFY(buffer.buffer().isEmpty());
            QQsbIODeviceCollection qsbc(buffer);
            QCOMPARE(qsbc.map(QQsbIODeviceCollection::Write), false);
            entry = qsbc.addEntry(hkey, { QByteArray(shaderDescription()), featureSet, vert, frag });
            QCOMPARE(entry.isValid(), false);
            qsbc.unmap();
            QVERIFY(buffer.buffer().isEmpty());
        }
    }
}

void ShaderCollection::test_inMemoryCollection()
{
    QQsbInMemoryCollection qsbc;
    QVERIFY(qsbc.availableEntries().isEmpty());

    const QByteArray hkey = QByteArrayLiteral("12345");
    const QByteArray otherKey = QByteArrayLiteral("12346");
    QQsbCollection::Entry entry;
    QVERIFY(!entry.isValid());

    {
        entry = qsbc.addEntry(hkey, { QByteArray(shaderDescription()), featureSet, vert, frag });
        QVERIFY(entry.isValid());
        QVERIFY(qsbc.availableEntries().contains(entry));
        qsbc.clear();
        QVERIFY(qsbc.availableEntries().isEmpty());
    }

    {
        entry = qsbc.addEntry(hkey, { QByteArray(shaderDescription()), featureSet, vert, frag });
        QVERIFY(entry.isValid());
        entry = qsbc.addEntry(hkey, { QByteArray(shaderDescription()), featureSet, vert, frag });
        QVERIFY(!entry.isValid()); // already added, this must fail
        entry = qsbc.addEntry(otherKey, { QByteArray(shaderDescription()), featureSet, vert, frag });
        QVERIFY(entry.isValid());
        QVERIFY(qsbc.availableEntries().count() == 2);

        QQsbCollection::EntryDesc entryDesc;
        QVERIFY(qsbc.extractEntry(entry, entryDesc));
        QCOMPARE(entryDesc.materialKey, QByteArray(shaderDescription()));
        QCOMPARE(entryDesc.vertShader, vert);
        QCOMPARE(entryDesc.fragShader, frag);
        QCOMPARE(entryDesc.featureSet, featureSet);
    }

}

QTEST_APPLESS_MAIN(ShaderCollection)

#include "tst_shadercollection.moc"
