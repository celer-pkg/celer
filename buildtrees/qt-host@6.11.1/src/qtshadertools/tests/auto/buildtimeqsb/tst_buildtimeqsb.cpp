// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QFile>
#include <rhi/qshader.h>
#include <rhi/qrhi.h>

class tst_BuildTimeQsb : public QObject
{
    Q_OBJECT

private slots:
    void defaultAddShaders();
    void customTargets();
    void withDefines();
    void replacements();
    void createPipeline();
    void tessellation();
    void multiviewSimple();
    void multiviewAutomatic();
};

static QShader getShader(const QString &name)
{
    QFile f(name);
    if (f.open(QIODevice::ReadOnly))
        return QShader::fromSerialized(f.readAll());

    return QShader();
}

static const int defaultShaderCount = 6;
static const QShaderKey defaultShaderKeys[defaultShaderCount] = {
    QShaderKey(QShader::SpirvShader, QShaderVersion(100)),
    QShaderKey(QShader::HlslShader, QShaderVersion(50)),
    QShaderKey(QShader::MslShader, QShaderVersion(12)),
    QShaderKey(QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs)),
    QShaderKey(QShader::GlslShader, QShaderVersion(120)),
    QShaderKey(QShader::GlslShader, QShaderVersion(150)),
};

void tst_BuildTimeQsb::defaultAddShaders()
{
    // "shaders"
    QShader color_vert = getShader(QLatin1String(":/test/color.vert.qsb"));
    QVERIFY(color_vert.isValid());
    QCOMPARE(color_vert.availableShaders().size(), defaultShaderCount);
    for (int i = 0; i < defaultShaderCount; ++i)
        QVERIFY(color_vert.availableShaders().contains(defaultShaderKeys[i]));

    QShader color_frag = getShader(QLatin1String(":/test/color.frag.qsb"));
    QVERIFY(color_frag.isValid());
    QCOMPARE(color_frag.availableShaders().size(), defaultShaderCount);
    for (int i = 0; i < defaultShaderCount; ++i)
        QVERIFY(color_frag.availableShaders().contains(defaultShaderKeys[i]));

    // "shaders_precompile"
    QShader color_vert_pre = getShader(QLatin1String(":/test/color_precomp.vert.qsb"));
    QVERIFY(color_vert_pre.isValid());
    QCOMPARE(color_vert_pre.availableShaders().size(), defaultShaderCount);

    QShader color_frag_pre = getShader(QLatin1String(":/test/color_precomp.frag.qsb"));
    QVERIFY(color_frag_pre.isValid());
    QCOMPARE(color_frag_pre.availableShaders().size(), defaultShaderCount);

    // "shaders_in_subdir"
    QShader texture_vert = getShader(QLatin1String(":/some/prefix/subdir/texture.vert.qsb"));
    QVERIFY(texture_vert.isValid());
    QCOMPARE(texture_vert.availableShaders().size(), defaultShaderCount);
    for (int i = 0; i < defaultShaderCount; ++i)
        QVERIFY(texture_vert.availableShaders().contains(defaultShaderKeys[i]));

    QShader texture_frag = getShader(QLatin1String(":/some/prefix/subdir/texture.frag.qsb"));
    QVERIFY(texture_frag.isValid());
    QCOMPARE(texture_frag.availableShaders().size(), defaultShaderCount);
    for (int i = 0; i < defaultShaderCount; ++i)
        QVERIFY(texture_frag.availableShaders().contains(defaultShaderKeys[i]));

    // "shaders_in_subdir_with_outputs_as_alias"
    QShader alias_texture_vert = getShader(QLatin1String(":/some/prefix/alias_texture.vert.qsb"));
    QVERIFY(alias_texture_vert.isValid());
    QCOMPARE(alias_texture_vert.availableShaders().size(), defaultShaderCount);
    for (int i = 0; i < defaultShaderCount; ++i)
        QVERIFY(alias_texture_vert.availableShaders().contains(defaultShaderKeys[i]));

    QShader alias_texture_frag = getShader(QLatin1String(":/some/prefix/x/y/z/alias_texture.frag.qsb"));
    QVERIFY(alias_texture_frag.isValid());
    QCOMPARE(alias_texture_frag.availableShaders().size(), defaultShaderCount);
    for (int i = 0; i < defaultShaderCount; ++i)
        QVERIFY(alias_texture_frag.availableShaders().contains(defaultShaderKeys[i]));

    // "shaders_in_subdir_with_base"
    QShader texture2_vert = getShader(QLatin1String(":/base_test/texture2.vert.qsb"));
    QVERIFY(texture2_vert.isValid());
    QCOMPARE(texture2_vert.availableShaders().size(), defaultShaderCount * 2); // batchable
}

