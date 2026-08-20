// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QFile>
#include <rhi/qshaderbaker.h>
#include <rhi/qshaderdescription.h>
#include <private/qshader_p.h>

class tst_QShaderBaker : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void emptyCompile();
    void noFileCompile();
    void noTargetsCompile();
    void noVariantsCompile();
    void simpleCompile();
    void simpleCompileNoSpirvSpecified();
    void simpleCompileCheckResults();
    void simpleCompileFromDevice();
    void simpleCompileFromString();
    void multiCompile();
    void reuse();
    void compileError();
    void translateError();
    void genVariants();
    void defines();
    void reflectArrayOfStructInBlock();
    void reflectCombinedImageSampler();
    void mslNativeBindingMap();
    void hlslNativeBindingMap();
    void reflectArraysOfSamplers();
    void perTargetCompileMode();
    void serializeDeserialize();
    void spirvOptions();
    void separateImagesAndSamplers();
    void tessellationForMetalVertCompile();
    void tessellationCompile();
    void tessellationCompileWithChangedTessArgs();
    void dontBreakOnTranslationError();
    void storageImageFlags();
    void storageBufferRuntimeArrayStride();
    void storageBufferQualifiers();
    void multiview();
    void mediump();
    void geometryShader();
};

void tst_QShaderBaker::initTestCase()
{
}

void tst_QShaderBaker::cleanup()
{
}

void tst_QShaderBaker::emptyCompile()
{
    QShaderBaker baker;
    QShader s = baker.bake();
    QVERIFY(!s.isValid());
    QVERIFY(!baker.errorMessage().isEmpty());
    qDebug() << baker.errorMessage();
}

void tst_QShaderBaker::noFileCompile()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/nonexistant.vert"));
    QShader s = baker.bake();
    QVERIFY(!s.isValid());
    QVERIFY(!baker.errorMessage().isEmpty());
    qDebug() << baker.errorMessage();
}

void tst_QShaderBaker::noTargetsCompile()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    QShader s = baker.bake();
    // an empty shader pack is invalid
    QVERIFY(!s.isValid());
    // not an error from the baker's point of view however
    QVERIFY(baker.errorMessage().isEmpty());
}

void tst_QShaderBaker::noVariantsCompile()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    // an empty shader pack is invalid
    QVERIFY(!s.isValid());
    // not an error from the baker's point of view however
    QVERIFY(baker.errorMessage().isEmpty());
}

void tst_QShaderBaker::simpleCompile()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 1);
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::SpirvShader, QShaderVersion(100))));
}

void tst_QShaderBaker::simpleCompileNoSpirvSpecified()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::GlslShader, QShaderVersion(330) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 1);
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(330))));
    QVERIFY(s.shader(s.availableShaders().first()).shader().contains(QByteArrayLiteral("#version 330")));
}

void tst_QShaderBaker::simpleCompileCheckResults()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 1);

    const QShaderCode shader = s.shader(QShaderKey(QShader::SpirvShader,
                                                   QShaderVersion(100)));
    QVERIFY(!shader.shader().isEmpty());
    QCOMPARE(shader.entryPoint(), QByteArrayLiteral("main"));

    const QShaderDescription desc = s.description();
    QVERIFY(desc.isValid());
    QCOMPARE(desc.inputVariables().size(), 2);
    for (const QShaderDescription::InOutVariable &v : desc.inputVariables()) {
        switch (v.location) {
        case 0:
            QCOMPARE(v.name, QByteArrayLiteral("position"));
            QCOMPARE(v.type, QShaderDescription::Vec4);
            break;
        case 1:
            QCOMPARE(v.name, QByteArrayLiteral("color"));
            QCOMPARE(v.type, QShaderDescription::Vec3);
            break;
        default:
            QFAIL(qPrintable(QStringLiteral("Unexpected location value: %1").arg(v.location)));
            break;
        }
    }
    QCOMPARE(desc.outputVariables().size(), 1);
    for (const QShaderDescription::InOutVariable &v : desc.outputVariables()) {
        switch (v.location) {
        case 0:
            QCOMPARE(v.name, QByteArrayLiteral("v_color"));
            QCOMPARE(v.type, QShaderDescription::Vec3);
            break;
        default:
            QFAIL(qPrintable(QStringLiteral("Unexpected location value: %1").arg(v.location)));
            break;
        }
    }
    QCOMPARE(desc.uniformBlocks().size(), 1);
    const QShaderDescription::UniformBlock blk = desc.uniformBlocks().first();
    QCOMPARE(blk.blockName, QByteArrayLiteral("buf"));
    QCOMPARE(blk.structName, QByteArrayLiteral("ubuf"));
    QCOMPARE(blk.size, 68);
    QCOMPARE(blk.binding, 0);
    QCOMPARE(blk.descriptorSet, 0);
    QCOMPARE(blk.members.size(), 2);
    for (int i = 0; i < blk.members.size(); ++i) {
        const QShaderDescription::BlockVariable v = blk.members[i];
        switch (i) {
        case 0:
            QCOMPARE(v.offset, 0);
            QCOMPARE(v.size, 64);
            QCOMPARE(v.name, QByteArrayLiteral("mvp"));
            QCOMPARE(v.type, QShaderDescription::Mat4);
            QCOMPARE(v.matrixStride, 16);
            break;
        case 1:
            QCOMPARE(v.offset, 64);
            QCOMPARE(v.size, 4);
            QCOMPARE(v.name, QByteArrayLiteral("opacity"));
            QCOMPARE(v.type, QShaderDescription::Float);
            break;
        default:
            QFAIL(qPrintable(QStringLiteral("Too many blocks").arg(blk.members.size())));
            break;
        }
    }
}

void tst_QShaderBaker::simpleCompileFromDevice()
{
    QFile f(QLatin1String(":/data/color.vert"));
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));

    QShaderBaker baker;
    baker.setSourceDevice(&f, QShader::VertexStage);
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 1);
}

void tst_QShaderBaker::simpleCompileFromString()
{
    QFile f(QLatin1String(":/data/color.vert"));
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
    const QByteArray contents = f.readAll();
    f.close();
    QVERIFY(!contents.isEmpty());

    QShaderBaker baker;
    baker.setSourceString(contents, QShader::VertexStage);
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 1);
}

