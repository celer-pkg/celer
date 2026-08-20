// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_SHADER_CODE_GENERATOR_V2_H
#define QSSG_RENDER_SHADER_CODE_GENERATOR_V2_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

enum class QSSGShaderGeneratorStage
{
    None = 0,
    Vertex = 1,
    Fragment = 1 << 1,
    StageCount = 2,
};

Q_DECLARE_FLAGS(QSSGShaderGeneratorStageFlags, QSSGShaderGeneratorStage)
Q_DECLARE_OPERATORS_FOR_FLAGS(QSSGShaderGeneratorStageFlags)

struct QSSGStageGeneratorBase;
class QSSGRenderContextInterface;
class QSSGShaderLibraryManager;

class QSSGShaderResourceMergeContext;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGStageGeneratorBase
{
    enum class ShaderItemType {
        VertexInput,
        Input,
        Output,
        Uniform,
        Image
    };

    enum class ShaderItemMapFlag {
        Flat = 0x01
    };
    Q_DECLARE_FLAGS(ShaderItemMapFlags, ShaderItemMapFlag)

    // Using QMap intentionally - being sorted by key when iterating is helpful
    // to get the same ordered list of vertex inputs, uniforms, etc. on every
    // run, which in turn helps shader (disk) cache efficiency due to not
    // generating a different shader string just because QHash decided to
    // iterate entries in a different order.
    typedef QMap<QByteArray, QByteArray> TStrTableStrMap;
    typedef QMap<QByteArray, QPair<quint32, QByteArray>> TStrTableSizedStrMap;

    typedef QPair<QByteArray, QByteArray> TParamPair;
    typedef QPair<QByteArray, TParamPair> TConstantBufferParamPair;
    typedef QVector<TConstantBufferParamPair> TConstantBufferParamArray;

    TStrTableStrMap m_incoming;
    TStrTableStrMap *m_outgoing = nullptr;
    TStrTableStrMap m_flatIncoming;
    TStrTableStrMap *m_flatOutgoing = nullptr;
    QSet<QByteArray> m_includes;
    QList<QByteArray> m_prefixes;
    TStrTableStrMap m_uniforms;
    TStrTableSizedStrMap m_uniformArrays;
    TStrTableStrMap m_constantBuffers;
    TConstantBufferParamArray m_constantBufferParams;
    QByteArray m_codeBuilder;
    QByteArray m_finalBuilder;
    QSSGShaderGeneratorStage m_stage;
    QSSGShaderGeneratorStageFlags m_enabledStages;
    QList<QByteArray> m_addedFunctions;
    TStrTableStrMap m_addedTypeDeclarations;
    TStrTableStrMap m_addedDefinitions;
    QSSGShaderResourceMergeContext *m_mergeContext = nullptr;

    explicit QSSGStageGeneratorBase(QSSGShaderGeneratorStage inStage);
    virtual ~QSSGStageGeneratorBase() = default;

    virtual void begin(QSSGShaderGeneratorStageFlags inEnabledStages);

    virtual void addIncoming(const QByteArray &name, const QByteArray &type);
    virtual void addOutgoing(const QByteArray &name, const QByteArray &type);

    virtual void addFlatIncoming(const QByteArray &name, const QByteArray &type);
    virtual void addFlatOutgoing(const QByteArray &name, const QByteArray &type);

    virtual void addUniform(const QByteArray &name, const QByteArray &type);

    virtual void addUniformArray(const QByteArray &name, const QByteArray &type, quint32 size);

    virtual void addConstantBuffer(const QByteArray &name, const QByteArray &layout);
    virtual void addConstantBufferParam(const QByteArray &cbName, const QByteArray &paramName, const QByteArray &type);

    virtual QSSGStageGeneratorBase &operator<<(const QByteArray &data);
    virtual void append(const QByteArray &data);
    QSSGShaderGeneratorStage stage() const;

    void addShaderPass2Marker(ShaderItemType itemType);

    void addShaderItemMap(ShaderItemType itemType, const TStrTableStrMap &itemMap, ShaderItemMapFlags flags = {});

    virtual void addShaderIncomingMap();

    virtual void addShaderUniformMap();

    virtual void addShaderOutgoingMap();

    virtual void addShaderConstantBufferItemMap(const QByteArray &itemType, const TStrTableStrMap &cbMap, TConstantBufferParamArray cbParamsArray);

    virtual void appendShaderCode() final;

    virtual void addInclude(const QByteArray &name) final;

    void buildShaderSourcePass1(QSSGShaderResourceMergeContext *mergeContext);

    QByteArray buildShaderSourcePass2(QSSGShaderResourceMergeContext *mergeContext);

    virtual void addFunction(const QByteArray &functionName) final;

    virtual void addDefinition(const QByteArray &name, const QByteArray &value) final;

    virtual void addTypeDeclaration(const QByteArray &typeName, const QByteArray &snippet) final;

    void addPrefix(const QByteArray &data);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSSGStageGeneratorBase::ShaderItemMapFlags)

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGVertexShaderGenerator final : public QSSGStageGeneratorBase
{
    QSSGVertexShaderGenerator();
};

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGFragmentShaderGenerator final : public QSSGStageGeneratorBase
{
    QSSGFragmentShaderGenerator();
    void addShaderIncomingMap() override;
    void addShaderOutgoingMap() override;
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGProgramGenerator
{
    Q_DISABLE_COPY(QSSGProgramGenerator)
public:
    QSSGProgramGenerator() = default;
    QSSGVertexShaderGenerator m_vs;
    QSSGFragmentShaderGenerator m_fs;

    QSSGShaderGeneratorStageFlags m_enabledStages;

    static constexpr QSSGShaderGeneratorStageFlags defaultFlags() { return QSSGShaderGeneratorStageFlags(QSSGShaderGeneratorStage::Vertex | QSSGShaderGeneratorStage::Fragment); }

    void linkStages();

    void beginProgram(QSSGShaderGeneratorStageFlags inEnabledStages = defaultFlags());

    QSSGShaderGeneratorStageFlags getEnabledStages() const;

    QSSGStageGeneratorBase &internalGetStage(QSSGShaderGeneratorStage inStage);
    // get the stage or nullptr if it has not been created.
    QSSGStageGeneratorBase *getStage(QSSGShaderGeneratorStage inStage);

    void registerShaderMetaDataFromSource(QSSGShaderResourceMergeContext *mergeContext,
                                          const QByteArray &contents,
                                          QSSGShaderGeneratorStage stage);

    QSSGRhiShaderPipelinePtr compileGeneratedRhiShader(const QByteArray &inMaterialInfoString,
                                                       const QSSGShaderFeatures &inFeatureSet,
                                                       QSSGShaderLibraryManager &shaderLibraryManager,
                                                       QSSGShaderCache &theCache,
                                                       QSSGRhiShaderPipeline::StageFlags stageFlags,
                                                       const QSSGUserShaderAugmentation &shaderAugmentation,
                                                       int viewCount,
                                                       bool perTargetCompilation);
};

QT_END_NAMESPACE
#endif
