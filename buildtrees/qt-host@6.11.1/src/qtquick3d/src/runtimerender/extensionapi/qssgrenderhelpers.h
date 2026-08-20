// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERHELPERS_H
#define QSSGRENDERHELPERS_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the QtQuick3D API, with limited compatibility guarantees.
// Usage of this API may make your code source and binary incompatible with
// future versions of Qt.
//

#include <QtQuick3DRuntimeRender/qtquick3druntimerenderglobal.h>

#include <ssg/qssgrenderbasetypes.h>

#include <QtCore/qsize.h>

#include <QtGui/qmatrix4x4.h>

QT_BEGIN_NAMESPACE

class QSSGFrameData;
class QRhiRenderPassDescriptor;
class QRhiTexture;
class QSSGRhiGraphicsPipelineState;
class QSSGRenderContextInterface;
class QSSGRenderExtension;

#ifdef Q_QDOC
typedef quint64 QSSGPrepContextId;
typedef quint64 QSSGPrepResultId;
typedef quint64 QSSGRenderablesId;
#else
enum class QSSGPrepContextId : quint64 { Invalid };
enum class QSSGPrepResultId : quint64 { Invalid };
enum class QSSGRenderablesId : quint64 { Invalid };
#endif

enum class QSSGRenderablesFilter : quint32
{
    Opaque = 0x1,
    Transparent = 0x2,
    All = Opaque | Transparent
};

Q_DECLARE_FLAGS(QSSGRenderablesFilters, QSSGRenderablesFilter)

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGModelHelpers
{
public:
    using MaterialList = QList<QSSGResourceId>;

    static void setModelMaterials(const QSSGFrameData &frameData,
                                  QSSGRenderablesId renderablesId,
                                  QSSGNodeId model,
                                  MaterialList materials);

    static void setModelMaterials(const QSSGFrameData &frameData,
                                  QSSGRenderablesId renderablesId,
                                  MaterialList materials);

    [[nodiscard]] static QMatrix4x4 getGlobalTransform(const QSSGFrameData &frameData,
                                                       QSSGNodeId model,
                                                       QSSGPrepContextId prepId = {});

    [[nodiscard]] static QMatrix4x4 getLocalTransform(const QSSGFrameData &frameData,
                                                      QSSGNodeId model);
    [[nodiscard]] static float getGlobalOpacity(const QSSGFrameData &frameData,
                                                QSSGNodeId model);
    [[nodiscard]] static float getGlobalOpacity(const QSSGFrameData &frameData,
                                                QSSGNodeId model,
                                                QSSGPrepContextId prepId);
    [[nodiscard]] static float getLocalOpacity(const QSSGFrameData &frameData,
                                               QSSGNodeId model);

    static void setGlobalTransform(const QSSGFrameData &frameData,
                                   QSSGRenderablesId prepId,
                                   QSSGNodeId model,
                                   const QMatrix4x4 &transform);

    static void setGlobalOpacity(const QSSGFrameData &frameData,
                                 QSSGRenderablesId renderablesId,
                                 QSSGNodeId model,
                                 float opacity);
private:
    QSSGModelHelpers();
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGCameraHelpers
{
public:
    static QMatrix4x4 getViewProjectionMatrix(const QSSGCameraId cameraId,
                                              const QMatrix4x4 *globalTransform = nullptr);

private:
    QSSGCameraHelpers();
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderHelpers
{
public:
    enum class CreateFlag : quint32
    {
        None,
        Recurse = 0x1,
        Steal  = 0x2
    };

    Q_DECLARE_FLAGS(CreateFlags, CreateFlag)

    [[nodiscard]] static QSSGRenderablesId createRenderables(const QSSGFrameData &frameData,
                                                             QSSGPrepContextId prepId,
                                                             const QSSGNodeIdList &nodes,
                                                             CreateFlags flags = CreateFlag::None);

    [[nodiscard]] static QSSGPrepContextId prepareForRender(const QSSGFrameData &frameData,
                                                            const QSSGRenderExtension &ext,
                                                            QSSGCameraId cameraId,
                                                            quint32 slot = 0);

    [[nodiscard]] static QSSGPrepResultId commit(const QSSGFrameData &frameData,
                                                 QSSGPrepContextId prepId,
                                                 QSSGRenderablesId renderablesId,
                                                 float lodThreshold = 1.0f);

    static void prepareRenderables(const QSSGFrameData &frameData,
                                   QSSGPrepResultId prepId,
                                   QRhiRenderPassDescriptor *renderPassDescriptor,
                                   QSSGRhiGraphicsPipelineState &ps,
                                   QSSGRenderablesFilters filter = QSSGRenderablesFilter::All);

    static void renderRenderables(const QSSGFrameData &frameData,
                                  QSSGPrepResultId prepId);


private:
    QSSGRenderHelpers();
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderExtensionHelpers
{
public:
    static void registerRenderResult(const QSSGFrameData &frameData,
                                     QSSGExtensionId extension,
                                     QRhiTexture *texture);

private:
    QSSGRenderExtensionHelpers();
};

QT_END_NAMESPACE

#endif // QSSGRENDERHELPERS_H