void tst_BuildTimeQsb::customTargets()
{
    QShader color_vert = getShader(QLatin1String(":/test/color_1.vert.qsb"));
    QVERIFY(color_vert.isValid());
    QCOMPARE(color_vert.availableShaders().size(), 3);
    QVERIFY(color_vert.availableShaders().contains(QShaderKey(QShader::SpirvShader, QShaderVersion(100))));
    QVERIFY(color_vert.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(300, QShaderVersion::GlslEs))));
    QVERIFY(color_vert.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(330))));

    QShader color_frag = getShader(QLatin1String(":/test/color_1.frag.qsb"));
    QVERIFY(color_frag.isValid());
    QCOMPARE(color_frag.availableShaders().size(), 3);
    QVERIFY(color_frag.availableShaders().contains(QShaderKey(QShader::SpirvShader, QShaderVersion(100))));
    QVERIFY(color_frag.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(300, QShaderVersion::GlslEs))));
    QVERIFY(color_frag.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(330))));

    QShader bat_color_vert = getShader(QLatin1String(":/test/color_1b.vert.qsb"));
    QVERIFY(bat_color_vert.isValid());
    QCOMPARE(bat_color_vert.availableShaders().size(), 6);
    QVERIFY(bat_color_vert.availableShaders().contains(QShaderKey(QShader::SpirvShader, QShaderVersion(100))));
    QVERIFY(bat_color_vert.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(300, QShaderVersion::GlslEs))));
    QVERIFY(bat_color_vert.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(330))));
    QVERIFY(bat_color_vert.availableShaders().contains(QShaderKey(QShader::SpirvShader, QShaderVersion(100), QShader::BatchableVertexShader)));
    QVERIFY(bat_color_vert.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(300, QShaderVersion::GlslEs), QShader::BatchableVertexShader)));
    QVERIFY(bat_color_vert.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(330), QShader::BatchableVertexShader)));

    QShader bat_color_frag = getShader(QLatin1String(":/test/color_1b.frag.qsb"));
    QVERIFY(bat_color_frag.isValid());
    QCOMPARE(bat_color_frag.availableShaders().size(), 3); // batchable applies to vertex shaders only
    QVERIFY(bat_color_frag.availableShaders().contains(QShaderKey(QShader::SpirvShader, QShaderVersion(100))));
    QVERIFY(bat_color_frag.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(300, QShaderVersion::GlslEs))));
    QVERIFY(bat_color_frag.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(330))));
}

void tst_BuildTimeQsb::withDefines()
{
    QShader s = getShader(QLatin1String(":/subdir/test/texture_def.frag.qsb"));
    QVERIFY(s.isValid());
    QCOMPARE(s.availableShaders().size(), defaultShaderCount);
    for (int i = 0; i < defaultShaderCount; ++i)
        QVERIFY(s.availableShaders().contains(defaultShaderKeys[i]));
}

void tst_BuildTimeQsb::replacements()
{
    QShader s = getShader(QLatin1String(":/test/color_repl.vert.qsb"));
    QVERIFY(s.isValid());
    QCOMPARE(s.availableShaders().size(), defaultShaderCount);
    for (int i = 0; i < defaultShaderCount; ++i)
        QVERIFY(s.availableShaders().contains(defaultShaderKeys[i]));

    QByteArray src = s.shader(QShaderKey(QShader::SpirvShader, QShaderVersion(100))).shader();
    QVERIFY(!src.isEmpty());
    QCOMPARE(src.left(21), QByteArrayLiteral("Not very valid SPIR-V"));

    src = s.shader(QShaderKey(QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs))).shader();
    QVERIFY(!src.isEmpty());
    QCOMPARE(src.left(7), QByteArrayLiteral("Test r1"));

    src = s.shader(QShaderKey(QShader::HlslShader, QShaderVersion(50))).shader();
    QVERIFY(!src.isEmpty());
    QCOMPARE(src.left(7), QByteArrayLiteral("Test r3"));

    s = getShader(QLatin1String(":/test/x/color_repl.frag.qsb"));
    QVERIFY(s.isValid());
    QCOMPARE(s.availableShaders().size(), defaultShaderCount);
    for (int i = 0; i < defaultShaderCount; ++i)
        QVERIFY(s.availableShaders().contains(defaultShaderKeys[i]));

    src = s.shader(QShaderKey(QShader::MslShader, QShaderVersion(12))).shader();
    QVERIFY(!src.isEmpty());
    QCOMPARE(src.left(7), QByteArrayLiteral("Test r4"));
}

void tst_BuildTimeQsb::createPipeline()
{
    // this focuses on loading up a vertex and fragment shader from .qsb files
    // that were generated with PRECOMPILE, and may be relevant mainly for D3D
    // and Metal where the flag may have an actual effect (not guaranteed), so
    // test only those

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS) || defined(Q_OS_WIN)

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    QRhiMetalInitParams initParams;
    QRhi::Implementation backend = QRhi::Metal;
