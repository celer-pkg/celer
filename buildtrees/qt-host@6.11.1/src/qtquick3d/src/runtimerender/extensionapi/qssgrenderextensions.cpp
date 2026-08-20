// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderextensions.h"
#include "private/qssgassert_p.h"
#include "private/qssglayerrenderdata_p.h"
#include "qssgrendercontextcore.h"

QT_BEGIN_NAMESPACE

/*!
    \class QSSGFrameData
    \inmodule QtQuick3D
    \since 6.7

    \brief Storage class containing data collected for a frame.
*/

/*!
    \return The renderable texture result from \a id. \nullptr if no matching \a id was found.

    \note Even if the function returns a non-null result, the returned QSSGRhiRenderableTexture
    might not be ready unless the pass rendering to the texture has been executed.

    \note The returned value is only valid within the current frame. On each new frame
    the renderable will be reset and should therefore be queried again.
*/
QSSGFrameData::Result QSSGFrameData::getRenderResult(RenderResult id) const
{
    const QSSGRhiRenderableTexture *res = nullptr;
    auto *data = QSSGLayerRenderData::getCurrent(*m_ctx->renderer());
    const auto resultKey = QSSGRenderResult::toInternalRenderResultKey(id);
    if (QSSG_GUARD(data && (std::size(data->renderResults) > size_t(resultKey))))
        res = data->getRenderResult(resultKey);

    return res ? Result{ res->texture, res->depthStencil } : Result{};
}

/*!
    Schedule the given \a results to be made available for this frame.

    This function should only be called during the prepare phase in \l QSSGRenderExtension::prepareData().

    \note The requested results might not be available if the underlying layer does not support
    them or if the layer does not contain any data that would make it necessary to produce the
    requested results, in which case \l getRenderResult() will return a empty result.

    \sa QSSGRenderExtension::getRenderResult()
*/

void QSSGFrameData::scheduleRenderResults(RenderResults results) const
{
    auto *data = QSSGLayerRenderData::getCurrent(*m_ctx->renderer());
    QSSG_ASSERT(data, return);

    auto &prepResult = data->layerPrepResult;

    if (prepResult.getState() != QSSGLayerRenderPreparationResult::State::DataPrep) {
        qWarning("QSSGFrameData::requestRenderResults: "
                "Requesting render results should only be done during the prepare phase in prepareData().");
        return;
    }

    if (results.testFlag(QSSGFrameData::RenderResult::DepthTexture))
        prepResult.flags.setRequiresDepthTexture(true);
    if (results.testFlag(QSSGFrameData::RenderResult::ScreenTexture))
        prepResult.flags.setRequiresScreenTexture(true);
    if (results.testFlag(RenderResult::AoTexture)) {
        // NOTE: AO depends on the depth texture
        prepResult.flags.setRequiresSsaoPass(true);
        prepResult.flags.setRequiresDepthTexture(true);
    }
    if (results.testFlag(RenderResult::NormalTexture))
        prepResult.flags.setRequiresNormalTexture(true);
}

qsizetype QSSGFrameData::getAttachmentCount(QSSGResourceId userPassId) const
{
    QSSGRenderUserPass *userPassNode = QSSGRenderGraphObjectUtils::getResource<QSSGRenderUserPass>(userPassId);
    QSSG_ASSERT(userPassNode && userPassNode->type == QSSGRenderGraphObject::Type::RenderPass, return 0);

    QSSGRhiRenderableTextureV2Ptr res;
    auto *data = QSSGLayerRenderData::getCurrent(*m_ctx->renderer());
    if (QSSG_GUARD(data))
        res = data->requestUserRenderPassManager()->getUserPassTexureResult(*userPassNode);

    return res->colorAttachmentCount();
}

/*!
    \fn QSSGFrameData::Result QSSGFrameData::getRenderResult(QSSGResourceId userPassId, AttachmentSelector attachment) const

    \return The renderable texture result from a user defined render pass with the given \a userPassId.
    If no matching user pass could be found, or if the user pass did not produce any renderable textures,
    an invalid Result is returned.

    \note Even if the function returns a non-null result, the returned QSSGRhiRenderableTexture
    might not be ready unless the pass rendering to the texture has been executed.

    \note The returned value is only valid within the current frame. On each new frame
    the renderable will be reset and should therefore be queried again.
*/
QSSGFrameData::Result QSSGFrameData::getRenderResult(QSSGResourceId userPassId, AttachmentSelector attachment) const
{
    QSSGRenderUserPass *userPassNode = QSSGRenderGraphObjectUtils::getResource<QSSGRenderUserPass>(userPassId);
    QSSG_ASSERT(userPassNode && userPassNode->type == QSSGRenderGraphObject::Type::RenderPass, return Result{});

    QSSGRhiRenderableTextureV2Ptr res;
    auto *data = QSSGLayerRenderData::getCurrent(*m_ctx->renderer());
    if (QSSG_GUARD(data))
        res = data->requestUserRenderPassManager()->getUserPassTexureResult(*userPassNode);

    Result result;

    const quint32 attachmentSelector = static_cast<quint32>(attachment);

    if (res && res->isValid()) {
        if (const qsizetype attachmentCount = res->colorAttachmentCount(); attachmentCount > qsizetype(attachmentSelector)) {
            // Get the selected attachment
            result = { res->getColorTexture(attachmentSelector)->texture().get(), res->getDepthStencil().get() };
        } else {
            qWarning() << "Requested attachment" << attachmentSelector << "but user pass only has" << attachmentCount << "attachments.";
        }
    }

    return result;
}