void tst_QShaderBaker::multiCompile()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 5);

    for (const QShaderBaker::GeneratedShader &genShader : targets) {
        const QShaderKey key(genShader.first, genShader.second);
        const QShaderCode shader = s.shader(key);
        QVERIFY(!shader.shader().isEmpty());
        if (genShader.first != QShader::MslShader)
            QCOMPARE(shader.entryPoint(), QByteArrayLiteral("main"));
    }
}

void tst_QShaderBaker::reuse()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 1);

    baker.setSourceFileName(QLatin1String(":/data/color.frag"));
    targets.clear();
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 5);
}

void tst_QShaderBaker::compileError()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/error.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(!s.isValid());
    QVERIFY(!baker.errorMessage().isEmpty());
    qDebug() << baker.errorMessage();
}

void tst_QShaderBaker::translateError()
{
    // assume the shader here fails in SPIRV-Cross with "cbuffer cannot be expressed with either HLSL packing layout or packoffset"
    //
    // This has been improved in SPIRV-Cross, presumably by 0ac65a353e94e9756e8b89fa74e953c482a915e0, so skip this test.
    QSKIP("Not applicable with updated SPIRV-Cross");

    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/hlsl_cbuf_error.frag"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(!s.isValid());
    QVERIFY(!baker.errorMessage().isEmpty());
    qDebug() << baker.errorMessage();
}

void tst_QShaderBaker::genVariants()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    baker.setGeneratedShaderVariants({
                                         QShader::StandardShader,
                                         QShader::BatchableVertexShader
                                     });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(330) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 2 * 6);

    int batchableVariantCount = 0;
    int batchableGlslVariantCount = 0;
    for (const QShaderKey &key : s.availableShaders()) {
        if (key.sourceVariant() == QShader::BatchableVertexShader) {
            ++batchableVariantCount;
            if (key.source() == QShader::GlslShader) {
                ++batchableGlslVariantCount;
                const QByteArray src = s.shader(key).shader();
                QVERIFY(src.contains(QByteArrayLiteral("_qt_order * ")));
            }
        } else {
            if (key.source() == QShader::GlslShader) {
                // make sure it did not get rewritten
                const QByteArray src = s.shader(key).shader();
                QVERIFY(!src.contains(QByteArrayLiteral("_qt_order * ")));
            }
        }
    }
    QCOMPARE(batchableVariantCount, 6);
    QCOMPARE(batchableGlslVariantCount, 3);
}

void tst_QShaderBaker::defines()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/defines.frag"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    baker.setGeneratedShaders({ { QShader::SpirvShader, QShaderVersion(100) } });
    QShader s = baker.bake();
    QVERIFY(!s.isValid());
    QVERIFY(!baker.errorMessage().isEmpty());
    qDebug() << baker.errorMessage();

    QByteArray preamble;
    preamble = QByteArrayLiteral("#define DO_NOT_BREAK\n");
    baker.setPreamble(preamble);
    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());

    QShaderDescription desc = s.description();
    QCOMPARE(desc.uniformBlocks().size(), 1);
    QShaderDescription::UniformBlock blk = desc.uniformBlocks().first();
    QCOMPARE(blk.members.size(), 2);
    bool opacity_ok = false;
    for (int i = 0; i < blk.members.size(); ++i) {
        const QShaderDescription::BlockVariable v = blk.members[i];
        if (v.name == QByteArrayLiteral("opacity")) {
            opacity_ok = v.type == QShaderDescription::Vec4;
            break;
        }
    }
    QVERIFY(opacity_ok);

    preamble += QByteArrayLiteral("#define OPACITY_SIZE 1\n");
    baker.setPreamble(preamble);
    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());

    desc = s.description();
    blk = desc.uniformBlocks().first();
    opacity_ok = false;
    for (int i = 0; i < blk.members.size(); ++i) {
        const QShaderDescription::BlockVariable v = blk.members[i];
        if (v.name == QByteArrayLiteral("opacity")) {
            opacity_ok = v.type == QShaderDescription::Float;
            break;
        }
    }
    QVERIFY(opacity_ok);
}

