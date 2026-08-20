// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "shaderpoker.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterialshadergenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhieffectsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicustommaterialsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgdebugdrawsystem_p.h>
#include <QtQuick3DRuntimeRender/ssg/qssgrendercontextcore.h>
#include <QtQuick3DRuntimeRender/private/qssgvertexpipelineimpl_p.h>

namespace {
QStringList splitIgnoringBraces(const QString &input) {
    QStringList result;
    QString currentPart;
    int braceDepth = 0;

    for (QChar c : input) {
        if (c == '{') {
            ++braceDepth;
            currentPart += c;
        } else if (c == '}') {
            --braceDepth;
            if (braceDepth < 0) {
                // because something is busted
                braceDepth = 0;
            }
            currentPart += c;
        } else if (c == ';' && braceDepth == 0) {
            result.append(currentPart);
            currentPart.clear();
        } else {
            currentPart += c;
        }
    }

    if (!currentPart.isEmpty()) {
        result.append(currentPart); // Add the last segment
    }

    return result;
}
}

ShaderPoker::ShaderPoker(QObject *parent)
    : QObject{parent}
{
    m_rhi = QRhi::create(QRhi::Null, nullptr);
    std::unique_ptr<QSSGRhiContext> rhiContext = std::make_unique<QSSGRhiContext>(m_rhi);

    m_renderContext = std::make_shared<QSSGRenderContextInterface>(std::make_unique<QSSGBufferManager>(),
                                                                    std::make_unique<QSSGRenderer>(),
                                                                    std::make_shared<QSSGShaderLibraryManager>(),
                                                                    std::make_unique<QSSGShaderCache>(*rhiContext),
                                                                    std::make_unique<QSSGCustomMaterialSystem>(),
                                                                    std::make_unique<QSSGProgramGenerator>(),
                                                                    std::move(rhiContext));

    m_material = new QSSGRenderDefaultMaterial(QSSGRenderGraphObject::Type::DefaultMaterial);

    connect(this, &ShaderPoker::shaderIndexChanged, this, &ShaderPoker::fragmentShaderChanged);
    connect(this, &ShaderPoker::shaderIndexChanged, this, &ShaderPoker::vertexShaderChanged);

    // points to builtin
    setShaderProperties(&m_builtinProperties);
    generateShader();
}

ShaderPoker::~ShaderPoker()
{
    delete m_material;
}

void ShaderPoker::generateShader()
{
    const auto &shaderLibraryManager = m_renderContext->shaderLibraryManager();
    const auto &shaderCache = m_renderContext->shaderCache();
    const auto &shaderProgramGenerator = m_renderContext->shaderProgramGenerator();
    const auto &renderer = m_renderContext->renderer();

    QSSGRenderModel renderModel;
    QSSGRenderSubset renderSubset;
    QSSGModelContext modelContext(renderModel, QMatrix4x4(), QMatrix3x3(), QSSGRenderMvpArray());
    QSSGShaderLightListView lights;
    QSSGRenderableImage *images = nullptr;

    QSSGShaderDefaultMaterialKey shaderKey;
    QSSGSubsetRenderable renderable(QSSGRenderableObject::Type::DefaultMaterialMeshSubset,
                                    QSSGRenderableObjectFlags(),
                                    QVector3D(),
                                    &*renderer,
                                    renderSubset,
                                    modelContext,
                                    1,
                                    0,
                                    *m_material,
                                    images,
                                    m_shaderProperties->key(),
                                    lights,
                                    false);

    QByteArray materialInfoString;
    m_currentShaderPipeline = QSSGRendererPrivate::generateRhiShaderPipelineImpl(renderable,
                                                                                 *shaderLibraryManager,
                                                                                 *shaderCache,
                                                                                 *shaderProgramGenerator,
                                                                                 m_shaderProperties->properties(),
                                                                                 m_featureSet,
                                                                                 { /* augmented shader */ },
                                                                                 materialInfoString);

    QSSGShaderCacheKey cacheKey(materialInfoString);
    cacheKey.m_features = m_featureSet;
    cacheKey.updateHashCode();
    const auto theIter = m_generatedShaderCodeCache.constFind(cacheKey);
    if (theIter != m_generatedShaderCodeCache.constEnd()) {
        auto &shaderCode = theIter.value();
        // Add it to the list
        m_vertexShader = shaderCode.first;
        m_fragmentShader = shaderCode.second;
    } else {
        // put it in the cache instead
        auto shaderCodePair = std::pair<QString, QString>(QString::fromLocal8Bit(shaderProgramGenerator->m_vs.m_finalBuilder.constData()),
                                                          QString::fromLocal8Bit(shaderProgramGenerator->m_fs.m_finalBuilder.constData()));
        m_generatedShaderCodeCache.insert(cacheKey, shaderCodePair);
        m_vertexShader = shaderCodePair.first;
        m_fragmentShader = shaderCodePair.second;
    }

    emit vertexShaderChanged();
    emit fragmentShaderChanged();


    //qDebug() << test->fragmentStage()->shader().availableShaders();

    m_shaderKey = QString::fromLocal8Bit(materialInfoString);
    emit shaderKeyChanged();
    m_shaderKeysList = splitIgnoringBraces(m_shaderKey);
    emit shaderKeysListChanged();

    updateAvailableShadersList();

    // // m_vertexShader = QString::fromLocal8Bit(shaderProgramGenerator->m_vs.m_finalBuilder.constData());
    // m_vertexShader = test->vertexStage()->shader().shader(test->vertexStage()->shader().availableShaders().last()).shader();
    // emit vertexShaderChanged();

    // // m_fragmentShader = QString::fromLocal8Bit(shaderProgramGenerator->m_fs.m_finalBuilder.constData());
    // m_fragmentShader = test->fragmentStage()->shader().shader(test->fragmentStage()->shader().availableShaders().last()).shader();
    // emit fragmentShaderChanged();
}

