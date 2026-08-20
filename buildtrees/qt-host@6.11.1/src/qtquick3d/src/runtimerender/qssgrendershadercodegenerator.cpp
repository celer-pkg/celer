// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrendershadercodegenerator_p.h"

#include <QtQuick3DUtils/private/qssgutils_p.h>

#include "qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgshaderresourcemergecontext_p.h>

QT_BEGIN_NAMESPACE

template<typename T>
static inline void addStartCond(QByteArray &block, const T &var)
{
    // must use #if not #ifdef, as we test for the value, because featureset flags
    // are written out even when 0, think for example #define QSSG_ENABLE_SSM 0
    if (var.conditionType == QSSGRenderShaderMetadata::Uniform::Regular)
        block += QString::asprintf("#if %s\n", var.conditionName.constData()).toUtf8();
    else if (var.conditionType == QSSGRenderShaderMetadata::Uniform::Negated)
        block += QString::asprintf("#if !%s\n", var.conditionName.constData()).toUtf8();
}

template<typename T>
static inline void addEndCond(QByteArray &block, const T &var)
{
    if (var.conditionType != QSSGRenderShaderMetadata::Uniform::None)
        block += QByteArrayLiteral("#endif\n");
}

int QSSGShaderResourceMergeContext::s_additionalBuffers = 0;

struct QSSGShaderGeneratedProgramOutput
{
    // never null; so safe to call strlen on.
    const char *m_vertexShader{ "" };
    const char *m_fragmentShader{ "" };

    QSSGShaderGeneratedProgramOutput() = default;
    QSSGShaderGeneratedProgramOutput(const char *vs, const char *fs)
        : m_vertexShader(vs), m_fragmentShader(fs)
    {
    }
};

QSSGStageGeneratorBase::QSSGStageGeneratorBase(QSSGShaderGeneratorStage inStage)

    : m_stage(inStage)
{
}

void QSSGStageGeneratorBase::begin(QSSGShaderGeneratorStageFlags inEnabledStages)
{
    m_incoming.clear();
    m_outgoing = nullptr;
    m_flatIncoming.clear();
    m_flatOutgoing = nullptr;
    m_includes.clear();
    m_uniforms.clear();
    m_uniformArrays.clear();
    m_constantBuffers.clear();
    m_constantBufferParams.clear();
    m_codeBuilder.clear();
    m_finalBuilder.clear();
    m_enabledStages = inEnabledStages;
    m_addedFunctions.clear();
    m_addedDefinitions.clear();
    m_addedTypeDeclarations.clear();
    m_prefixes.clear();
    // the shared buffers will be cleared elsewhere.
}

void QSSGStageGeneratorBase::addIncoming(const QByteArray &name, const QByteArray &type)
{
    m_incoming.insert(name, type);
    m_flatIncoming.remove(name);
}

void QSSGStageGeneratorBase::addOutgoing(const QByteArray &name, const QByteArray &type)
{
    if (m_outgoing == nullptr) {
        Q_ASSERT(false);
        return;
    }
    m_outgoing->insert(name, type);
    if (m_flatOutgoing)
        m_flatOutgoing->remove(name);
}

void QSSGStageGeneratorBase::addFlatIncoming(const QByteArray &name, const QByteArray &type)
{
    m_flatIncoming.insert(name, type);
    m_incoming.remove(name);
}

void QSSGStageGeneratorBase::addFlatOutgoing(const QByteArray &name, const QByteArray &type)
{
    if (m_flatOutgoing == nullptr) {
        Q_ASSERT(false);
        return;
    }
    m_flatOutgoing->insert(name, type);
    if (m_outgoing)
        m_outgoing->remove(name);
}

void QSSGStageGeneratorBase::addUniform(const QByteArray &name, const QByteArray &type)
{
    m_uniforms.insert(name, type);
}

void QSSGStageGeneratorBase::addUniformArray(const QByteArray &name, const QByteArray &type, quint32 size)
{
    m_uniformArrays.insert(name, qMakePair(size, type));
}

void QSSGStageGeneratorBase::addConstantBuffer(const QByteArray &name, const QByteArray &layout)
{
    m_constantBuffers.insert(name, layout);
}

