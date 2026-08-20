// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERGRAPHOBJECT_P_H
#define QSSGRENDERGRAPHOBJECT_P_H

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

#include <ssg/qssgrendergraphobject.h>

#include <ssg/qssgrenderbasetypes.h>

#include <QtQuick3DUtils/private/qssgassert_p.h>

namespace QSSGRenderGraphObjectUtils {

enum class InternalFlags : quint32
{
    AutoRegisterExtension = 0x10000,
};

constexpr QSSGResourceId getResourceId(const QSSGRenderGraphObject &o)
{
    QSSG_ASSERT(QSSGRenderGraphObject::isResource(o.type), return QSSGResourceId::Invalid);
    return QSSGResourceId{ quintptr(&o) };
}

template <typename T = QSSGRenderGraphObject>
T *getResource(QSSGResourceId resId)
{
    return static_cast<T *>(reinterpret_cast<QSSGRenderGraphObject *>(resId));
}

constexpr QSSGNodeId getNodeId(const QSSGRenderGraphObject &o)
{
    QSSG_ASSERT(QSSGRenderGraphObject::isNodeType(o.type), return QSSGNodeId::Invalid);
    return QSSGNodeId{ quintptr(&o) };
}

template <typename T = QSSGRenderGraphObject>
T *getNode(QSSGNodeId nodeId)
{
    return static_cast<T *>(reinterpret_cast<QSSGRenderGraphObject *>(nodeId));
}

constexpr QSSGCameraId getCameraId(const QSSGRenderGraphObject &o)
{
    QSSG_ASSERT(QSSGRenderGraphObject::isCamera(o.type), return QSSGCameraId::Invalid);
    return QSSGCameraId{ quintptr(&o) };
}

template <typename T = QSSGRenderGraphObject>
T *getCamera(QSSGCameraId cameraId)
{
    return static_cast<T *>(reinterpret_cast<QSSGRenderGraphObject *>(cameraId));
}

constexpr QSSGExtensionId getExtensionId(const QSSGRenderGraphObject &o)
{
    QSSG_ASSERT(QSSGRenderGraphObject::isExtension(o.type), return QSSGExtensionId::Invalid);
    return QSSGExtensionId{ quintptr(&o) };
}

template <typename T = QSSGRenderGraphObject>
T *getExtension(QSSGExtensionId extensionId)
{
    return static_cast<T *>(reinterpret_cast<QSSGRenderGraphObject *>(extensionId));
}

template <typename QSSGTypeId>
constexpr bool isNull(QSSGTypeId id) { return (id == QSSGTypeId::Invalid); }

[[nodiscard]] static constexpr QSSGRenderGraphObject::BaseType getBaseType(QSSGRenderGraphObject::Type type) noexcept
{
    constexpr auto BaseTypeMask = QSSGRenderGraphObject::TypeT(~0xFFF);
    return static_cast<QSSGRenderGraphObject::BaseType>(QSSGRenderGraphObject::TypeT(type) & BaseTypeMask);
}

// The TextureProvider type is not a BaseType but it's a special type of Extension that is user
// facing and requires special handling in some places, so we keep this helper here for convenience.
[[nodiscard]] static constexpr bool isTextureProvider(QSSGRenderGraphObject::Type type) noexcept
{
    return (QSSGRenderGraphObject::TypeT(type) & QSSGRenderGraphObject::TypeT(QSSGRenderGraphObject::Type::TextureProvider));
}

// A scene root is a special node that acts as the root of a single viewport's part of the scene graph.
[[nodiscard]] static constexpr bool isSceneRoot(QSSGRenderGraphObject::Type type) noexcept
{
    return (type == QSSGRenderGraphObject::Type::SceneRoot);
}

[[nodiscard]] static constexpr bool isUserRenderPass(QSSGRenderGraphObject::Type type) noexcept
{
    return (type == QSSGRenderGraphObject::Type::RenderPass);
}

[[nodiscard]] static constexpr bool hasInternalFlags(const QSSGRenderGraphObject &obj)
{
    return (obj.flags & QSSGRenderGraphObject::FlagT(QSSGRenderGraphObject::Flags::InternallyReserved));
}

[[nodiscard]] static constexpr bool hasInternalFlag(const QSSGRenderGraphObject &obj, InternalFlags flag)
{
    return (obj.flags & QSSGRenderGraphObject::FlagT(flag));

}

} // namespace QSSGRenderGraphObjectUtils

#endif // QSSGRENDERGRAPHOBJECT_P_H