void tst_QShaderBaker::reflectArrayOfStructInBlock()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/array_of_struct_in_ubuf.frag"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::GlslShader, QShaderVersion(150) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());

    QShaderDescription desc = s.description();

    QCOMPARE(desc.inputVariables().size(), 3);
    QCOMPARE(desc.outputVariables().size(), 1);
    QCOMPARE(desc.uniformBlocks().size(), 1);

    const QList<QShaderDescription::InOutVariable> inputs = desc.inputVariables();
    for (const auto &var : inputs) {
        switch (var.location) {
        case 0:
            QCOMPARE(var.name, QByteArrayLiteral("vECVertNormal"));
            QCOMPARE(var.type, QShaderDescription::Vec3);
            break;
        case 1:
            QCOMPARE(var.name, QByteArrayLiteral("vECVertPos"));
            QCOMPARE(var.type, QShaderDescription::Vec3);
            break;
        case 2:
            QCOMPARE(var.name, QByteArrayLiteral("vDiffuseAdjust"));
            QCOMPARE(var.type, QShaderDescription::Vec3);
            break;
        default:
            QFAIL(qPrintable(QStringLiteral("Unexpected input variable: %1").arg(var.location)));
            return;
        }
    }

    const QList<QShaderDescription::InOutVariable> outputs = desc.outputVariables();
    QCOMPARE(outputs.first().location, 0);
    QCOMPARE(outputs.first().name, QByteArrayLiteral("fragColor"));
    QCOMPARE(outputs.first().type, QShaderDescription::Vec4);

    const QList<QShaderDescription::UniformBlock> ublocks = desc.uniformBlocks();
    const QShaderDescription::UniformBlock ub = ublocks.first();
    QCOMPARE(ub.binding, 1);
    QCOMPARE(ub.blockName, QByteArrayLiteral("buf"));
    QCOMPARE(ub.structName, QByteArrayLiteral("ubuf"));
    QCOMPARE(ub.size, 768);
    QCOMPARE(ub.members.size(), 7);

    for (const QShaderDescription::BlockVariable &var : ub.members) {
        if (var.name == QByteArrayLiteral("ECCameraPosition")) {
            QCOMPARE(var.offset, 0);
            QCOMPARE(var.size, 12);
            QCOMPARE(var.type, QShaderDescription::Vec3);
        } else if (var.name == QByteArrayLiteral("ka")) {
            QCOMPARE(var.offset, 16);
            QCOMPARE(var.size, 12);
            QCOMPARE(var.type, QShaderDescription::Vec3);
        } else if (var.name == QByteArrayLiteral("kd")) {
            QCOMPARE(var.offset, 32);
            QCOMPARE(var.size, 12);
            QCOMPARE(var.type, QShaderDescription::Vec3);
        } else if (var.name == QByteArrayLiteral("ks")) {
            QCOMPARE(var.offset, 48);
            QCOMPARE(var.size, 12);
            QCOMPARE(var.type, QShaderDescription::Vec3);
        } else if (var.name == QByteArrayLiteral("numLights")) {
            QCOMPARE(var.offset, 704);
            QCOMPARE(var.size, 4);
            QCOMPARE(var.type, QShaderDescription::Int);
        } else if (var.name == QByteArrayLiteral("mm")) {
            QCOMPARE(var.offset, 720);
            QCOMPARE(var.size, 48);
            QCOMPARE(var.type, QShaderDescription::Mat3);
            QCOMPARE(var.matrixStride, 16);
            QCOMPARE(var.matrixIsRowMajor, true);
        } else if (var.name == QByteArrayLiteral("lights")) {
            QCOMPARE(var.offset, 64);
            QCOMPARE(var.size, 640);
            QCOMPARE(var.type, QShaderDescription::Struct);
            QCOMPARE(var.arrayDims, QList<int>() << 10);
            QCOMPARE(var.structMembers.size(), 8);
            for (const QShaderDescription::BlockVariable &structVar : var.structMembers) {
                if (structVar.name == QByteArrayLiteral("ECLightPosition")) {
                    QCOMPARE(structVar.offset, 0);
                    QCOMPARE(structVar.size, 12);
                    QCOMPARE(structVar.type, QShaderDescription::Vec3);
                } else if (structVar.name == QByteArrayLiteral("attenuation")) {
                    QCOMPARE(structVar.offset, 16);
                    QCOMPARE(structVar.size, 12);
                    QCOMPARE(structVar.type, QShaderDescription::Vec3);
                } else if (structVar.name == QByteArrayLiteral("color")) {
                    QCOMPARE(structVar.offset, 32);
                    QCOMPARE(structVar.size, 12);
                    QCOMPARE(structVar.type, QShaderDescription::Vec3);
                } else if (structVar.name == QByteArrayLiteral("intensity")) {
                    QCOMPARE(structVar.offset, 48);
                    QCOMPARE(structVar.size, 4);
                    QCOMPARE(structVar.type, QShaderDescription::Float);
                } else if (structVar.name == QByteArrayLiteral("specularExp")) {
                    QCOMPARE(structVar.offset, 52);
                    QCOMPARE(structVar.size, 4);
                    QCOMPARE(structVar.type, QShaderDescription::Float);
                } else if (structVar.name == QByteArrayLiteral("padding1")) {
                    QCOMPARE(structVar.offset, 12);
                    QCOMPARE(structVar.size, 4);
                    QCOMPARE(structVar.type, QShaderDescription::Float);
                } else if (structVar.name == QByteArrayLiteral("padding2")) {
                    QCOMPARE(structVar.offset, 28);
                    QCOMPARE(structVar.size, 4);
                    QCOMPARE(structVar.type, QShaderDescription::Float);
                } else if (structVar.name == QByteArrayLiteral("padding3")) {
                    QCOMPARE(structVar.offset, 44);
                    QCOMPARE(structVar.size, 4);
                    QCOMPARE(structVar.type, QShaderDescription::Float);
                } else {
                    QFAIL(qPrintable(
                            QStringLiteral(
                                    "Unexpected member in 'lights' struct in uniform block: %1")
                                    .arg(structVar.name)));
                }
            }
        } else {
            QFAIL(qPrintable(QStringLiteral("Unexpected uniform block member: %1").arg(var.name)));
        }
    }
}

void tst_QShaderBaker::reflectCombinedImageSampler()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/sgtexture.frag"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::GlslShader, QShaderVersion(150) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());

    QShaderDescription desc = s.description();
    QCOMPARE(desc.inputVariables().size(), 1);
    QCOMPARE(desc.outputVariables().size(), 1);
    QCOMPARE(desc.uniformBlocks().size(), 1);
    QCOMPARE(desc.combinedImageSamplers().size(), 2);

    auto inputVar = desc.inputVariables().first();
    QCOMPARE(inputVar.location, 0);
    QCOMPARE(inputVar.name, QByteArrayLiteral("qt_TexCoord"));
    QCOMPARE(inputVar.type, QShaderDescription::Vec2);

    auto outputVar = desc.outputVariables().first();
    QCOMPARE(outputVar.location, 0);
    QCOMPARE(outputVar.name, QByteArrayLiteral("fragColor"));
    QCOMPARE(outputVar.type, QShaderDescription::Vec4);

    auto block = desc.uniformBlocks().first();
    QCOMPARE(block.binding, 0);
    QCOMPARE(block.size, 68);
    QCOMPARE(block.blockName, QByteArrayLiteral("buf"));
    QCOMPARE(block.structName, QByteArrayLiteral("ubuf"));
    QCOMPARE(block.members.size(), 2);
    for (int i = 0; i < block.members.size(); ++i) {
        const QShaderDescription::BlockVariable &blockVar(block.members[i]);
        switch (i) {
        case 0:
            QCOMPARE(blockVar.name, QByteArrayLiteral("qt_Matrix"));
            QCOMPARE(blockVar.offset, 0);
            QCOMPARE(blockVar.size, 64);
            QCOMPARE(blockVar.type, QShaderDescription::Mat4);
            QCOMPARE(blockVar.matrixStride, 16);
            QCOMPARE(blockVar.matrixIsRowMajor, false);
            break;
        case 1:
            QCOMPARE(blockVar.name, QByteArrayLiteral("opacity"));
            QCOMPARE(blockVar.offset, 64);
            QCOMPARE(blockVar.size, 4);
            QCOMPARE(blockVar.type, QShaderDescription::Float);
            break;
        default:
            break;
        }
    }

    for (const QShaderDescription::InOutVariable &imSampVar : desc.combinedImageSamplers()) {
        switch (imSampVar.binding) {
        case 1:
            QCOMPARE(imSampVar.name, QByteArrayLiteral("qt_Texture"));
            QCOMPARE(imSampVar.type, QShaderDescription::Sampler2D);
            break;
        case 2:
            QCOMPARE(imSampVar.name, QByteArrayLiteral("t1"));
            QCOMPARE(imSampVar.type, QShaderDescription::Sampler2D);
            break;
        default:
            QFAIL(qPrintable(QStringLiteral("Unexpected combined image sampler: %1")
                                     .arg(imSampVar.binding)));
            return;
        }
    }
}