void QSSGStageGeneratorBase::addConstantBufferParam(const QByteArray &cbName, const QByteArray &paramName, const QByteArray &type)
{
    TParamPair theParamPair(paramName, type);
    TConstantBufferParamPair theBufferParamPair(cbName, theParamPair);
    m_constantBufferParams.push_back(theBufferParamPair);
}

QSSGStageGeneratorBase &QSSGStageGeneratorBase::operator<<(const QByteArray &data)
{
    m_codeBuilder.append(data);
    return *this;
}

void QSSGStageGeneratorBase::append(const QByteArray &data)
{
    m_codeBuilder.append(data);
    m_codeBuilder.append("\n");
}

QSSGShaderGeneratorStage QSSGStageGeneratorBase::stage() const { return m_stage; }

void QSSGStageGeneratorBase::addShaderPass2Marker(QSSGStageGeneratorBase::ShaderItemType itemType)
{
    Q_ASSERT(m_mergeContext);
    m_finalBuilder.append(QByteArrayLiteral("//@@") + QByteArray::number(int(itemType)) + QByteArrayLiteral("\n"));
}

void QSSGStageGeneratorBase::addShaderItemMap(QSSGStageGeneratorBase::ShaderItemType itemType, const TStrTableStrMap &itemMap, ShaderItemMapFlags flags)
{
    m_finalBuilder.append("\n");

    Q_ASSERT(m_mergeContext);
    for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end(); iter != end; ++iter) {
        const QByteArray name = iter.key();
        switch (itemType) {
        case ShaderItemType::VertexInput:
            m_mergeContext->registerInput(QSSGShaderGeneratorStage::Vertex, iter.value(), name);
            break;
        case ShaderItemType::Input:
            m_mergeContext->registerInput(m_stage, iter.value(), name, flags.testFlag(ShaderItemMapFlag::Flat));
            break;
        case ShaderItemType::Output:
            m_mergeContext->registerOutput(m_stage, iter.value(), name, flags.testFlag(ShaderItemMapFlag::Flat));
            break;
        case ShaderItemType::Uniform:
            if (iter.value().startsWith(QByteArrayLiteral("sampler")))
                m_mergeContext->registerSampler(iter.value(), name);
            else
                m_mergeContext->registerUniformMember(iter.value(), name);
            break;
        default:
            qWarning("Unknown shader item %d", int(itemType));
            Q_UNREACHABLE();
        }
    }
}

void QSSGStageGeneratorBase::addShaderIncomingMap()
{
    addShaderItemMap(ShaderItemType::VertexInput, m_incoming);
    addShaderPass2Marker(ShaderItemType::VertexInput);
}

void QSSGStageGeneratorBase::addShaderUniformMap()
{
    addShaderItemMap(ShaderItemType::Uniform, m_uniforms);
    for (TStrTableSizedStrMap::const_iterator iter = m_uniformArrays.begin(), end = m_uniformArrays.end(); iter != end; ++iter) {
        const QByteArray name = iter.key() +
                                "[" + QByteArray::number(iter.value().first) + "]";
        if (iter.value().second.startsWith(QByteArrayLiteral("sampler")))
            m_mergeContext->registerSampler(iter.value().second, name);
        else
            m_mergeContext->registerUniformMember(iter.value().second, name);
    }
    addShaderPass2Marker(ShaderItemType::Uniform);
    addShaderPass2Marker(ShaderItemType::Image);
}

void QSSGStageGeneratorBase::addShaderOutgoingMap()
{
    if (m_outgoing)
        addShaderItemMap(ShaderItemType::Output, *m_outgoing);
    if (m_flatOutgoing)
        addShaderItemMap(ShaderItemType::Output, *m_flatOutgoing, ShaderItemMapFlag::Flat);

    addShaderPass2Marker(ShaderItemType::Output);
}

