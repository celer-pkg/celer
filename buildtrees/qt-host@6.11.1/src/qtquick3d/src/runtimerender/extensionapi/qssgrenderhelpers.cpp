// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrendercontextcore.h"
#include "qssgrenderhelpers.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderhelpers_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>

#include "graphobjects/qssgrendermodel_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>

#include <QtQuick3DUtils/private/qssgassert_p.h>

QT_BEGIN_NAMESPACE

 /*!
    \class QSSGRenderHelpers
    \inmodule QtQuick3D
    \since 6.7

    \brief Class containing helper functions for setting up and render QtQuick3D renderables.
*/

/*!
    \typedef QSSGPrepContextId
    \relates QtQuick3D

    Handle to a preparation context. Setting up a preparation context is the first
    step needed before renderables can be created and rendered.

    \sa QSSGRenderHelpers::prepareForRender()
*/

/*!
    \typedef QSSGRenderablesId
    \brief Handle to a set of renderables.
    \relates QtQuick3D

    Handle to a set of renderables created for one or more node(s). This \c id can be used
    to for example modify the renderables created for a specific model.
*/

/*!
    \typedef QSSGPrepResultId
    \relates QtQuick3D

    Handle to a preparation result.

    Once the \l {QSSGRenderHelpers::createRenderables}{renderables} for a frame are updated and ready
    to be translated into rendering code by the engine, the \l {QSSGRenderHelpers::createRenderables}{renderables}
    and the \l {QSSGRenderHelpers::prepareForRender()}{preparation context} can be \l {QSSGRenderHelpers::commit()}{committed}.
    If the commit succeeds, the returned preparation result can be used to \l {QSSGRenderHelpers::prepareRenderables()}{prepare}
    and \l {QSSGRenderHelpers::renderRenderables}{record} the rendering for the frame.
*/

/*!
    \enum QSSGRenderHelpers::CreateFlag

    \value None The default value. Renderables are created only for the nodes specified.
    \value Recurse Renderables are created for each node and their children.
    \value Steal Renderables are taken from the engine and won't be rendered by QtQuick3D.

    \note Calling \l QSSGRenderHelpers::createRenderables() without the {QSSGRenderHelpers::CreateFlag::Steal}{Steal}
    flag set means nodes are duplicated and QtQuick3D will render its copy of the nodes as normal.
*/

/*!
    Takes a list of node ids and create renderables that can be further processed by the renderer.
    If there are no nodes, or no renderable nodes in the list, the returned id will be invalid.

    By default the function does not recurse down and included children of the \a nodes in the list.
    Enabling recursion can be achieved by passing in the \l{CreateFlag::}{Recurse} flag in
    the \a flags argument.

    \return an id to the created renderables.

    \a frameData, \a prepId

    \sa CreateFlags, prepareForRender()
 */
QSSGRenderablesId QSSGRenderHelpers::createRenderables(const QSSGFrameData &frameData,
                                                       QSSGPrepContextId prepId,
                                                       const QSSGNodeIdList &nodes,
                                                       CreateFlags flags)
{
    QSSGRenderablesId rid { QSSGRenderablesId::Invalid };
    if (nodes.size() > 0) {
        auto *ctx = frameData.contextInterface();
        auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
        QSSG_ASSERT_X(layer, "No active layer for renderer!", return rid);
        return layer->createRenderables(prepId, nodes, flags);
    }

    return rid;
}

/*!
    prepareForRender() creates a context for collecting and storing information about the render-data
    associated with this render extension.

    If the same nodes are to be rendered more then once but with different properties, for example
    a different material or camera, then a new context will be needed. To create several contexts for
    one extension the \a slot argument can be used. The default context is created in slot \b 0.

    \return an id to the prep context.

    \a frameData, \a ext, \a cameraId

    \sa commit()
 */