#elif defined(Q_OS_WIN)
    QRhiD3D11InitParams initParams;
    QRhi::Implementation backend = QRhi::D3D11;
#endif

    QScopedPointer<QRhi> rhi(QRhi::create(backend, &initParams, {}, nullptr));
    if (!rhi)
        QSKIP("Failed to create QRhi, skipping pipeline creation test");

    qDebug() << rhi->driverInfo();

    QScopedPointer<QRhiTexture> texture(rhi->newTexture(QRhiTexture::RGBA8, QSize(256, 256), 1, QRhiTexture::RenderTarget));
    QVERIFY(texture->create());
    QScopedPointer<QRhiTextureRenderTarget> rt(rhi->newTextureRenderTarget({ texture.data() }));
    QScopedPointer<QRhiRenderPassDescriptor> rpDesc(rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rpDesc.data());
    QVERIFY(rt->create());

    QScopedPointer<QRhiShaderResourceBindings> srb(rhi->newShaderResourceBindings());
    QVERIFY(srb->create());

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ { 6 * sizeof(float) } });
    inputLayout.setAttributes({ { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
                                { 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }
                              });

    QShader color_vert_pre = getShader(QLatin1String(":/test/color_precomp.vert.qsb"));
    QVERIFY(color_vert_pre.isValid());
    QShader color_frag_pre = getShader(QLatin1String(":/test/color_precomp.frag.qsb"));
    QVERIFY(color_frag_pre.isValid());
    QScopedPointer<QRhiGraphicsPipeline> pipeline;
    pipeline.reset(rhi->newGraphicsPipeline());
    pipeline->setShaderStages({ { QRhiShaderStage::Vertex, color_vert_pre }, { QRhiShaderStage::Fragment, color_frag_pre } });
    pipeline->setVertexInputLayout(inputLayout);
    pipeline->setShaderResourceBindings(srb.data());
    pipeline->setRenderPassDescriptor(rpDesc.data());
    QVERIFY(pipeline->create());

#else
    QSKIP("Skipping pipeline creation test on this platform");
#endif
}

void tst_BuildTimeQsb::tessellation()
{
    QShader s = getShader(QLatin1String(":/test/tess.vert.qsb"));
    QVERIFY(s.isValid());
    QCOMPARE(s.availableShaders().size(), 8); // SPIR-V, 2xGLSL, HLSL, 4xMSL

    s = getShader(QLatin1String(":/test/tess.tesc.qsb"));
    QVERIFY(s.isValid());
    QCOMPARE(s.availableShaders().size(), 5); // SPIR-V, 2xGLSL, HLSL, MSL

    s = getShader(QLatin1String(":/test/tess.tese.qsb"));
    QVERIFY(s.isValid());
    QCOMPARE(s.availableShaders().size(), 5); // SPIR-V, 2xGLSL, HLSL, MSL

    s = getShader(QLatin1String(":/test/tess.frag.qsb"));
    QVERIFY(s.isValid());
    QCOMPARE(s.availableShaders().size(), 5); // SPIR-V, 2xGLSL, HLSL, MSL
}

void tst_BuildTimeQsb::multiviewSimple()
{
    QShader s = getShader(QLatin1String(":/test/color_multiview.vert.qsb"));
    QVERIFY(s.isValid());
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(330))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::HlslShader, QShaderVersion(61))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::MslShader, QShaderVersion(12))));

    s = getShader(QLatin1String(":/test/color_multiview.frag.qsb"));
    QVERIFY(s.isValid());
}

void tst_BuildTimeQsb::multiviewAutomatic()
{
    // these are the non-multiview versions
    QShader s = getShader(QLatin1String(":/test/color_multiview_def.vert.qsb"));
    QVERIFY(s.isValid());
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(120))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::HlslShader, QShaderVersion(50))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::MslShader, QShaderVersion(12))));

    s = getShader(QLatin1String(":/test/color_multiview_def.frag.qsb"));
    QVERIFY(s.isValid());
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(120))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::HlslShader, QShaderVersion(50))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::MslShader, QShaderVersion(12))));

    // the view count 2 versions with the special suffix
    s = getShader(QLatin1String(":/test/color_multiview_def.vert.qsb.mv2qsb"));
    QVERIFY(s.isValid());
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::SpirvShader, QShaderVersion(100))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(330))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::HlslShader, QShaderVersion(61))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::MslShader, QShaderVersion(21))));

    s = getShader(QLatin1String(":/test/color_multiview_def.frag.qsb.mv2qsb"));
    QVERIFY(s.isValid());
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::SpirvShader, QShaderVersion(100))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::GlslShader, QShaderVersion(330))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::HlslShader, QShaderVersion(61))));
    QVERIFY(s.availableShaders().contains(QShaderKey(QShader::MslShader, QShaderVersion(21))));
}

#include <tst_buildtimeqsb.moc>
QTEST_MAIN(tst_BuildTimeQsb)