void QSSGStageGeneratorBase::addShaderConstantBufferItemMap(const QByteArray &itemType, const TStrTableStrMap &cbMap, TConstantBufferParamArray cbParamsArray)
{
    m_finalBuilder.append("\n");

    // iterate over all constant buffers
    for (TStrTableStrMap::const_iterator iter = cbMap.begin(), end = cbMap.end(); iter != end; ++iter) {
        m_finalBuilder.append(iter.value());
        m_finalBuilder.append(" ");
        m_finalBuilder.append(itemType);
        m_finalBuilder.append(" ");
        m_finalBuilder.append(iter.key());
        m_finalBuilder.append(" {\n");
        // iterate over all param entries and add match
        for (TConstantBufferParamArray::const_iterator iter1 = cbParamsArray.begin(), end = cbParamsArray.end(); iter1 != end;
             ++iter1) {
            if (iter1->first == iter.key()) {
                m_finalBuilder.append(iter1->second.second);
                m_finalBuilder.append(" ");
                m_finalBuilder.append(iter1->second.first);
                m_finalBuilder.append(";\n");
            }
        }

        m_finalBuilder.append("};\n");
    }
}

void QSSGStageGeneratorBase::appendShaderCode() { m_finalBuilder.append(m_codeBuilder); }

void QSSGStageGeneratorBase::addInclude(const QByteArray &name) { m_includes.insert(name); }

void QSSGStageGeneratorBase::buildShaderSourcePass1(QSSGShaderResourceMergeContext *mergeContext)
{
    m_mergeContext = mergeContext;

    if (m_prefixes.size())
        m_finalBuilder.append("\n");
    for (const auto &prefix : std::as_const(m_prefixes)) {
        m_finalBuilder.append(prefix);
        m_finalBuilder.append("\n");
    }

    addShaderIncomingMap();
    addShaderUniformMap();
    addShaderConstantBufferItemMap("uniform", m_constantBuffers, m_constantBufferParams);
    addShaderOutgoingMap();
    m_mergeContext = nullptr;

    for (auto iter = m_addedDefinitions.begin(), end = m_addedDefinitions.end();
            iter != end; ++iter) {
        m_finalBuilder.append("#ifndef ");
        m_finalBuilder.append(iter.key());
        m_finalBuilder.append("\n");
        m_finalBuilder.append("#define ");
        m_finalBuilder.append(iter.key());
        if (!iter.value().isEmpty())
            m_finalBuilder.append(QByteArrayLiteral(" ") + iter.value());
        m_finalBuilder.append("\n#endif\n");
    }

    for (const auto& value : std::as_const(m_addedTypeDeclarations)) {
        m_finalBuilder.append(value);
        m_finalBuilder.append("\n");
    }

    // Sort for deterministic shader text when printing/debugging
    QList<QByteArray> sortedIncludes(m_includes.begin(), m_includes.end());
    std::sort(sortedIncludes.begin(), sortedIncludes.end());

    for (const auto &include : sortedIncludes) {
        m_finalBuilder.append("#include \"");
        m_finalBuilder.append(include);
        m_finalBuilder.append("\"\n");
    }

    appendShaderCode();
}

