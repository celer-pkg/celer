// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "custommaterial.h"

#include <QtCore/qdir.h>

#include <QtQuick3DAssetUtils/private/qssgrtutilities_p.h>
#include <QtQuick3DAssetUtils/private/qssgqmlutilities_p.h>

#include <QtQuick3D/private/qquick3dshaderutils_p.h>

QT_BEGIN_NAMESPACE

using TextureStore = QHash<QString, QImage *>;
Q_GLOBAL_STATIC(TextureStore, s_textureStore);

template<typename T>
static void setProperty(QQuick3DObject &obj, const char *name, T v)
{
    if (QQuick3DObjectPrivate::get(&obj)->type == QQuick3DObjectPrivate::Type::CustomMaterial) {
        if constexpr (std::is_same_v<T, QSSGSceneDesc::Texture *>) {
            if (QQuick3DTexture *texture = qobject_cast<QQuick3DTexture *>(v ? v->obj : nullptr)) {
                auto textureInput = new QQuick3DShaderUtilsTextureInput(&obj);
                textureInput->setTexture(texture);
                obj.setProperty(name, QVariant::fromValue(textureInput));
            }
        } else {
            obj.setProperty(name, QVariant::fromValue(v));
        }
    }
}

void CustomMaterial::setUniform(QSSGSceneDesc::Material &material, const Uniform &uniform)
{
    if (uniform.name.isEmpty()) {
        qWarning() << "Uniform without name! Skipping";
        return;
    }

    constexpr auto Dynamic = QSSGSceneDesc::Property::Type::Dynamic;

    switch (uniform.type) {
    case Uniform::Type::Bool:
        QSSGSceneDesc::setProperty(material, uniform.name.constData(), &setProperty<bool>, uniform.b, Dynamic);
        break;
    case Uniform::Type::Int:
        QSSGSceneDesc::setProperty(material, uniform.name.constData(), &setProperty<int>, uniform.i, Dynamic);
        break;
    case Uniform::Type::Float:
        QSSGSceneDesc::setProperty(material, uniform.name.constData(), &setProperty<float>, uniform.f, Dynamic);
        break;
    case Uniform::Type::Vec2:
        QSSGSceneDesc::setProperty(material, uniform.name.constData(), &setProperty<QVector2D>, uniform.vec2, Dynamic);
        break;
    case Uniform::Type::Vec3:
        QSSGSceneDesc::setProperty(material, uniform.name.constData(), &setProperty<QVector3D>, uniform.vec3, Dynamic);
        break;
    case Uniform::Type::Vec4:
        QSSGSceneDesc::setProperty(material, uniform.name.constData(), &setProperty<QVector4D>, uniform.vec4, Dynamic);
        break;
    case Uniform::Type::Mat44:
        QSSGSceneDesc::setProperty(material, uniform.name.constData(), &setProperty<QMatrix4x4>, uniform.m44, Dynamic);
        break;
    case Uniform::Type::Sampler:
    {
        QFileInfo fi(uniform.imagePath);
        const auto &path = fi.canonicalFilePath();
        if (!path.isEmpty()) {
            auto it = s_textureStore->constFind(path);
            const auto end = s_textureStore->constEnd();
            if (it == end) {
                QImage image;
                if (image.load(path))
                    it = s_textureStore->insert(path, new QImage(image));
                else
                    qWarning("Failed to load image %s", qPrintable(path));
            }

            if (it != end) {
                const auto &image = *(*it);
                const QSize resSize = image.size();
                // Note: the image must not be deleted or modified while the textureData node is in the scene
                auto *imageData = reinterpret_cast<const char *>(image.constBits());
                QByteArray dataref = QByteArray::fromRawData(imageData, image.sizeInBytes());
                const auto format = QByteArrayLiteral("rgba8888");
                const auto &baseName = QString(fi.baseName() + QString::number(material.id));
                auto name = baseName.toUtf8();
                auto textureData = new QSSGSceneDesc::TextureData(dataref, resSize, format, 0, name);
                QSSGSceneDesc::addNode(material, *textureData);
                auto texture = new QSSGSceneDesc::Texture(QSSGSceneDesc::Texture::RuntimeType::Image2D);
                QSSGSceneDesc::addNode(material, *texture);
                QSSGSceneDesc::setProperty(*texture, "textureData", &QQuick3DTexture::setTextureData, textureData);
                QSSGSceneDesc::setProperty(material, uniform.name.constData(), &setProperty<QSSGSceneDesc::Texture *>, texture, Dynamic);
            }
        }
        break;
    }
    case Uniform::Type::Last:
        Q_UNREACHABLE();
        break;
    }
}