QString ShaderPoker::vertexShader() const
{
    if (m_shaderIndex == 0)
        return m_vertexShader;

    int index = m_shaderIndex - 1;
    if (index >= m_availableShadersList.size() || index < 0)
        return QString();


    return QString::fromLocal8Bit(m_currentShaderPipeline->vertexStage()->shader().shader(m_currentShaderPipeline->vertexStage()->shader().availableShaders().at(index)).shader());
}

QString ShaderPoker::fragmentShader() const
{
    if (m_shaderIndex == 0)
        return m_fragmentShader;

    int index = m_shaderIndex - 1;
    if (index >= m_availableShadersList.size() || index < 0)
        return QString();

    return QString::fromLocal8Bit(m_currentShaderPipeline->fragmentStage()->shader().shader(m_currentShaderPipeline->fragmentStage()->shader().availableShaders().at(index)).shader());
}

QString ShaderPoker::shaderKey() const
{
    return m_shaderKey;
}

QStringList ShaderPoker::shaderKeysList() const
{
    return m_shaderKeysList;
}

ShaderPoker::MaterialType ShaderPoker::materialType() const
{
    return m_materialType;
}

void ShaderPoker::setMaterialType(const MaterialType &newMaterialType)
{
    if (m_materialType == newMaterialType)
        return;
    m_materialType = newMaterialType;

    delete m_material;
    // This doesn't really matter in practice
    if (m_materialType == MaterialType::Default)
        m_material = new QSSGRenderDefaultMaterial(QSSGRenderGraphObject::Type::DefaultMaterial);
    else if (m_materialType == MaterialType::Principled)
        m_material = new QSSGRenderDefaultMaterial(QSSGRenderGraphObject::Type::PrincipledMaterial);
    else if (m_materialType == MaterialType::SpecularGlossy)
        m_material = new QSSGRenderDefaultMaterial(QSSGRenderGraphObject::Type::SpecularGlossyMaterial);


    emit materialTypeChanged();

    generateShader();
}

quint32 ShaderPoker::featureBitfield() const
{
    return m_featureBitfield;
}

void ShaderPoker::setFeatureBitfield(quint32 newFeatureBitfield)
{
    if (m_featureBitfield == newFeatureBitfield)
        return;
    m_featureBitfield = newFeatureBitfield;
    emit featureBitfieldChanged();

    m_featureSet.flags = newFeatureBitfield;
    generateShader();
}

QStringList ShaderPoker::availableShadersList() const
{
    return m_availableShadersList;
}

void ShaderPoker::updateAvailableShadersList()
{
    m_availableShadersList.clear();
    m_availableShadersList.append("Original");

    if (m_currentShaderPipeline) {
        const auto &availableShaders = m_currentShaderPipeline->fragmentStage()->shader().availableShaders();
        for (const QShaderKey &shaderKey : availableShaders) {
            QString source;
            QString version;

            switch (shaderKey.source()) {
            case QShader::SpirvShader:
                source = "SPIR-V";
                break;
            case QShader::GlslShader:
                source = "GLSL";
                break;
            case QShader::HlslShader:
                source = "HLSL";
                break;
            case QShader::DxbcShader:
                source = "DXBC";
                break;
            case QShader::MslShader:
                source = "MSL";
                break;
            case QShader::DxilShader:
                source = "DXIL";
                break;
            case QShader::MetalLibShader:
                source = "MetalLib";
                break;
            case QShader::WgslShader:
                source = "WGSL";
                break;
            }
            version = QString::number(shaderKey.sourceVersion().version());

            m_availableShadersList.append(source + " Version: " + version);
        }
    }

    emit availableShadersListChanged();
}

int ShaderPoker::shaderIndex() const
{
    return m_shaderIndex;
}

void ShaderPoker::setShaderIndex(int newShaderIndex)
{
    if (m_shaderIndex == newShaderIndex)
        return;
    m_shaderIndex = newShaderIndex;
    emit shaderIndexChanged();
}

DefaultMaterialShaderProperties *ShaderPoker::shaderProperties() const
{
    return m_shaderProperties;
}

void ShaderPoker::setShaderProperties(DefaultMaterialShaderProperties *newShaderProperties)
{
    if (m_shaderProperties == newShaderProperties)
        return;

    if (m_shaderPropertiesDirtyConnection)
        disconnect(m_shaderPropertiesDirtyConnection);

    if (newShaderProperties == nullptr)
        m_shaderProperties = &m_builtinProperties;
    else
        m_shaderProperties = newShaderProperties;

    m_shaderPropertiesDirtyConnection = connect(m_shaderProperties, &DefaultMaterialShaderProperties::shaderKeyUpdated, this, &ShaderPoker::shaderPropertiesDirty);

    emit shaderPropertiesChanged();
    generateShader();
}

void ShaderPoker::shaderPropertiesDirty()
{
    generateShader();
}