void tst_QShaderBaker::mslNativeBindingMap()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/sgtexture.frag"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::GlslShader, QShaderVersion(150) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());

    const QShaderKey mslShaderKey(QShader::MslShader, QShaderVersion(12));
    QVERIFY(!s.nativeResourceBindingMap(mslShaderKey).isEmpty());

    QShader::NativeResourceBindingMap nativeBindingMap = s.nativeResourceBindingMap(mslShaderKey);
    QCOMPARE(nativeBindingMap.size(), 3);
    QVERIFY(nativeBindingMap.contains(0)); // uniform block
    QVERIFY(nativeBindingMap.contains(1)); // combined image sampler, maps to a texture and sampler in MSL
    QVERIFY(nativeBindingMap.contains(2)); // same

    // MSL has per-resource namespaces
    std::pair<int, int> nativeBindingPair = nativeBindingMap.value(0);
    QCOMPARE(nativeBindingPair.first, 0); // uniform buffer

    nativeBindingPair = nativeBindingMap.value(1);
    QCOMPARE(nativeBindingPair.first, 0); // texture
    QCOMPARE(nativeBindingPair.second, 0); // sampler

    nativeBindingPair = nativeBindingMap.value(2);
    QCOMPARE(nativeBindingPair.first, 1); // texture
    QCOMPARE(nativeBindingPair.second, 1); // sampler

    baker.setSourceFileName(QLatin1String(":/data/manyresources.frag"));
    targets.clear();
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());

    nativeBindingMap = s.nativeResourceBindingMap(mslShaderKey);
    QCOMPARE(nativeBindingMap.size(), 8);

    // rather won't compare with exact values, do not make assumptions, just
    // make sure there is a valid value for each since all resources are used
    // in the shader code
    QVERIFY(nativeBindingMap.value(1).first != -1);
    QVERIFY(nativeBindingMap.value(2).first != -1);
    QVERIFY(nativeBindingMap.value(3).first != -1);
    QVERIFY(nativeBindingMap.value(4).first != -1);
    QVERIFY(nativeBindingMap.value(5).first != -1);
    QVERIFY(nativeBindingMap.value(10).first != -1);
    QVERIFY(nativeBindingMap.value(11).first != -1);
    QVERIFY(nativeBindingMap.value(12).first != -1);
}

void tst_QShaderBaker::hlslNativeBindingMap()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/manyresources.frag"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());

    const QShaderKey shaderKey(QShader::HlslShader, QShaderVersion(50));
    QShader::NativeResourceBindingMap nativeBindingMap = s.nativeResourceBindingMap(shaderKey);
    QCOMPARE(nativeBindingMap.size(), 8);

    // do not assume anything about the ordering, so verify that we have valid
    // values, not the register binding values themselves

    // combined image samplers
    QVERIFY(nativeBindingMap.value(1).first != -1);
    QVERIFY(nativeBindingMap.value(1).second == nativeBindingMap.value(1).first);
    QVERIFY(nativeBindingMap.value(2).first != -1);
    QVERIFY(nativeBindingMap.value(2).second == nativeBindingMap.value(2).first);
    QVERIFY(nativeBindingMap.value(3).first != -1);
    QVERIFY(nativeBindingMap.value(3).second == nativeBindingMap.value(3).first);

    QVERIFY(nativeBindingMap.value(1).first != nativeBindingMap.value(2).first);
    QVERIFY(nativeBindingMap.value(1).first != nativeBindingMap.value(3).first);

    // storage images
    QVERIFY(nativeBindingMap.value(4).first != -1);
    QVERIFY(nativeBindingMap.value(5).first != -1);
    QVERIFY(nativeBindingMap.value(4).first != nativeBindingMap.value(5).first);

    // storage buffer
    QVERIFY(nativeBindingMap.value(10).first != -1);

    // uniform buffers
    QVERIFY(nativeBindingMap.value(11).first != -1);
    QVERIFY(nativeBindingMap.value(12).first != -1);
    QVERIFY(nativeBindingMap.value(11).first != nativeBindingMap.value(12).first);
}

void tst_QShaderBaker::reflectArraysOfSamplers()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/arrays_of_samplers.frag"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::GlslShader, QShaderVersion(150) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(20) }); // arrays of textures needs MSL 2.0
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(baker.errorMessage().isEmpty());
    QVERIFY(s.isValid());

    QShaderDescription desc = s.description();

    QCOMPARE(desc.inputVariables().size(), 1);
    QCOMPARE(desc.outputVariables().size(), 1);
    QCOMPARE(desc.combinedImageSamplers().size(), 4);

    for (const QShaderDescription::InOutVariable &var : desc.combinedImageSamplers()) {
        switch (var.binding) {
        case 4:
            QCOMPARE(var.type, QShaderDescription::Sampler2D);
            QVERIFY(var.arrayDims.isEmpty());
            break;
        case 8:
            QCOMPARE(var.type, QShaderDescription::Sampler2D);
            QCOMPARE(var.arrayDims.size(), 1);
            QCOMPARE(var.arrayDims.first(), 4);
            break;
        case 9:
            QCOMPARE(var.type, QShaderDescription::SamplerCube);
            QCOMPARE(var.arrayDims.size(), 1);
            QCOMPARE(var.arrayDims.first(), 4);
            break;
        case 10:
            QCOMPARE(var.type, QShaderDescription::Sampler2DArray);
            QVERIFY(var.arrayDims.isEmpty());
            break;
        default:
            QFAIL(qPrintable(QStringLiteral("Wrong binding value: %1").arg(var.binding)));
        }
    }
}