QByteArray QSSGStageGeneratorBase::buildShaderSourcePass2(QSSGShaderResourceMergeContext *mergeContext)
{
    static const char *prefix = "//@@";
    const int prefixLen = 4;
    const int typeLen = 1;
    int from = 0;

    mergeContext->rearrangeResources();

    for (; ;) {
        int pos = m_finalBuilder.indexOf(prefix, from);
        if (pos >= 0) {
            from = pos;
            ShaderItemType itemType = ShaderItemType(m_finalBuilder.mid(pos + prefixLen, typeLen).toInt());
            switch (itemType) {
            case ShaderItemType::VertexInput:
                if (m_stage == QSSGShaderGeneratorStage::Vertex) {
                    QByteArray block;
                    for (const QSSGShaderResourceMergeContext::InOutVar &var : std::as_const(mergeContext->m_inOutVars)) {
                        if (var.stagesInputIn.testFlag(m_stage))
                            block += QString::asprintf("layout(location = %d) in %s %s;\n", var.location, var.type.constData(), var.name.constData()).toUtf8();
                    }
                    m_finalBuilder.replace(pos, prefixLen + typeLen, block);
                }
                break;
            case ShaderItemType::Input:
            {
                QByteArray block;
                for (const QSSGShaderResourceMergeContext::InOutVar &var : std::as_const(mergeContext->m_inOutVars)) {
                    if (var.stagesInputIn.testFlag(m_stage))
                        block += QString::asprintf("layout(location = %d) in %s%s %s;\n", var.location, var.flat ? "flat " : "", var.type.constData(), var.name.constData()).toUtf8();
                }
                m_finalBuilder.replace(pos, prefixLen + typeLen, block);
            }
                break;
            case ShaderItemType::Output:
            {
                QByteArray block;
                for (const QSSGShaderResourceMergeContext::InOutVar &var : std::as_const(mergeContext->m_inOutVars)) {
                    if (var.stageOutputFrom.testFlag(m_stage))
                        block += QString::asprintf("layout(location = %d) out %s%s %s;\n", var.location, var.flat ? "flat " : "", var.type.constData(), var.name.constData()).toUtf8();
                }
                m_finalBuilder.replace(pos, prefixLen + typeLen, block);
            }
                break;
            case ShaderItemType::Image:
            {
                QByteArray block;

                for (const auto &image : std::as_const(mergeContext->m_images)) {
                    addStartCond(block, image);
                    block += QString::asprintf("layout(binding = %d, %s) uniform %s %s %s;\n",
                                               image.binding,
                                               image.imgType.constData(),
                                               image.qualifiers.constData(),
                                               image.type.constData(),
                                               image.name.constData()).toUtf8();
                    addEndCond(block, image);
                }
                m_finalBuilder.replace(pos, prefixLen + typeLen, block);
            } break;
            case ShaderItemType::Uniform:
            {
                QByteArray block;

                for (const auto &sampler : std::as_const(mergeContext->m_samplers)) {
                    addStartCond(block, sampler);
                    block += QString::asprintf("layout(binding = %d) uniform %s %s;\n",
                                               sampler.binding,
                                               sampler.type.constData(),
                                               sampler.name.constData()).toUtf8();
                    addEndCond(block, sampler);
                }

                if (!mergeContext->m_uniformMembers.isEmpty()) {
                    // The layout (offsets of the members) of the main
                    // uniform block cannot be different in the stages.
                    // (f.ex., a given member must be assumed to be at same
                    // offset both in the vertex and the fragment shader)
                    // Therefore we output everything in all stages.
                    block += QByteArrayLiteral("layout(std140, binding = 0) uniform cbMain {\n");
                    for (auto iter = mergeContext->m_uniformMembers.cbegin(), end = mergeContext->m_uniformMembers.cend();
                         iter != end; ++iter)
                    {
                        addStartCond(block, iter.value());
                        block += QString::asprintf("  %s %s;\n", iter.value().type.constData(), iter.value().name.constData()).toUtf8();
                        addEndCond(block, iter.value());
                    }
                    // No instance name for this uniform block. This is
                    // essential since custom material shader code will not use
                    // any instance name prefix when accessing the members. So
                    // while the internal stuff for default/principled material
                    // could be fixed up with prefixing everything, custom
                    // materials cannot. So leave it out.
                    block += QByteArrayLiteral("};\n");
                }
                m_finalBuilder.replace(pos, prefixLen + typeLen, block);
            }
                break;
            default:
                Q_UNREACHABLE_RETURN(m_finalBuilder);
            }
        } else {
            break;
        }
    }

    return m_finalBuilder;
}

void QSSGStageGeneratorBase::addFunction(const QByteArray &functionName)
{
    if (!m_addedFunctions.contains(functionName)) {
        m_addedFunctions.push_back(functionName);
        QByteArray includeName;
        includeName = "func" + functionName + ".glsllib";
        addInclude(includeName);
    }
}

void QSSGStageGeneratorBase::addPrefix(const QByteArray &data)
{
    m_prefixes.append(data);
}

void QSSGStageGeneratorBase::addDefinition(const QByteArray &name, const QByteArray &value)
{
    m_addedDefinitions.insert(name, value);
}

void QSSGStageGeneratorBase::addTypeDeclaration(const QByteArray &typeName, const QByteArray &snippet)
{
    if (snippet.isEmpty())
        return;
    m_addedTypeDeclarations.insert(typeName, snippet);
}