/*!
  \internal
*/
void QSSGFrameData::scheduleRenderResults(QSSGResourceId userPassId) const
{
    QSSGRenderUserPass *userPassNode = QSSGRenderGraphObjectUtils::getResource<QSSGRenderUserPass>(userPassId);
    QSSG_ASSERT(userPassNode && userPassNode->type == QSSGRenderGraphObject::Type::RenderPass, return);

    QSSGLayerRenderData *data = QSSGLayerRenderData::getCurrent(*m_ctx->renderer());
    if (QSSG_GUARD(data))
        data->requestUserRenderPassManager()->scheduleUserPass(userPassNode);
}

/*!
    \return Base pipeline state for this frame
 */
QSSGRhiGraphicsPipelineState QSSGFrameData::getPipelineState() const
{
    auto *data = QSSGLayerRenderData::getCurrent(*m_ctx->renderer());
    QSSG_ASSERT(data, return {});
    return data->getPipelineState();
}

/*!
    \return The active camera for the scene, or null if non could be found.
*/
QSSGCameraId QSSGFrameData::activeCamera() const
{
    QSSGCameraId ret { QSSGCameraId::Invalid };
    auto *data = QSSGLayerRenderData::getCurrent(*m_ctx->renderer());
    QSSG_ASSERT(data, return ret);
    if (auto *ac = data->activeCamera())
        ret = QSSGRenderGraphObjectUtils::getCameraId(*ac);

    return ret;
}

QSSGRenderContextInterface *QSSGFrameData::contextInterface() const
{
    return m_ctx;
}

/*!
  \return A list of layer nodes that match the \a layerMask and \a typeMask.
 */
QSSGNodeIdList QSSGFrameData::getLayerNodes(quint32 layerMask, TypeMask typeMask) const
{
    QSSG_ASSERT(m_ctx, return {});

    auto *layer = getCurrent();
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return {});
    const auto &layerNodes = layer->layerNodes;

    return QSSGLayerRenderData::filter(layerNodes, layerMask, typeMask);;
}

/*!
    \return A list of layer nodes for the given \a cameraId that match the \a typeMask.
    If the camera does not have a layer mask, an empty list is returned.
*/
QSSGNodeIdList QSSGFrameData::getLayerNodes(QSSGCameraId cameraId, TypeMask typeMask) const
{
    auto *camera = QSSGRenderGraphObjectUtils::getNode<QSSGRenderCamera>(QSSGNodeId(cameraId));
    const quint32 layerMask = camera ? camera->tag.value() : 0 /* LayerNone */;

    return (layerMask != 0) ? getLayerNodes(layerMask, typeMask) : QSSGNodeIdList{};
}

void QSSGFrameData::clear()
{

}

QSSGLayerRenderData *QSSGFrameData::getCurrent() const
{
    return QSSGLayerRenderData::getCurrent(*m_ctx->renderer());
}

QSSGFrameData::QSSGFrameData(QSSGRenderContextInterface *ctx)
    : m_ctx(ctx)
{

}

/*!
    \class QSSGRenderExtension
    \inmodule QtQuick3D
    \since 6.7

    \brief Base class for extension backend node implementations.

    \sa QQuick3DRenderExtension
*/

/*!
    Constructor that allows users to specifying a user-type and flags for an extension.

    \note For user-defined extensions the type must be a combination of \l QSSGRenderGraphObject::BaseType::User
    and a value between 0 and 4095.

    \note The \l QSSGRenderGraphObject::BaseType::Extension type is automatically added to the given \a inType.

    \note The \a inFlags must include \l Flags::HasGraphicsResources if the extension
          allocates graphics resources.

 */
