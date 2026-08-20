// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDEREXTENSIIONS_H
#define QSSGRENDEREXTENSIIONS_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the QtQuick3D API, with limited compatibility guarantees.
// Usage of this API may make your code source and binary incompatible with
// future versions of Qt.
//

#include <QtQuick3DRuntimeRender/qtquick3druntimerenderexports.h>
#include <ssg/qssgrenderbasetypes.h>
#include <ssg/qssgrendergraphobject.h>
#include <ssg/qssgrhicontext.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QSSGRenderContextInterface;
class QSSGLayerRenderData;
class QRhiTexture;
class QRhiRenderBuffer;

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGFrameData
{
public:
    enum class RenderResult : quint32
    {
        AoTexture,
        DepthTexture,
        ScreenTexture,
        NormalTexture,
        MotionVectorTexture,
    };
    Q_DECLARE_FLAGS(RenderResults, RenderResult)

    enum class AttachmentSelector : quint32
    {
        Attachment0 = 0,
        Attachment1 = 1,
        Attachment2 = 2,
        Attachment3 = 3,
    };

    struct Result
    {
        QRhiTexture *texture = nullptr;
        QRhiRenderBuffer *buffer = nullptr;
    };

    using TypeMask = QSSGRenderGraphObject::TypeT;
    static constexpr TypeMask NodeMask = QSSGRenderGraphObject::BaseType::Node;

    void scheduleRenderResults(RenderResults results) const;

    Result getRenderResult(RenderResult id) const;
    Result getRenderResult(QSSGResourceId userPassId, AttachmentSelector attachment) const;

    qsizetype getAttachmentCount(QSSGResourceId userPassId) const;

    [[nodiscard]] QSSGRhiGraphicsPipelineState getPipelineState() const;

    [[nodiscard]] QSSGCameraId activeCamera() const;

    [[nodiscard]] QSSGRenderContextInterface *contextInterface() const;

    [[nodiscard]] QSSGNodeIdList getLayerNodes(quint32 layerMask, TypeMask typeMask = NodeMask) const;
    [[nodiscard]] QSSGNodeIdList getLayerNodes(QSSGCameraId cameraId, TypeMask typeMask = NodeMask) const;

private:
    friend class QSSGLayerRenderData;
    friend class QSSGRenderOutputProviderExtension;

    void clear();

    [[nodiscard]] QSSGLayerRenderData *getCurrent() const;

    void scheduleRenderResults(QSSGResourceId userPassId) const;

    QSSGFrameData() = default;
    explicit QSSGFrameData(QSSGRenderContextInterface *ctx);
    QSSGRenderContextInterface *m_ctx = nullptr;
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderExtension : public QSSGRenderGraphObject
{
public:
    enum class RenderMode
    {
        Standalone,
        Main
    };

    enum class RenderStage
    {
        PreColor,
        PostColor
    };

    QSSGRenderExtension();
    virtual ~QSSGRenderExtension();

    virtual bool prepareData(QSSGFrameData &data) = 0;
    virtual void prepareRender(QSSGFrameData &data) = 0;
    virtual void render(QSSGFrameData &data) = 0;

    virtual void resetForFrame() = 0;

    virtual RenderMode mode() const = 0;
    virtual RenderStage stage() const = 0;

protected:
    QSSGRenderExtension(Type inType, FlagT inFlags);
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderTextureProviderExtension : public QSSGRenderExtension
{
public:
    QSSGRenderTextureProviderExtension();
    ~QSSGRenderTextureProviderExtension() override;

    RenderMode mode() const final;
    RenderStage stage() const final;
};

QT_END_NAMESPACE

#endif // QSSGRENDEREXTENSIIONS_H