QPointer<QQuick3DCustomMaterial> CustomMaterial::create(QQuick3DNode &parent, const UniformTable &uniforms, const Properties &properties, const Shaders &shaders)
{
    using namespace QSSGSceneDesc;
    QQuick3DCustomMaterial *ret = nullptr;
    if (scene.root)
        scene.reset();

    QSSGSceneDesc::Material *material = new Material(Material::RuntimeType::CustomMaterial);
    addNode(scene, *material);

    // Set uniforms
    for (const auto &uniform : uniforms)
        setUniform(*material, uniform);

    // Set common properties
    const Properties def;
    if (def.cullMode != properties.cullMode)
        setProperty(*material, "cullMode", &QQuick3DCustomMaterial::setCullMode, properties.cullMode);
    if (def.depthDrawMode != properties.depthDrawMode)
        setProperty(*material, "depthDrawMode", &QQuick3DCustomMaterial::setDepthDrawMode, properties.depthDrawMode);
    if (def.shadingMode != properties.shadingMode)
        setProperty(*material, "shadingMode", &QQuick3DCustomMaterial::setShadingMode, properties.shadingMode);
    if (def.sourceBlend != properties.sourceBlend)
        setProperty(*material, "sourceBlend", &QQuick3DCustomMaterial::setSrcBlend, properties.sourceBlend);
    if (def.destinationBlend != properties.destinationBlend)
        setProperty(*material, "destinationBlend", &QQuick3DCustomMaterial::setDstBlend, properties.destinationBlend);

    if (!shaders.vert.isEmpty())
        setProperty(*material, "vertexShader", &QQuick3DCustomMaterial::setVertexShader, QUrl{ shaders.vert.toString() });
    if (!shaders.frag.isEmpty())
        setProperty(*material, "fragmentShader", &QQuick3DCustomMaterial::setFragmentShader, QUrl{ shaders.frag.toString() });

    auto resourceParent = std::make_unique<QQuick3DNode>();
    QSSGRuntimeUtils::createScene(parent, scene);
    if (auto customMaterial = qobject_cast<QQuick3DCustomMaterial *>(material->obj)) {
        resourceParent.release()->setParent(&parent);
        ret = customMaterial;
    }

    return ret;
}

bool CustomMaterial::isValid() const { return scene.root != nullptr && scene.root->runtimeType == QSSGSceneDesc::Material::RuntimeType::CustomMaterial; }

QDataStream &CustomMaterial::readFromDataStream(QDataStream &stream, Uniform &uniform)
{
    int type;
    stream >> type >> uniform.name;
    uniform.type = Uniform::Type(type);
    switch (uniform.type) {
    case Uniform::Type::Bool:
        stream >> uniform.b;
        break;
    case Uniform::Type::Int:
        stream >> uniform.i;
        break;
    case Uniform::Type::Float:
        stream >> uniform.f;
        break;
    case Uniform::Type::Vec2:
        stream >> uniform.vec2;
        break;
    case Uniform::Type::Vec3:
        stream >> uniform.vec3;
        break;
    case Uniform::Type::Vec4:
        stream >> uniform.vec4;
        break;
    case Uniform::Type::Mat44:
        stream >> uniform.m44;
        break;
    case Uniform::Type::Sampler:
    {
        QImage image;
        stream >> uniform.imagePath >> image;
        s_textureStore->insert(uniform.imagePath, new QImage(image));
    }
        break;
    case Uniform::Type::Last:
        Q_UNREACHABLE();
        break;
    }
    return stream;
}

QDataStream &CustomMaterial::writeToDataStream(QDataStream &stream, const Uniform &uniform)
{
    stream << int(uniform.type) << uniform.name;
    switch (uniform.type) {
    case Uniform::Type::Bool:
        stream << uniform.b;
        break;
    case Uniform::Type::Int:
        stream << uniform.i;
        break;
    case Uniform::Type::Float:
        stream << uniform.f;
        break;
    case Uniform::Type::Vec2:
        stream << uniform.vec2;
        break;
    case Uniform::Type::Vec3:
        stream << uniform.vec3;
        break;
    case Uniform::Type::Vec4:
        stream << uniform.vec4;
        break;
    case Uniform::Type::Mat44:
        stream << uniform.m44;
        break;
    case Uniform::Type::Sampler:
    {
        const auto &path = uniform.imagePath;
        auto it = s_textureStore->constFind(path);
        const auto end = s_textureStore->cend();
        if (it != end)
            stream << path << *(*it);
    }
        break;
    case Uniform::Type::Last:
        Q_UNREACHABLE();
        break;
    }
    return stream;
}

void CustomMaterial::writeQmlComponent(const QSSGSceneDesc::Material &material, QTextStream &stream)
{
    QSSGQmlUtilities::writeQmlComponent(material, stream, QDir());
}

QT_END_NAMESPACE
