// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_GRAPH_OBJECT_H
#define QSSG_RENDER_GRAPH_OBJECT_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the QtQuick3D API, with limited compatibility guarantees.
// Usage of this API may make your code source and binary incompatible with
// future versions of Qt.
//

#include <QtQuick3DRuntimeRender/qtquick3druntimerenderexports.h>
#include <QtCore/QDebug>
#include <QtCore/qtconfigmacros.h>
#include <QtQuick/qtquickglobal.h>

QT_BEGIN_NAMESPACE

// NOTE: Duplicat to avoid pulling in private headers
#ifndef Q_QUICK3D_PROFILE_ID
#    if QT_CONFIG(qml_debug)
#       define Q_QUICK3D_PROFILE_ID_ qint32 profilingId = -1;
#    else
#       define Q_QUICK3D_PROFILE_ID_
#    endif
#else
#    define Q_QUICK3D_PROFILE_ID_ Q_QUICK3D_PROFILE_ID
#endif


class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderGraphObject
{
    Q_DISABLE_COPY_MOVE(QSSGRenderGraphObject)
public:
    // Types should be setup on construction. Change the type
    // at your own risk as the type is used for RTTI purposes.
    // See QSSGRenderGraphObject, QQuick3DObject and QSSceneDesc (asset useage).

    enum BaseType : quint32 {
        // Internal types
        Node = 0x1000,
        Light = 0x2000,
        Camera = 0x4000,
        Renderable = 0x8000,
        Resource = 0x10000,
        Material = 0x20000,
        Texture = 0x40000,
        Extension = 0x80000,
        User =   0x80000000,
    };

    enum class Type : quint32 {
        Unknown = 0,
        // Nodes
        Node = BaseType::Node,
        Root, // Node
        Layer, // Node
        Joint, // Node
        Skeleton, // Node (A resource to the model node)
        ImportScene, // Node
        ReflectionProbe,
        SceneRoot, // Node
        // Light nodes
        DirectionalLight = BaseType::Light | BaseType::Node,
        PointLight,
        SpotLight,
        // Camera nodes
        OrthographicCamera = BaseType::Camera | BaseType::Node,
        PerspectiveCamera,
        CustomFrustumCamera, // Perspective camera with user specified frustum bounds.
        CustomCamera,
        // Renderable nodes
        Model = BaseType::Renderable | BaseType::Node, // Renderable Node
        Item2D, // Renderable Node
        Particles, // Renderable Node
        // Resources
        SceneEnvironment = BaseType::Resource, // Resource
        Effect, // Resource
        Geometry, // Resource
        TextureData, // Resource
        MorphTarget, // Resource
        ModelInstance, // Resource
        ModelBlendParticle, // Resource
        ResourceLoader, // Resource [meta]
        RenderPass, // Resource
        // Materials
        DefaultMaterial = BaseType::Material | BaseType::Resource, // Resource
        PrincipledMaterial, // Resource
        CustomMaterial, // Resource
        SpecularGlossyMaterial, //Resource
        Skin, // Resource
        // Textures
        Image2D = BaseType::Texture | BaseType::Resource, // Resource
        ImageCube, // Resource
        RenderExtension = BaseType::Extension, // Extension
        TextureProvider, // Extension
        // User types E.g.: (User | Node) + 1)
    };
    using TypeT = std::underlying_type_t<Type>;

    enum class Flags : quint32 {
        HasGraphicsResources = 0x1,
        InternallyReserved = 0xffff0000,
    };
    using FlagT = std::underlying_type_t<Flags>;

    [[nodiscard]] static constexpr bool isNodeType(Type type) noexcept
    {
        return (TypeT(type) & BaseType::Node);
    }

    [[nodiscard]] static constexpr bool isLight(Type type) noexcept
    {
        return (TypeT(type) & BaseType::Light);
    }

    [[nodiscard]] static constexpr bool isCamera(Type type) noexcept
    {
        return (TypeT(type) & BaseType::Camera);
    }

    [[nodiscard]] static constexpr bool isMaterial(Type type) noexcept
    {
        return (TypeT(type) & BaseType::Material);
    }

    [[nodiscard]] static constexpr bool isTexture(Type type) noexcept
    {
        return (TypeT(type) & BaseType::Texture);
    }

    [[nodiscard]] static constexpr bool isRenderable(Type type) noexcept
    {
        return (TypeT(type) & BaseType::Renderable);
    }

    [[nodiscard]] static constexpr bool isResource(Type type) noexcept
    {
        return (TypeT(type) & BaseType::Resource);
    }

    [[nodiscard]] static constexpr bool isExtension(Type type) noexcept
    {
        return (TypeT(type) & BaseType::Extension);
    }

    // Note: This could have been a non-static member, as we never create or use
    // user types we do the built-in types; In any case just follow the existing pattern.
    [[nodiscard]] static constexpr bool isUserType(Type type) noexcept
    {
        return (TypeT(type) & BaseType::User);
    }

    // Objects tagged with HasGraphicsResources get special handling and will be queued for release
    // on the render thread after the frame has been submitted. See cleanupNodes() in the scene manager.
    [[nodiscard]] inline bool hasGraphicsResources() const noexcept
    {
        return ((flags & FlagT(Flags::HasGraphicsResources)) != 0);
    }

    const Type type;
    FlagT flags { 0 };
    Q_QUICK3D_PROFILE_ID_

    virtual ~QSSGRenderGraphObject();

#ifndef QT_NO_DEBUG_STREAM
    friend Q_QUICK3DRUNTIMERENDER_EXPORT QDebug operator<<(QDebug stream, QSSGRenderGraphObject::Type type);
#endif

protected:
    explicit QSSGRenderGraphObject(QSSGRenderGraphObject::Type inType);
    explicit QSSGRenderGraphObject(QSSGRenderGraphObject::Type inType, FlagT inFlags) : type(inType), flags(inFlags) {}
};

#ifndef QT_NO_DEBUG_STREAM
Q_QUICK3DRUNTIMERENDER_EXPORT QDebug operator<<(QDebug, QSSGRenderGraphObject::Type type);
#endif

QT_END_NAMESPACE

#endif // QSSG_RENDER_GRAPH_OBJECT_H
