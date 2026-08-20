// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CUSTOMMATERIAL_H
#define CUSTOMMATERIAL_H

#include <QtQuick3DAssetUtils/private/qssgscenedesc_p.h>

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class CustomMaterial
{
public:
    struct Uniform
    {
        enum class Type
        {
            Bool,
            Int,
            Float,
            Vec2,
            Vec3,
            Vec4,
            Mat44,
            Sampler,
            Last
        };
        union {
            bool b;
            int i;
            float f;
            QVector2D vec2;
            QVector3D vec3;
            QVector4D vec4;
            quintptr d;
        };
        Type type;
        QByteArray name;
        QMatrix4x4 m44;
        QString imagePath;
    };
    using UniformTable = QList<Uniform>;
    using TextureStore = QList<QImage *>;

    struct Properties
    {
        QQuick3DCustomMaterial::CullMode cullMode { QQuick3DCustomMaterial::CullMode::BackFaceCulling };
        QQuick3DCustomMaterial::DepthDrawMode depthDrawMode { QQuick3DCustomMaterial::DepthDrawMode::OpaqueOnlyDepthDraw };
        QQuick3DCustomMaterial::ShadingMode shadingMode { QQuick3DCustomMaterial::ShadingMode::Shaded };
        QQuick3DCustomMaterial::BlendMode sourceBlend { QQuick3DCustomMaterial::BlendMode::NoBlend };
        QQuick3DCustomMaterial::BlendMode destinationBlend { QQuick3DCustomMaterial::BlendMode::NoBlend };
    } properties;

    struct Shaders
    {
        QUrl vert;
        QUrl frag;
    };

    static void setUniform(QSSGSceneDesc::Material &material, const Uniform &uniform);
    QPointer<QQuick3DCustomMaterial> create(QQuick3DNode &parent, const UniformTable &uniforms, const Properties &properties, const Shaders &shaders);
    bool isValid() const;
    friend QTextStream &operator<<(QTextStream &stream, const CustomMaterial &material)
    {
        using namespace QSSGSceneDesc;
        const auto &scene = material.scene;
        if (auto node = scene.root) {
            Q_ASSERT(node->runtimeType == Material::RuntimeType::CustomMaterial);
            writeQmlComponent(static_cast<const QSSGSceneDesc::Material &>(*node), stream);
        }
        return stream;
    }

    friend QDataStream &operator<<(QDataStream &stream, const CustomMaterial::Uniform &uniform) { return writeToDataStream(stream, uniform); }
    friend QDataStream &operator>>(QDataStream &stream, CustomMaterial::Uniform &uniform) { return readFromDataStream(stream, uniform); }

private:
    static QDataStream &readFromDataStream(QDataStream &stream, Uniform &uniform);
    static QDataStream &writeToDataStream(QDataStream &stream, const Uniform &uniform);
    static void writeQmlComponent(const QSSGSceneDesc::Material &material, QTextStream &stream);
    QSSGSceneDesc::Scene scene;
};

QT_END_NAMESPACE

#endif // CUSTOMMATERIAL_H