QSSGPrepContextId QSSGRenderHelpers::prepareForRender(const QSSGFrameData &frameData,
                                                      const QSSGRenderExtension &ext,
                                                      QSSGCameraId cameraId,
                                                      quint32 slot)
{
    auto *cn = QSSGRenderGraphObjectUtils::getCamera<QSSGRenderCamera>(cameraId);
    QSSG_ASSERT_X(cn && QSSGRenderGraphObject::isCamera(cn->type), "CameraId is not a camera!", return QSSGPrepContextId::Invalid);
    auto *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return QSSGPrepContextId::Invalid);
    return layer->getOrCreateExtensionContext(ext, cn, slot);
}

/*!
    Once the required changes have been done to the renderables, the data can marked as ready for
    the renderer.

    \return an id to the preparation result.

    \a frameData, \a prepId, \a renderablesId, \a lodThreshold

    \sa prepareRenderables(), renderRenderables()
 */
QSSGPrepResultId QSSGRenderHelpers::commit(const QSSGFrameData &frameData,
                                           QSSGPrepContextId prepId,
                                           QSSGRenderablesId renderablesId,
                                           float lodThreshold)
{
    auto *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return QSSGPrepResultId::Invalid);
    return layer->prepareModelsForRender(*ctx, prepId, renderablesId, lodThreshold);
}

/*!
    Prepare the draw call data needed for the renderables before calling \l {renderRenderables}.

    \return an id to the preparation result.

    \a frameData, \a renderPassDescriptor, \a ps, \a prepId, \a filter

    \sa renderRenderables()
 */
void QSSGRenderHelpers::prepareRenderables(const QSSGFrameData &frameData,
                                           QSSGPrepResultId prepId,
                                           QRhiRenderPassDescriptor *renderPassDescriptor,
                                           QSSGRhiGraphicsPipelineState &ps,
                                           QSSGRenderablesFilters filter)
{
    auto *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return);
    layer->prepareRenderables(*ctx, prepId, renderPassDescriptor, ps, filter);
}

/*!
    Render the renderables.

    \a frameData, \a prepId

    \sa prepareRenderables()
 */
void QSSGRenderHelpers::renderRenderables(const QSSGFrameData &frameData,
                                          QSSGPrepResultId prepId)
{
    QSSGRenderContextInterface *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return);
    layer->renderRenderables(*ctx, prepId);
}

QSSGRenderHelpers::QSSGRenderHelpers()
{

}

 /*!
    \class QSSGModelHelpers
    \inmodule QtQuick3D
    \since 6.7

    \brief Class containing helper functions for modifying and setting data for model renderables.
*/

/*!
    Sets the \a materials to be used on \a model.

    \note As with the \l {QtQuick3D::Model::materials}{materials} on the \l {QtQuick3D::Model}{model} item,
    materials are applied in the same manner.

    The sub-mesh uses a material from the \l{materials} list, corresponding to its index. If the number
    of materials is less than the sub-meshes, the last material in the list is used for subsequent
    sub-meshes.

    \a frameData \a renderablesId

    \sa QSSGRenderHelpers::createRenderables()
 */
void QSSGModelHelpers::setModelMaterials(const QSSGFrameData &frameData,
                                         QSSGRenderablesId renderablesId,
                                         QSSGNodeId model,
                                         MaterialList materials)
{
    auto *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return);
    auto *renderModel = QSSGRenderGraphObjectUtils::getNode<QSSGRenderModel>(model);
    QSSG_ASSERT_X(renderModel && renderModel->type == QSSGRenderGraphObject::Type::Model, "Invalid model-id!", return);
    layer->setModelMaterials(renderablesId, *renderModel, materials);
}

/*!
    Convenience function to apply \a materials to all models in the renderablesId set.

    \a frameData, \a renderablesId

    \sa QSSGRenderHelpers::createRenderables()
 */
void QSSGModelHelpers::setModelMaterials(const QSSGFrameData &frameData,
                                         QSSGRenderablesId renderablesId,
                                         MaterialList materials)
{
    auto *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return);
    layer->setModelMaterials(renderablesId, materials);
}