void tst_QShaderBaker::perTargetCompileMode()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color_pertarget.vert"));
    baker.setGeneratedShaderVariants({
                                         QShader::StandardShader,
                                         QShader::BatchableVertexShader
                                     });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(330) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);

    baker.setPreamble(QByteArrayLiteral("#define MY_VALUE 321\n#define MY_MACRO\n"));

    baker.setPerTargetCompilation(true);

    QShader s = baker.bake();
    if (!s.isValid())
        qDebug() << baker.errorMessage();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 2 * 6);

    int batchableVariantCount = 0;
    int batchableGlslVariantCount = 0;
    for (const QShaderKey &key : s.availableShaders()) {
        if (key.sourceVariant() == QShader::BatchableVertexShader) {
            ++batchableVariantCount;
            if (key.source() == QShader::GlslShader) {
                ++batchableGlslVariantCount;
                const QByteArray src = s.shader(key).shader();
                QVERIFY(src.contains(QByteArrayLiteral("_qt_order * ")));
            }
        } else {
            if (key.source() == QShader::GlslShader) {
                // make sure it did not get rewritten
                const QByteArray src = s.shader(key).shader();
                QVERIFY(!src.contains(QByteArrayLiteral("_qt_order * ")));
            }
        }
    }
    QCOMPARE(batchableVariantCount, 6);
    QCOMPARE(batchableGlslVariantCount, 3);

    // now compile something that succeeds only if the QSHADER_SPIRV macros are
    // present as expected
    baker.setSourceFileName(QLatin1String(":/data/color_spirv_only.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    targets.clear();
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);
    baker.setPreamble(QByteArray());
    s = baker.bake();
    if (!s.isValid())
        qDebug() << baker.errorMessage();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 1);

    // now something similar, with GLSL. This will mean no QSHADER_SPIRV,
    // QSHADER_HLSL, QSHADER_MSL are defined (and is an error if they are)
    baker.setSourceFileName(QLatin1String(":/data/color_glsl_only.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    targets.clear();
    targets.append({ QShader::GlslShader, QShaderVersion(330) });
    targets.append({ QShader::GlslShader, QShaderVersion(440) });
    baker.setGeneratedShaders(targets);
    s = baker.bake();
    if (!s.isValid())
        qDebug() << baker.errorMessage();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 2);

    // HLSL 5.0 + MSL 1.2 (fails with others)
    baker.setSourceFileName(QLatin1String(":/data/color_hlsl_msl_only.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    targets.clear();
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    s = baker.bake();
    if (!s.isValid())
        qDebug() << baker.errorMessage();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 2);
}

void tst_QShaderBaker::serializeDeserialize()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 5);

    QByteArray data = s.serialized();
    QVERIFY(!data.isEmpty());

    QShader s2;
    QVERIFY(!s2.isValid());
    s2 = QShader::fromSerialized(data);
    QVERIFY(s2.isValid());
    QCOMPARE(s, s2);
}

void tst_QShaderBaker::spirvOptions()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);

    // no options specified (no full debug info, no strip)
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 1);
    const QShaderKey key(QShader::SpirvShader, QShaderVersion(100));
    QVERIFY(s.availableShaders().contains(key));
    QCOMPARE(s.shader(key).entryPoint(), "main");
    QByteArray bin = s.shader(key).shader();

    // full debug info
    baker.setSpirvOptions(QShaderBaker::SpirvOption::GenerateFullDebugInfo);
    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 1);
    QCOMPARE(s.shader(key).entryPoint(), "main");
    QByteArray debugBin = s.shader(key).shader();

    QVERIFY(bin != debugBin);
    QVERIFY(debugBin.size() > bin.size());

    // strip all (variable names, etc.)
    baker.setSpirvOptions(QShaderBaker::SpirvOption::StripDebugAndVarInfo);
    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 1);
    QCOMPARE(s.shader(key).entryPoint(), "main");
    QByteArray strippedBin = s.shader(key).shader();

    QVERIFY(strippedBin != debugBin);
    QVERIFY(debugBin.size() > strippedBin.size());
    QVERIFY(strippedBin != bin);
    QVERIFY(bin.size() > strippedBin.size());
}

void tst_QShaderBaker::separateImagesAndSamplers()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/combined_and_separate_samplers.frag"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(330) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());

    for (QShaderKey shaderKey : {
         QShaderKey(QShader::HlslShader, QShaderVersion(50)),
         QShaderKey(QShader::MslShader, QShaderVersion(12)) })
    {
        QShader::NativeResourceBindingMap nativeBindingMap = s.nativeResourceBindingMap(shaderKey);
        QCOMPARE(nativeBindingMap.size(), 4);

        // binding 1 is a combined image sampler
        QVERIFY(nativeBindingMap.value(1).first != -1);
        QVERIFY(nativeBindingMap.value(1).second != -1);

        // binding 2 is a separate image
        QVERIFY(nativeBindingMap.value(2).first != -1);
        // presumably binding 1 got t0, then this is probably t1, the point is that they should be something different
        QVERIFY(nativeBindingMap.value(2).first != nativeBindingMap.value(1).first);

        // binding 3 is a separate sampler
        QVERIFY(nativeBindingMap.value(3).first != -1);

        // binding 4 as well
        QVERIFY(nativeBindingMap.value(4).first != -1);
        QVERIFY(nativeBindingMap.value(4).first != nativeBindingMap.value(3).first);

        QVERIFY(s.separateToCombinedImageSamplerMappingList(shaderKey).isEmpty());
    }

    // Now onto to the real special deal here: OpenGL. That has its own mapping
    // table because SPIRV-Cross auto-generates a "combined image sampler"
    // (e.g. a sampler2D) for each separate texture+sampler combination.
    for (QShaderKey shaderKey : {
         QShaderKey(QShader::GlslShader, QShaderVersion(330)),
         QShaderKey(QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs)) })
    {
        QShader::SeparateToCombinedImageSamplerMappingList list = s.separateToCombinedImageSamplerMappingList(shaderKey);
        QCOMPARE(list.size(), 2);
        for (int i = 0; i < list.size(); ++i) {
            int tbinding = list.at(i).textureBinding;
            int sbinding = list.at(i).samplerBinding;
            QCOMPARE(tbinding, 2);
            QVERIFY(sbinding == 3 || sbinding == 4);
            QByteArray combinedName = list.at(i).combinedSamplerName;
            QVERIFY(!combinedName.isEmpty());
        }

        QVERIFY(s.nativeResourceBindingMap(shaderKey).isEmpty());
    }
}

