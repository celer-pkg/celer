// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtCore/qfile.h>
#include <QtCore/qbytearray.h>

#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3ddefaultmaterial_p.h>
#include <QtQuick3D/private/qquick3dcustommaterial_p.h>
#include <QtQuick3D/private/qquick3deffect_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3D/private/qquick3dinstancing_p.h>

#include <parser.h>

#include <QtGui/qquaternion.h>

class Shadergen : public QObject
{
    Q_OBJECT
public:
    Shadergen() = default;
    ~Shadergen() = default;

private Q_SLOTS:
    void initTestCase();
    void tst_principledMaterialComponent();
    void tst_defaultMaterialComponent();
    void tst_customMaterialComponent();
    void tst_customMaterialUniforms_data();
    void tst_customMaterialUniforms();
    void tst_effectComponent();
    void tst_effectUniforms_data();
    void tst_effectUniforms();
    void tst_componentResolving();
    void tst_instancing();

private:
    MaterialParser::SceneData lastSceneData;
};

void Shadergen::initTestCase()
{
}

void Shadergen::tst_principledMaterialComponent()
{
    QFile file(QLatin1String(":/qml/PrincipledMaterialA.qml"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray data = file.readAll();

    MaterialParser::SceneData sceneData;
    MaterialParser::parseQmlData(data, file.fileName(), sceneData);

    QCOMPARE(sceneData.materials.size(), 1);
    QQuick3DPrincipledMaterial *mat = qobject_cast<QQuick3DPrincipledMaterial *>(sceneData.materials.at(0));
    QVERIFY(qFuzzyCompare(mat->metalness(), 0.1f));
    QVERIFY(qFuzzyCompare(mat->roughness(), 0.2f));
    QCOMPARE(mat->baseColor(), QColor("green"));
}

void Shadergen::tst_defaultMaterialComponent()
{
    QFile file(QLatin1String(":/qml/DefaultMaterialA.qml"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray data = file.readAll();

    MaterialParser::SceneData sceneData;
    MaterialParser::parseQmlData(data, file.fileName(), sceneData);

    QCOMPARE(sceneData.materials.size(), 1);
    QQuick3DDefaultMaterial *mat = qobject_cast<QQuick3DDefaultMaterial *>(sceneData.materials.at(0));
    QVERIFY(qFuzzyCompare(mat->specularAmount(), 0.1f));
    QVERIFY(qFuzzyCompare(mat->specularRoughness(), 0.2f));
    QCOMPARE(mat->diffuseColor(), QColor("pink"));
}

void Shadergen::tst_customMaterialComponent()
{
    QFile file(QLatin1String(":/qml/CustomMaterialA.qml"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray data = file.readAll();

    auto &sceneData = lastSceneData;
    MaterialParser::parseQmlData(data, file.fileName(), sceneData);

    QCOMPARE(sceneData.materials.size(), 1);
    QQuick3DCustomMaterial *mat = qobject_cast<QQuick3DCustomMaterial *>(sceneData.materials.at(0));
    QCOMPARE(mat->shadingMode(), QQuick3DCustomMaterial::ShadingMode::Unshaded);
}

void Shadergen::tst_customMaterialUniforms_data()
{
    QTest::addColumn<QMetaType>("type");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QVariant>("value");

    QTest::newRow("bool0") << QMetaType(QMetaType::Bool) << "uBoolFalse" << QVariant::fromValue(false);
    QTest::newRow("bool1") << QMetaType(QMetaType::Bool) << "uBoolTrue" << QVariant::fromValue(true);
    QTest::newRow("int") << QMetaType(QMetaType::Double) /*Not a typo*/ << "uInt" << QVariant::fromValue(int(33));
    QTest::newRow("real") << QMetaType(QMetaType::Double) << "uReal" << QVariant::fromValue(qreal(3.3));
    QTest::newRow("pointS") << QMetaType(QMetaType::QPointF) << "uPointS" << QVariant::fromValue(QPointF(0, 1));
    QTest::newRow("pointF") << QMetaType(QMetaType::QPointF) << "uPointF" << QVariant::fromValue(QPointF(1, 0));
    QTest::newRow("sizeS") << QMetaType(QMetaType::QSizeF) << "uSizeS" << QVariant::fromValue(QSizeF(1, 1));
    QTest::newRow("sizeF") << QMetaType(QMetaType::QSizeF) << "uSizeF" << QVariant::fromValue(QSizeF(2, 2));
    QTest::newRow("rectS") << QMetaType(QMetaType::QRectF) << "uRectS" << QVariant::fromValue(QRectF(0, 1, 100, 101));
    QTest::newRow("rectF") << QMetaType(QMetaType::QRectF) << "uRectF" << QVariant::fromValue(QRectF(1, 0, 101, 100));
    QTest::newRow("vec2") << QMetaType(QMetaType::QVector2D) << "uVec2" << QVariant::fromValue(QVector2D(1, 2));
    QTest::newRow("vec3") << QMetaType(QMetaType::QVector3D) << "uVec3" << QVariant::fromValue(QVector3D(1, 2, 3));
    QTest::newRow("vec4") << QMetaType(QMetaType::QVector4D) << "uVec4" << QVariant::fromValue(QVector4D(1, 2, 3, 4));
    QTest::newRow("quat") << QMetaType(QMetaType::QQuaternion) << "uQuat" << QVariant::fromValue(QQuaternion(1, 2, 3, 4));
    QTest::newRow("m44") << QMetaType(QMetaType::QMatrix4x4) << "uM44"
                         << QVariant::fromValue(QMatrix4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
    QTest::newRow("Color1") << QMetaType(QMetaType::QColor) << "uColor1" << QVariant::fromValue(QColor("green"));
    QTest::newRow("Color2") << QMetaType(QMetaType::QColor) << "uColor2" << QVariant::fromValue(QColor("#ff0000"));
    QTest::newRow("Texture") << QMetaType(qMetaTypeId<QQuick3DTexture *>()) << "uTex"
                             << QVariant::fromValue((QQuick3DTexture *)nullptr);
    QTest::newRow("TextureInput") << QMetaType(qMetaTypeId<QQuick3DShaderUtilsTextureInput *>()) << "uTexInput"
                                  << QVariant::fromValue((QQuick3DShaderUtilsTextureInput *)nullptr);
}

void Shadergen::tst_customMaterialUniforms()
{
    // Test that public QML members are added to the object
    const auto &sceneData = lastSceneData;
    QCOMPARE(sceneData.materials.size(), 1);
    QQuick3DCustomMaterial *mat = qobject_cast<QQuick3DCustomMaterial *>(sceneData.materials.at(0));
    QCOMPARE(mat->shadingMode(), QQuick3DCustomMaterial::ShadingMode::Unshaded);

    QFETCH(QMetaType, type);
    QFETCH(QString, name);
    QFETCH(QVariant, value);

    const auto prop = mat->property(name.toLatin1().constData());
    QVERIFY(prop.isValid());
    QCOMPARE(prop.metaType(), type);
    if (type == QMetaType(QMetaType::Double)) {
        QVERIFY(qFuzzyCompare(prop.toDouble(), value.toDouble()));
    } else if (type.id() >= QMetaType::User) {
        const bool ok = (prop.metaType().id() == qMetaTypeId<QQuick3DTexture *>()) ||
                (prop.metaType().id() == qMetaTypeId<QQuick3DShaderUtilsTextureInput *>());
        if (!ok) {
            qDebug() << "Texture: " << qMetaTypeId<QQuick3DTexture *>();
            qDebug() << "TextureInput: " << qMetaTypeId<QQuick3DShaderUtilsTextureInput *>();
            qDebug() << prop.metaType().id();
        }
        QVERIFY(ok);
    } else {
        QCOMPARE(prop, value);
    }
}

void Shadergen::tst_effectComponent()
{
    QFile file(QLatin1String(":/qml/EffectA.qml"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray data = file.readAll();

    MaterialParser::SceneData sceneData;
    MaterialParser::parseQmlData(data, file.fileName(), sceneData);

    QCOMPARE(sceneData.effects.size(), 1);
    QQuick3DEffect *effect = sceneData.effects.at(0);
    auto passes = effect->passes();
    QCOMPARE(passes.count(&passes), 1);
    auto pass = passes.at(&passes, 0);
    QVERIFY(pass);
    auto shaders = pass->shaders();
    {
        const auto count = shaders.count(&shaders);
        QCOMPARE(count, 2);
        // Vertex shader
        auto shader = shaders.at(&shaders, 0);
        // Shader url
        QVERIFY(!shader->shader.isEmpty());
        QCOMPARE(shader->stage, QQuick3DShaderUtilsShader::Stage::Vertex);

        // Fragment shader
        shader = shaders.at(&shaders, 1);
        // Shader url
        QVERIFY(!shader->shader.isEmpty());
        QCOMPARE(shader->stage, QQuick3DShaderUtilsShader::Stage::Fragment);
    }

    lastSceneData = std::move(sceneData);
}

void Shadergen::tst_effectUniforms_data()
{
    QTest::addColumn<QMetaType>("type");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QVariant>("value");

    QTest::newRow("bool0") << QMetaType(QMetaType::Bool) << "uBoolFalse" << QVariant::fromValue(false);
    QTest::newRow("bool1") << QMetaType(QMetaType::Bool) << "uBoolTrue" << QVariant::fromValue(true);
    QTest::newRow("int") << QMetaType(QMetaType::Double) /*Not a typo*/ << "uInt" << QVariant::fromValue(int(33));
    QTest::newRow("real") << QMetaType(QMetaType::Double) << "uReal" << QVariant::fromValue(qreal(3.3));
    QTest::newRow("pointS") << QMetaType(QMetaType::QPointF) << "uPointS" << QVariant::fromValue(QPointF(0, 1));
    QTest::newRow("pointF") << QMetaType(QMetaType::QPointF) << "uPointF" << QVariant::fromValue(QPointF(1, 0));
    QTest::newRow("sizeS") << QMetaType(QMetaType::QSizeF) << "uSizeS" << QVariant::fromValue(QSizeF(1, 1));
    QTest::newRow("sizeF") << QMetaType(QMetaType::QSizeF) << "uSizeF" << QVariant::fromValue(QSizeF(2, 2));
    QTest::newRow("rectS") << QMetaType(QMetaType::QRectF) << "uRectS" << QVariant::fromValue(QRectF(0, 1, 100, 101));
    QTest::newRow("rectF") << QMetaType(QMetaType::QRectF) << "uRectF" << QVariant::fromValue(QRectF(1, 0, 101, 100));
    QTest::newRow("vec2") << QMetaType(QMetaType::QVector2D) << "uVec2" << QVariant::fromValue(QVector2D(1, 2));
    QTest::newRow("vec3") << QMetaType(QMetaType::QVector3D) << "uVec3" << QVariant::fromValue(QVector3D(1, 2, 3));
    QTest::newRow("vec4") << QMetaType(QMetaType::QVector4D) << "uVec4" << QVariant::fromValue(QVector4D(1, 2, 3, 4));
    QTest::newRow("quat") << QMetaType(QMetaType::QQuaternion) << "uQuat" << QVariant::fromValue(QQuaternion(1, 2, 3, 4));
    QTest::newRow("m44") << QMetaType(QMetaType::QMatrix4x4) << "uM44"
                         << QVariant::fromValue(QMatrix4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
    QTest::newRow("Color1") << QMetaType(QMetaType::QColor) << "uColor1" << QVariant::fromValue(QColor("green"));
    QTest::newRow("Color2") << QMetaType(QMetaType::QColor) << "uColor2" << QVariant::fromValue(QColor("#ff0000"));
    QTest::newRow("TextureInput") << QMetaType(qMetaTypeId<QQuick3DShaderUtilsTextureInput *>()) << "uTexInput"
                                  << QVariant::fromValue((QQuick3DShaderUtilsTextureInput *)nullptr);
}

void Shadergen::tst_effectUniforms()
{
    // Test that public QML members are added to the object
    const auto &sceneData = lastSceneData;
    QCOMPARE(sceneData.effects.size(), 1);
    QQuick3DEffect *effect = qobject_cast<QQuick3DEffect *>(sceneData.effects.at(0));

    QFETCH(QMetaType, type);
    QFETCH(QString, name);
    QFETCH(QVariant, value);

    const auto prop = effect->property(name.toLatin1().constData());
    QVERIFY(prop.isValid());
    QCOMPARE(prop.metaType(), type);
    if (type == QMetaType(QMetaType::Double)) {
        QVERIFY(qFuzzyCompare(prop.toDouble(), value.toDouble()));
    } else if (type.id() >= QMetaType::User) {
        const bool ok = (prop.metaType().id() == qMetaTypeId<QQuick3DShaderUtilsTextureInput *>());
        if (!ok) {
            qDebug() << "TextureInput: " << qMetaTypeId<QQuick3DShaderUtilsTextureInput *>();
            qDebug() << prop.metaType().id();
        }
        QVERIFY(ok);
    } else {
        QCOMPARE(prop, value);
    }
}

void Shadergen::tst_componentResolving()
{
    QVector<QString> filePaths { ":/qml/main.qml", ":/qml/ModelA.qml", ":/qml/DefaultMaterialA.qml" };

    MaterialParser::SceneData sceneData;
    MaterialParser::parseQmlFiles(filePaths, QString(), sceneData, false);

    // main.qml(ModelA, Model, Model, ModelA, ModelA, ModelA, ModelA). Component should not be added as it's used
    const auto &models = sceneData.models;
    // The correct number should be 7 here, but we don't track if a component is
    // actually used so it gets pushed into the same list as the regular instances.
    QCOMPARE(models.size(), 8);

    // We skip testing the component here and start at index 1

    { // Model component (ModelA) with inherited material (1)
        const auto &model = models.at(1);
        auto materials = model->materials();
        QCOMPARE(materials.count(&materials), 1);
    }

    { // Model with refed component material (DefaultMaterialA) (1)
        const auto &model = models.at(2);
        auto materials = model->materials();
        QCOMPARE(materials.count(&materials), 1);
    }

    { // Model with inline component material (DefaultMaterialA) (1)
        const auto &model = models.at(3);
        auto materials = model->materials();
        QCOMPARE(materials.count(&materials), 1);
    }

    { // Model component (ModelA) overrides inherited material with refed component material (DefaultMaterialA) (1)
        const auto &model = models.at(4);
        auto materials = model->materials();
        QCOMPARE(materials.count(&materials), 1);
    }

    { // Model component (ModelA) overrides inherited material inline (DefaultMaterial) (1)
        const auto &model = models.at(5);
        auto materials = model->materials();
        QCOMPARE(materials.count(&materials), 1);
    }

    { // Model component (ModelA) override inherited material with refed materials (2)
        const auto &model = models.at(6);
        auto materials = model->materials();
        QCOMPARE(materials.count(&materials), 2);
    }

    { // Model component (ModelA) override inherited material with inline materials (2)
        const auto &model = models.at(7);
        auto materials = model->materials();
        QCOMPARE(materials.count(&materials), 2);
    }
}

void Shadergen::tst_instancing()
{
    QVector<QString> filePaths { ":/qml/instancing.qml" };

    MaterialParser::SceneData sceneData;
    MaterialParser::parseQmlFiles(filePaths, QString(), sceneData, false);

    // instancing.qml(Model).
    const auto &models = sceneData.models;
    QCOMPARE(models.size(), 1);

    {
        const auto &model = models.at(0);
        auto instancing = model->instancing();
        QVERIFY(instancing != nullptr);
        int count = -1;
        const auto buffer = instancing->instanceBuffer(&count);
        (void)buffer;
        QCOMPARE(count, 2);
    }
}

QTEST_APPLESS_MAIN(Shadergen)

#include "tst_shadergen.moc"