void QSSGProgramGenerator::linkStages()
{
    // Link stages incoming to outgoing variables.
    QSSGStageGeneratorBase *previous = nullptr;
    quint32 theStageId = 1;
    for (quint32 idx = 0, end = quint32(QSSGShaderGeneratorStage::StageCount); idx < end; ++idx, theStageId = theStageId << 1) {
        QSSGStageGeneratorBase *thisStage = nullptr;
        QSSGShaderGeneratorStage theStageEnum = static_cast<QSSGShaderGeneratorStage>(theStageId);
        if ((m_enabledStages & theStageEnum)) {
            thisStage = &internalGetStage(theStageEnum);
            if (previous) {
                previous->m_outgoing = &thisStage->m_incoming;
                previous->m_flatOutgoing = &thisStage->m_flatIncoming;
            }
            previous = thisStage;
        }
    }
}

void QSSGProgramGenerator::beginProgram(QSSGShaderGeneratorStageFlags inEnabledStages)
{
    m_vs.begin(inEnabledStages);
    m_fs.begin(inEnabledStages);
    m_enabledStages = inEnabledStages;
    linkStages();
}

QSSGShaderGeneratorStageFlags QSSGProgramGenerator::getEnabledStages() const { return m_enabledStages; }

QSSGStageGeneratorBase &QSSGProgramGenerator::internalGetStage(QSSGShaderGeneratorStage inStage)
{
    switch (inStage) {
    case QSSGShaderGeneratorStage::Vertex:
        return m_vs;
    case QSSGShaderGeneratorStage::Fragment:
        return m_fs;
    default:
        Q_ASSERT(false);
        break;
    }
    return m_vs;
}

QSSGStageGeneratorBase *QSSGProgramGenerator::getStage(QSSGShaderGeneratorStage inStage)
{
    if ((m_enabledStages & inStage))
        return &internalGetStage(inStage);
    return nullptr;
}

void QSSGProgramGenerator::registerShaderMetaDataFromSource(QSSGShaderResourceMergeContext *mergeContext, const QByteArray &contents, QSSGShaderGeneratorStage stage)
{
    QSSGRenderShaderMetadata::ShaderMetaData meta = QSSGRenderShaderMetadata::getShaderMetaData(contents);

    for (const QSSGRenderShaderMetadata::Uniform &u : std::as_const(meta.uniforms)) {
        if (u.type.startsWith(QByteArrayLiteral("sampler"))) {
            if (u.multiview && mergeContext->viewCount >= 2) {
                // 'sampler2D qt_screenTexture' becomes 'sampler2DArray qt_screenTextureArray'
                mergeContext->registerSampler(u.type + QByteArrayLiteral("Array"), u.name + QByteArrayLiteral("Array"), u.condition, u.conditionName);
            } else {
                mergeContext->registerSampler(u.type, u.name, u.condition, u.conditionName);
            }
        } else {
            if (u.multiview && mergeContext->viewCount >= 2) {
                const QByteArray name = u.name + "[" + QByteArray::number(mergeContext->viewCount) + "]";
                mergeContext->registerUniformMember(u.type, name, u.condition, u.conditionName);
            } else {
                mergeContext->registerUniformMember(u.type, u.name, u.condition, u.conditionName);
            }
        }
    }

    for (const QSSGRenderShaderMetadata::Image &img : std::as_const(meta.images))
        mergeContext->registerImage(img.type, img.name, img.imageType, img.qualifiers, img.condition, img.conditionName);

    for (const QSSGRenderShaderMetadata::InputOutput &inputVar : std::as_const(meta.inputs)) {
        if (inputVar.stage == stage)
            mergeContext->registerInput(stage, inputVar.type, inputVar.name, inputVar.flat);
    }

    for (const QSSGRenderShaderMetadata::InputOutput &outputVar : std::as_const(meta.outputs)) {
        if (outputVar.stage == stage)
            mergeContext->registerOutput(stage, outputVar.type, outputVar.name, outputVar.flat);
    }

    for (auto it = mergeContext->m_inOutVars.cbegin(), end = mergeContext->m_inOutVars.cend(); it != end; ++it) {
        if (it->stagesInputIn == int(QSSGShaderGeneratorStage::Fragment) && it->stageOutputFrom == 0)
            qWarning("Fragment stage input %s is not output from vertex stage; expect errors.", it.key().constData());
    }
}