void tst_QShaderBaker::tessellationForMetalVertCompile()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader,
                                       QShader::NonIndexedVertexAsComputeShader,
                                       QShader::UInt16IndexedVertexAsComputeShader,
                                       QShader::UInt32IndexedVertexAsComputeShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(430) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QVERIFY(s.stage() == QShader::VertexStage);

    QCOMPARE(s.availableShaders().size(), 6);
    QVERIFY(!s.shader(QShaderKey(QShader::MslShader, QShaderVersion(12), QShader::NonIndexedVertexAsComputeShader)).shader().isEmpty());
    QVERIFY(!s.shader(QShaderKey(QShader::MslShader, QShaderVersion(12), QShader::UInt16IndexedVertexAsComputeShader)).shader().isEmpty());
    QVERIFY(!s.shader(QShaderKey(QShader::MslShader, QShaderVersion(12), QShader::UInt32IndexedVertexAsComputeShader)).shader().isEmpty());

    QShaderDescription desc = s.description();
    QCOMPARE(desc.outputBuiltinVariables().size(), 1);
    QCOMPARE(desc.outputBuiltinVariables()[0].type, QShaderDescription::PositionBuiltin);

    QShader::NativeShaderInfo info = s.nativeShaderInfo(QShaderKey(QShader::MslShader, QShaderVersion(12), QShader::NonIndexedVertexAsComputeShader));
    QCOMPARE(info.extraBufferBindings.size(), 1);
    QVERIFY(info.extraBufferBindings.value(QShaderPrivate::MslTessVertTescOutputBufferBinding, -1) >= 0);

    info = s.nativeShaderInfo(QShaderKey(QShader::MslShader, QShaderVersion(12), QShader::UInt32IndexedVertexAsComputeShader));
    QCOMPARE(info.extraBufferBindings.size(), 2);
    const int outVertBufIndex = info.extraBufferBindings.value(QShaderPrivate::MslTessVertTescOutputBufferBinding, -1);
    QVERIFY(outVertBufIndex >= 0);
    QVERIFY(info.extraBufferBindings.value(QShaderPrivate::MslTessVertIndicesBufferBinding, -1) >= 0);
    QVERIFY(info.extraBufferBindings.value(QShaderPrivate::MslTessVertIndicesBufferBinding, -1) != outVertBufIndex);
}

void tst_QShaderBaker::tessellationCompile()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/tess.tesc"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(430) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);
    baker.setTessellationMode(QShaderDescription::TrianglesTessellationMode);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(s.stage() == QShader::TessellationControlStage);
    QVERIFY(baker.errorMessage().isEmpty());

    QCOMPARE(s.availableShaders().size(), 3);
    QShaderDescription desc = s.description();
    // builtins are expected to be stored sorted by type
    QCOMPARE(desc.inputBuiltinVariables()[0].type, QShaderDescription::PositionBuiltin);
    QCOMPARE(desc.inputBuiltinVariables()[1].type, QShaderDescription::InvocationIdBuiltin);
    QCOMPARE(desc.outputBuiltinVariables()[0].type, QShaderDescription::PositionBuiltin);
    QCOMPARE(desc.outputBuiltinVariables()[1].type, QShaderDescription::TessLevelOuterBuiltin);
    QCOMPARE(desc.outputBuiltinVariables()[2].type, QShaderDescription::TessLevelInnerBuiltin);

    QCOMPARE(desc.tessellationOutputVertexCount(), uint(3));

    QCOMPARE(desc.outputVariables().size(), 3);
    for (const QShaderDescription::InOutVariable &v : desc.outputVariables()) {
        switch (v.location) {
        case 0:
            QCOMPARE(v.name, QByteArrayLiteral("outColor"));
            QCOMPARE(v.type, QShaderDescription::Vec3);
            QCOMPARE(v.perPatch, false);
            break;
        case 1:
            QCOMPARE(v.name, QByteArrayLiteral("stuff"));
            QCOMPARE(v.type, QShaderDescription::Vec3);
            QCOMPARE(v.perPatch, true);
            break;
        case 2:
            QCOMPARE(v.name, QByteArrayLiteral("more_stuff"));
            QCOMPARE(v.type, QShaderDescription::Float);
            QCOMPARE(v.perPatch, true);
            break;
        default:
            QFAIL(qPrintable(QStringLiteral("Bad location: %1").arg(v.location)));
            break;
        }
    }

    QVERIFY(s.shader(QShaderKey(QShader::MslShader, QShaderVersion(12))).shader().contains(QByteArrayLiteral("MTLTriangleTessellationFactorsHalf")));

    QCOMPARE(s.nativeShaderInfo(QShaderKey(QShader::MslShader, QShaderVersion(12))).extraBufferBindings.size(), 5);

    baker.setSourceFileName(QLatin1String(":/data/tess.tese"));
    baker.setGeneratedShaders(targets);
    baker.setTessellationOutputVertexCount(3);
    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(s.stage() == QShader::TessellationEvaluationStage);
    QVERIFY(baker.errorMessage().isEmpty());

    QCOMPARE(s.availableShaders().size(), 3);
    desc = s.description();
    QCOMPARE(desc.inputBuiltinVariables()[0].type, QShaderDescription::PositionBuiltin);
    QCOMPARE(desc.inputBuiltinVariables()[1].type, QShaderDescription::TessLevelOuterBuiltin);
    QCOMPARE(desc.inputBuiltinVariables()[2].type, QShaderDescription::TessLevelInnerBuiltin);
    QCOMPARE(desc.inputBuiltinVariables()[3].type, QShaderDescription::TessCoordBuiltin);

    QCOMPARE(desc.tessellationMode(), QShaderDescription::TrianglesTessellationMode);
    QCOMPARE(desc.tessellationWindingOrder(), QShaderDescription::CcwTessellationWindingOrder);
    QCOMPARE(desc.tessellationPartitioning(), QShaderDescription::FractionalOddTessellationPartitioning);

    QCOMPARE(desc.inputVariables().size(), 3);
    for (const QShaderDescription::InOutVariable &v : desc.inputVariables()) {
        switch (v.location) {
        case 0:
            QCOMPARE(v.name, QByteArrayLiteral("inColor"));
            QCOMPARE(v.type, QShaderDescription::Vec3);
            QCOMPARE(v.perPatch, false);
            break;
        case 1:
            QCOMPARE(v.name, QByteArrayLiteral("stuff"));
            QCOMPARE(v.type, QShaderDescription::Vec3);
            QCOMPARE(v.perPatch, true);
            break;
        case 2:
            QCOMPARE(v.name, QByteArrayLiteral("more_stuff"));
            QCOMPARE(v.type, QShaderDescription::Float);
            QCOMPARE(v.perPatch, true);
            break;
        default:
            QFAIL(qPrintable(QStringLiteral("Bad location: %1").arg(v.location)));
            break;
        }
    }

    // the uniform buffer binding -> Metal buffer index mapping
    QCOMPARE(s.nativeResourceBindingMap(QShaderKey(QShader::MslShader, QShaderVersion(12))).size(), 1);
    QCOMPARE(s.nativeResourceBindingMap(QShaderKey(QShader::MslShader, QShaderVersion(12))).value(0), std::make_pair(0, -1));

    QVERIFY(s.shader(QShaderKey(QShader::MslShader, QShaderVersion(12))).shader().contains(QByteArrayLiteral("[[ patch(triangle, 3) ]]")));
}