/*!
    \return Returns the global transform for the \a model in context of the \a prepId. By default the prep context argument is
    QSSGPrepContextId::Uninitialized which returns the model's original global transform.

    \a frameData

    \sa QSSGRenderHelpers::createRenderables()
*/
QMatrix4x4 QSSGModelHelpers::getGlobalTransform(const QSSGFrameData &frameData,
                                                QSSGNodeId model,
                                                QSSGPrepContextId prepId)
{
    auto *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return {});
    auto *renderModel = QSSGRenderGraphObjectUtils::getNode<QSSGRenderModel>(model);
    QSSG_ASSERT_X(renderModel && renderModel->type == QSSGRenderGraphObject::Type::Model, "Invalid model-id!", return {});
    return (prepId != QSSGPrepContextId::Invalid) ? layer->getGlobalTransform(prepId, *renderModel)
                                                  : layer->getGlobalTransform(*renderModel);
}

/*!
    \return Returns the local transform for the \a model.

    \a frameData
*/
QMatrix4x4 QSSGModelHelpers::getLocalTransform(const QSSGFrameData &frameData, QSSGNodeId model)
{
    Q_UNUSED(frameData);
    auto *renderModel = QSSGRenderGraphObjectUtils::getNode<QSSGRenderModel>(model);
    QSSG_ASSERT_X(renderModel && renderModel->type == QSSGRenderGraphObject::Type::Model, "Invalid model-id!", return {});
    return renderModel->localTransform;
}

/*!
    \return Returns the global opacity for the \a model.

    \a frameData
*/
float QSSGModelHelpers::getGlobalOpacity(const QSSGFrameData &frameData, QSSGNodeId model)
{
    auto *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return {});
    auto *renderModel = QSSGRenderGraphObjectUtils::getNode<QSSGRenderModel>(model);
    QSSG_ASSERT_X(renderModel && renderModel->type == QSSGRenderGraphObject::Type::Model, "Invalid model-id!", return {});
    return layer->getGlobalOpacity(*renderModel);
}

/*!
    \return Returns the global opacity for the \a model in context of the \a prepId. By default the prep context argument is
    QSSGPrepContextId::Uninitialized which returns the model's original global opacity.

    \a frameData

    \sa QSSGRenderHelpers::createRenderables()
*/
float QSSGModelHelpers::getGlobalOpacity(const QSSGFrameData &frameData, QSSGNodeId model, QSSGPrepContextId prepId = QSSGPrepContextId::Invalid)
{
    auto *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return {});
    auto *renderModel = QSSGRenderGraphObjectUtils::getNode<QSSGRenderModel>(model);
    QSSG_ASSERT_X(renderModel && renderModel->type == QSSGRenderGraphObject::Type::Model, "Invalid model-id!", return {});
    return (prepId != QSSGPrepContextId::Invalid) ? layer->getGlobalOpacity(prepId, *renderModel) : layer->getGlobalOpacity(*renderModel);
}

/*!
    \return Returns the local opacity for the \a model.

    \a frameData
*/
float QSSGModelHelpers::getLocalOpacity(const QSSGFrameData &frameData, QSSGNodeId model)
{
    Q_UNUSED(frameData);
    auto *renderModel = QSSGRenderGraphObjectUtils::getNode<QSSGRenderModel>(model);
    QSSG_ASSERT_X(renderModel && renderModel->type == QSSGRenderGraphObject::Type::Model, "Invalid model-id!", return {});
    return renderModel->localOpacity;
}

/*!
    Sets the global transform for \a model in the context of the \a renderablesId.

    \a frameData, \a transform

    \sa QSSGRenderHelpers::createRenderables()
 */