QSSGRhiShaderPipelinePtr QSSGProgramGenerator::compileGeneratedRhiShader(const QByteArray &inMaterialInfoString,
                                                                         const QSSGShaderFeatures &inFeatureSet,
                                                                         QSSGShaderLibraryManager &shaderLibraryManager,
                                                                         QSSGShaderCache &theCache,
                                                                         QSSGRhiShaderPipeline::StageFlags stageFlags,
                                                                         const QSSGUserShaderAugmentation &shaderAugmentation,
                                                                         int viewCount,
                                                                         bool perTargetCompilation)
{
    // No stages enabled
    if (((quint32)m_enabledStages) == 0) {
        Q_ASSERT(false);
        return nullptr;
    }

    QSSGShaderResourceMergeContext mergeContext;
    mergeContext.viewCount = viewCount;

    for (quint32 stageIdx = 0; stageIdx < static_cast<quint32>(QSSGShaderGeneratorStage::StageCount); ++stageIdx) {
        QSSGShaderGeneratorStage stageName = static_cast<QSSGShaderGeneratorStage>(1 << stageIdx);
        if (m_enabledStages & stageName) {
            QSSGStageGeneratorBase &theStage(internalGetStage(stageName));
            theStage.buildShaderSourcePass1(&mergeContext);
        }
    }

    for (quint32 stageIdx = 0; stageIdx < static_cast<quint32>(QSSGShaderGeneratorStage::StageCount); ++stageIdx) {
        QSSGShaderGeneratorStage stageName = static_cast<QSSGShaderGeneratorStage>(1 << stageIdx);
        if (m_enabledStages & stageName) {
            QSSGStageGeneratorBase &theStage(internalGetStage(stageName));
            shaderLibraryManager.resolveIncludeFiles(theStage.m_finalBuilder, inMaterialInfoString);
            registerShaderMetaDataFromSource(&mergeContext, theStage.m_finalBuilder, stageName);
        }
    }

    for (quint32 stageIdx = 0; stageIdx < static_cast<quint32>(QSSGShaderGeneratorStage::StageCount); ++stageIdx) {
        QSSGShaderGeneratorStage stageName = static_cast<QSSGShaderGeneratorStage>(1 << stageIdx);
        if (m_enabledStages & stageName) {
            QSSGStageGeneratorBase &theStage(internalGetStage(stageName));
            theStage.buildShaderSourcePass2(&mergeContext);
        }
    }

    QSSGShaderResourceMergeContext::setAdditionalBufferAmount(0);

    // qDebug("VERTEX:\n%s\n\n", m_vs.m_finalBuilder.constData());
    // qDebug("FRAGMENT:\n%s\n\n", m_fs.m_finalBuilder.constData());

    return theCache.compileForRhi(inMaterialInfoString,
                                   m_vs.m_finalBuilder,
                                   m_fs.m_finalBuilder,
                                   inFeatureSet,
                                   stageFlags,
                                   shaderAugmentation,
                                   viewCount,
                                   perTargetCompilation);
}

QSSGVertexShaderGenerator::QSSGVertexShaderGenerator()
    : QSSGStageGeneratorBase(QSSGShaderGeneratorStage::Vertex)
{}

QSSGFragmentShaderGenerator::QSSGFragmentShaderGenerator()
    : QSSGStageGeneratorBase(QSSGShaderGeneratorStage::Fragment)
{}

void QSSGFragmentShaderGenerator::addShaderIncomingMap()
{
    addShaderItemMap(ShaderItemType::Input, m_incoming);
    addShaderItemMap(ShaderItemType::Input, m_flatIncoming, ShaderItemMapFlag::Flat);
    addShaderPass2Marker(ShaderItemType::Input);
}

void QSSGFragmentShaderGenerator::addShaderOutgoingMap()
{
    addShaderPass2Marker(ShaderItemType::Output);
}

QT_END_NAMESPACE