void tst_QShaderBaker::tessellationCompileWithChangedTessArgs()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/tess.tesc"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);

    // not what the .tese would say but let's pretend
    baker.setTessellationMode(QShaderDescription::QuadTessellationMode);

    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());

    QVERIFY(s.shader(QShaderKey(QShader::MslShader, QShaderVersion(12))).shader().contains(QByteArrayLiteral("MTLQuadTessellationFactorsHalf")));

    baker.setSourceFileName(QLatin1String(":/data/tess.tese"));
    baker.setGeneratedShaders(targets);

    baker.setTessellationOutputVertexCount(13);

    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(s.stage() == QShader::TessellationEvaluationStage);
    QVERIFY(baker.errorMessage().isEmpty());

    QVERIFY(s.shader(QShaderKey(QShader::MslShader, QShaderVersion(12))).shader().contains(QByteArrayLiteral("[[ patch(triangle, 13) ]]")));
}

void tst_QShaderBaker::dontBreakOnTranslationError()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/texturesize.frag"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });

    QVector<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(300, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });
    baker.setGeneratedShaders(targets);

    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QCOMPARE(s.availableShaders().size(), 5);
    QVERIFY(baker.errorMessage().isEmpty());

    // now add a problematic target (no textureSize() in GLSL ES 100)
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    baker.setGeneratedShaders(targets);

    QVERIFY(!baker.bake().isValid());
    QVERIFY(!baker.errorMessage().isEmpty());

    // now make it succeed by skipping the failing targets
    baker.setBreakOnShaderTranslationError(false);
    s = baker.bake();
    QVERIFY(s.isValid());

    // somewhat undocumented but the error message still contains the
    // SPIRV-Cross error message about GLSL ES 100 even though the
    // baking succeeded
    QVERIFY(!baker.errorMessage().isEmpty());

    QCOMPARE(s.availableShaders().size(), 5);
}


void tst_QShaderBaker::storageImageFlags()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/imageloadstore.comp"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });

    QVector<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);

    QShader s = baker.bake();
    QVERIFY(s.isValid());

    QCOMPARE(s.description().storageImages().size(), 4);

    QCOMPARE(s.description().storageImages().at(0).binding, 0);
    QCOMPARE(s.description().storageImages().at(0).imageFlags, QShaderDescription::ImageFlags());

    QCOMPARE(s.description().storageImages().at(1).binding, 1);
    QCOMPARE(s.description().storageImages().at(1).imageFlags,
             QShaderDescription::ImageFlags(QShaderDescription::ImageFlag::WriteOnlyImage));

    QCOMPARE(s.description().storageImages().at(2).binding, 2);
    QCOMPARE(s.description().storageImages().at(2).imageFlags,
             QShaderDescription::ImageFlags(QShaderDescription::ImageFlag::ReadOnlyImage));

    QCOMPARE(s.description().storageImages().at(3).binding, 3);
    QCOMPARE(s.description().storageImages().at(3).imageFlags,
             QShaderDescription::ImageFlags(QShaderDescription::ImageFlag::ReadOnlyImage
                                            | QShaderDescription::ImageFlag::WriteOnlyImage));
}

void tst_QShaderBaker::storageBufferRuntimeArrayStride()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/storagebuffer.comp"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });

    QVector<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);

    QShader s = baker.bake();
    QVERIFY(s.isValid());

    QCOMPARE(s.description().storageBlocks().size(), 8);

    QCOMPARE(s.description().storageBlocks().at(0).binding, 0);
    QCOMPARE(s.description().storageBlocks().at(0).runtimeArrayStride, 4);

    QCOMPARE(s.description().storageBlocks().at(1).binding, 1);
    QCOMPARE(s.description().storageBlocks().at(1).runtimeArrayStride, 16);

    QCOMPARE(s.description().storageBlocks().at(2).binding, 2);
    QCOMPARE(s.description().storageBlocks().at(2).runtimeArrayStride, 8);

    QCOMPARE(s.description().storageBlocks().at(3).binding, 3);
    QCOMPARE(s.description().storageBlocks().at(3).runtimeArrayStride, 16);

    QCOMPARE(s.description().storageBlocks().at(4).binding, 4);
    QCOMPARE(s.description().storageBlocks().at(4).runtimeArrayStride, 16);

    QCOMPARE(s.description().storageBlocks().at(5).binding, 5);
    QCOMPARE(s.description().storageBlocks().at(5).runtimeArrayStride, 16);

    QCOMPARE(s.description().storageBlocks().at(6).binding, 6);
    QCOMPARE(s.description().storageBlocks().at(6).runtimeArrayStride, 16);

    QCOMPARE(s.description().storageBlocks().at(7).binding, 7);
    QCOMPARE(s.description().storageBlocks().at(7).runtimeArrayStride, 16);
}