void QSSGModelHelpers::setGlobalTransform(const QSSGFrameData &frameData,
                                          QSSGRenderablesId renderablesId,
                                          QSSGNodeId model,
                                          const QMatrix4x4 &transform)
{
    auto *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return);
    auto *node = QSSGRenderGraphObjectUtils::getNode(model);
    QSSG_ASSERT_X(node && node->type == QSSGRenderGraphObject::Type::Model, "Invalid model-id!", return);
    const auto &renderModel = static_cast<const QSSGRenderModel &>(*node);
    layer->setGlobalTransform(renderablesId, renderModel, transform);
}

/*!
    Sets the global opacity for \a model in the context of the \a renderablesId.

    \a frameData, \a opacity

    \sa QSSGRenderHelpers::createRenderables()
 */
void QSSGModelHelpers::setGlobalOpacity(const QSSGFrameData &frameData, QSSGRenderablesId renderablesId, QSSGNodeId model, float opacity)
{
    auto *ctx = frameData.contextInterface();
    auto *layer = QSSGLayerRenderData::getCurrent(*ctx->renderer());
    QSSG_ASSERT_X(layer, "No active layer for renderer!", return);
    auto *node = QSSGRenderGraphObjectUtils::getNode(model);
    QSSG_ASSERT_X(node && node->type == QSSGRenderGraphObject::Type::Model, "Invalid model-id!", return);
    const auto &renderModel = static_cast<const QSSGRenderModel &>(*node);
    layer->setGlobalOpacity(renderablesId, renderModel, opacity);
}

 /*!
    \class QSSGCameraHelpers
    \inmodule QtQuick3D
    \since 6.7

    \brief Class containing helper functions for getting camera data used for rendering.
*/

/*!
    Get the projection matrix for \a cameraId. An optional transform argument can be given to be used
    instead of the cameras global transform when calculating the projection matrix.

    \return projection matrix for \a cameraId

    \a globalTransform

    \sa QSSGRenderHelpers::createRenderables()
 */
QMatrix4x4 QSSGCameraHelpers::getViewProjectionMatrix(const QSSGCameraId cameraId,
                                                      const QMatrix4x4 *globalTransform)
{
    auto *renderCamera = QSSGRenderGraphObjectUtils::getCamera<QSSGRenderCamera>(cameraId);
    QSSG_ASSERT(renderCamera && QSSGRenderGraphObject::isCamera(renderCamera->type), return {});

    QMatrix4x4 mat44{Qt::Uninitialized};
    const auto &projection = renderCamera->projection;
    const auto &transform = (globalTransform != 0) ? *globalTransform : renderCamera->localTransform;
    QSSGRenderCamera::calculateViewProjectionMatrix(transform, projection, mat44);
    return mat44;
}

 /*!
    \class QSSGRenderExtensionHelpers
    \inmodule QtQuick3D
    \since 6.7

    \brief Class containing helper functions for the extensions.
*/

/*!
    Register a render result, in form of a texture, for this \a extension. Once a texture is registered,
    the extension can be used as a {QtQuick3D::Texture::textureProvider}{texture provider} in QML.

    \note To ensure that the \a texture is available for renderables, for example to be used by a {QtQuick3D::Texture} item,
    textures should be registered during the \l QSSGRenderExtension::prepareData call of the extension.

    \note Calling this function with a new texture will unregister any previously registered texture.
    To unregister a texture, call this function with a \c nullptr texture.

    \a frameData

    \sa {QtQuick3D::Texture::textureProvider}{textureProvider}
 */
void QSSGRenderExtensionHelpers::registerRenderResult(const QSSGFrameData &frameData,
                                                      QSSGExtensionId extension,
                                                      QRhiTexture *texture)
{
    if (auto *ext = QSSGRenderGraphObjectUtils::getExtension<QSSGRenderExtension>(extension)) {
        const QSSGRenderContextInterface *ctx = frameData.contextInterface();
        ctx->bufferManager()->registerExtensionResult(*ext, texture);
    }
}

QT_END_NAMESPACE