QSSGRenderExtension::QSSGRenderExtension(Type inType, FlagT inFlags)
    : QSSGRenderGraphObject(static_cast<Type>(TypeT(inType) | TypeT(QSSGRenderGraphObject::BaseType::Extension)), inFlags)
{
    Q_ASSERT_X((QSSGRenderGraphObjectUtils::getBaseType(type) == QSSGRenderGraphObject::BaseType::Extension) ||
                  (QSSGRenderGraphObjectUtils::getBaseType(type) == (QSSGRenderGraphObject::BaseType::Extension | QSSGRenderGraphObject::BaseType::User)),
                  "QSSGRenderExtension()",
                  "The type must be a combination of QSSGRenderGraphObject::BaseType::Extension "
                  "and optionally QSSGRenderGraphObject::BaseType::User.");
}

QSSGRenderExtension::QSSGRenderExtension()
    : QSSGRenderGraphObject(QSSGRenderGraphObject::Type::RenderExtension, FlagT(Flags::HasGraphicsResources))
{

}

QSSGRenderExtension::~QSSGRenderExtension()
{

}

/*!
    \enum QSSGRenderExtension::RenderMode

    Specifies the render extension mode.

    \value Standalone The rendering code is recorded in full during the render prepare phase.
    This will usually imply that there are some output crated for a preceding render extension(s).
    When this mode is used the \l prepareRender() and \l render() functions are both called during
    the frame's prepare phase.

    \value Main The rendering code is recorded within the main render pass. In this mode the
    \l prepareRender() is called in the frame's prepare phase while \l render() is called the frame's render phase.

*/

/*!
    \enum QSSGRenderExtension::RenderStage

    Specifies the order the extension will be called.

    \value PreColor The rendering code is recorded and executed before the main (color) pass.
    \value PostColor The rendering code is recorded and executed after the main (color) pass.

    \note The \l RenderStage is only relevant when the \l RenderMode is set to \l {RenderMode::Main}{Main}.
*/


/*!
    Called after scene \a data is collected, but before any render data or rendering in the current
    frame has been done.

    \return Dirty state. Return \c true if the there are dirty data
    that needs to be rendered.

    \note Much of the data created/collected from the engine during the prepare and render phases
    is per-frame and should be released or assumed released at the start of the next frame

    \sa QSSGFrameData
*/
bool QSSGRenderExtension::prepareData(QSSGFrameData &data)
{
    Q_UNUSED(data);
    return false;
}

/*!
    Prepare data for rendering. Build and collect \a data needed for rendering. Any render extension
    scheduled before this one has been processed. In addition; any render extension of
    mode \l RenderMode::Standalone will, if successful, have been completed in full.

    \note Much of the data created/collected from the engine during the prepare and render phases
    is per-frame and should be released or assumed released at the start of the next frame

    \sa QSSGFrameData
*/
void QSSGRenderExtension::prepareRender(QSSGFrameData &data)
{
    Q_UNUSED(data);
}

/*!
    Record the render pass. Depending on the extensions \l {RenderMode}{mode} this function will be called
    during the frame's prepare or render phase.

    Use \a data to gain access to the render context from which the active QRhi object can be queried.

    \sa QSSGRenderExtension::RenderMode
*/
void QSSGRenderExtension::render(QSSGFrameData &data)
{
    Q_UNUSED(data);
}

/*!
    Called each time a new frame starts. Any data from the previous frame should be cleared at
    this point.
*/
void QSSGRenderExtension::resetForFrame()
{

}

/*!
    \return The render mode used for this extension.
 */
QSSGRenderExtension::RenderMode QSSGRenderExtension::mode() const
{
    return RenderMode::Main;
}

/*!
    \return The stage in which this render extension will be used.
*/
QSSGRenderExtension::RenderStage QSSGRenderExtension::stage() const
{
    return RenderStage::PostColor;
}

/*!
    \class QSSGRenderTextureProviderExtension
    \inmodule QtQuick3D
    \since 6.11

    \brief Base class for texture providers backend node implementations.

    \note This class is meant to be used together with \l QQuick3DTextureProviderExtension.
    and the \l mode and \l stage is always \c Standalone and \c PostColor respectively.

    \sa QQuick3DTextureProviderExtension
*/

QSSGRenderTextureProviderExtension::QSSGRenderTextureProviderExtension()
    : QSSGRenderExtension(QSSGRenderGraphObject::Type::TextureProvider, FlagT(Flags::HasGraphicsResources) | FlagT(QSSGRenderGraphObjectUtils::InternalFlags::AutoRegisterExtension))
{

}

QSSGRenderTextureProviderExtension::~QSSGRenderTextureProviderExtension()
{

}

QSSGRenderExtension::RenderStage QSSGRenderTextureProviderExtension::stage() const { return QSSGRenderExtension::RenderStage::PreColor; }

QSSGRenderExtension::RenderMode QSSGRenderTextureProviderExtension::mode() const { return QSSGRenderExtension::RenderMode::Standalone; }

QT_END_NAMESPACE