void tst_QShaderBaker::storageBufferQualifiers()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/storagebuffer.comp"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });

    QVector<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    baker.setGeneratedShaders(targets);

    QShader s = baker.bake();
    QVERIFY(s.isValid());

    // GLSL 4.60 Specification - Section 4.10 Memory Qualifiers:
    // Since the external source reading or writing a volatile variable may be another shader
    // invocation, variables declared as volatile are automatically treated as coherent.

    QCOMPARE(s.description().storageBlocks().size(), 8);

    QCOMPARE(s.description().storageBlocks().at(0).binding, 0);
    QCOMPARE(s.description().storageBlocks().at(0).qualifierFlags,
             QShaderDescription::QualifierFlags());

    QCOMPARE(s.description().storageBlocks().at(1).binding, 1);
    QCOMPARE(s.description().storageBlocks().at(1).qualifierFlags,
             QShaderDescription::QualifierFlags(QShaderDescription::QualifierCoherent));

    QCOMPARE(s.description().storageBlocks().at(2).binding, 2);
    QCOMPARE(s.description().storageBlocks().at(2).qualifierFlags,
             QShaderDescription::QualifierFlags(QShaderDescription::QualifierWriteOnly
                                                | QShaderDescription::QualifierVolatile
                                                | QShaderDescription::QualifierCoherent));

    QCOMPARE(s.description().storageBlocks().at(3).binding, 3);
    QCOMPARE(s.description().storageBlocks().at(3).qualifierFlags,
             QShaderDescription::QualifierFlags(QShaderDescription::QualifierWriteOnly
                                                | QShaderDescription::QualifierRestrict));

    QCOMPARE(s.description().storageBlocks().at(4).binding, 4);
    QCOMPARE(s.description().storageBlocks().at(4).qualifierFlags,
             QShaderDescription::QualifierFlags(QShaderDescription::QualifierReadOnly));

    QCOMPARE(s.description().storageBlocks().at(5).binding, 5);
    QCOMPARE(s.description().storageBlocks().at(5).qualifierFlags,
             QShaderDescription::QualifierFlags(QShaderDescription::QualifierReadOnly
                                                | QShaderDescription::QualifierCoherent));

    QCOMPARE(s.description().storageBlocks().at(6).binding, 6);
    QCOMPARE(s.description().storageBlocks().at(6).qualifierFlags,
             QShaderDescription::QualifierFlags(QShaderDescription::QualifierWriteOnly
                                                | QShaderDescription::QualifierReadOnly
                                                | QShaderDescription::QualifierVolatile
                                                | QShaderDescription::QualifierCoherent));

    QCOMPARE(s.description().storageBlocks().at(7).binding, 7);
    QCOMPARE(s.description().storageBlocks().at(7).qualifierFlags,
             QShaderDescription::QualifierFlags(QShaderDescription::QualifierWriteOnly
                                                | QShaderDescription::QualifierReadOnly
                                                | QShaderDescription::QualifierRestrict));
}

void tst_QShaderBaker::multiview()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/multiview_simple.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });

    QVector<QShaderBaker::GeneratedShader> targets;
    // SPV_KHR_multiview
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    // SV_ViewID is in shader model 6.1
    targets.append({ QShader::HlslShader, QShaderVersion(61) });
    // GL_OVR_multiview(2) is written for OpenGL 3.0 / OpenGL ES 3.0 and newer, just pick
    // some GLSL versions accordingly for the test
    targets.append({ QShader::GlslShader, QShaderVersion(150) });
    targets.append({ QShader::GlslShader, QShaderVersion(300, QShaderVersion::GlslEs) });
    baker.setGeneratedShaders(targets);

    // should fail due to GLSL
    QShader s = baker.bake();
    QVERIFY(!s.isValid());
    QVERIFY(!baker.errorMessage().isEmpty());
    qDebug() << baker.errorMessage();

    // now provide num_views and it should succeed
    baker.setMultiViewCount(2);
    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
}

void tst_QShaderBaker::mediump()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/color.vert"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
    targets.append({ QShader::GlslShader, QShaderVersion(120) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    targets.append({ QShader::MslShader, QShaderVersion(12) });

    baker.setGlslOptions(QShaderBaker::GlslOption::GlslEsFragDefaultFloatPrecisionMedium);

    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 5);
    QShaderCode shader = s.shader(QShaderKey(QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs)));
    QVERIFY(!shader.shader().isEmpty());
    QVERIFY(!shader.shader().contains("precision ")); // no effect for vertex shaders

    baker.setSourceFileName(QLatin1String(":/data/color.frag"));

    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 5);
    shader = s.shader(QShaderKey(QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs)));
    QVERIFY(!shader.shader().isEmpty());
    QVERIFY(shader.shader().contains("precision mediump float;"));

    // back to defaults
    baker.setGlslOptions({});
    s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 5);
    shader = s.shader(QShaderKey(QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs)));
    QVERIFY(!shader.shader().isEmpty());
    QVERIFY(shader.shader().contains("precision highp float;"));
}

void tst_QShaderBaker::geometryShader()
{
    QShaderBaker baker;
    baker.setSourceFileName(QLatin1String(":/data/voxelize.geom"));
    baker.setGeneratedShaderVariants({ QShader::StandardShader });
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) });
    targets.append({ QShader::GlslShader, QShaderVersion(410) });
    targets.append({ QShader::GlslShader, QShaderVersion(320, QShaderVersion::GlslEs) });
    targets.append({ QShader::HlslShader, QShaderVersion(50) });
    baker.setGeneratedShaders(targets);
    QShader s = baker.bake();
    QVERIFY(s.isValid());
    QVERIFY(s.stage() == QShader::GeometryStage);
    QVERIFY(baker.errorMessage().isEmpty());
    QCOMPARE(s.availableShaders().size(), 4);
}


#include <tst_qshaderbaker.moc>
QTEST_MAIN(tst_QShaderBaker)
