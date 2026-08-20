// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "assimpimporter.h"

#include <assimputils.h>

#include <QtCore/qurl.h>
#include <QtCore/qbytearrayalgorithms.h>
#include <QtGui/QQuaternion>
#include <QtQml/QQmlFile>

#include <QtQuick3DAssetImport/private/qssgassetimporterfactory_p.h>
#include <QtQuick3DAssetImport/private/qssgassetimporter_p.h>
#include <QtQuick3DAssetUtils/private/qssgscenedesc_p.h>
#include <QtQuick3DAssetUtils/private/qssgsceneedit_p.h>

#include <QtQuick3DUtils/private/qssgutils_p.h>

// ASSIMP INC
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/GltfMaterial.h>
#include <assimp/importerdesc.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>

// ASSIMP INC

QT_BEGIN_NAMESPACE

////////////////////////  ASSIMP IMP

#define AI_GLTF_FILTER_NEAREST                  0x2600
#define AI_GLTF_FILTER_LINEAR                   0x2601
#define AI_GLTF_FILTER_NEAREST_MIPMAP_NEAREST   0x2700
#define AI_GLTF_FILTER_LINEAR_MIPMAP_NEAREST    0x2701
#define AI_GLTF_FILTER_NEAREST_MIPMAP_LINEAR    0x2702
#define AI_GLTF_FILTER_LINEAR_MIPMAP_LINEAR     0x2703

Q_REQUIRED_RESULT static inline QColor aiColorToQColor(const aiColor3D &color)
{
    return QColor::fromRgbF(color.r, color.g, color.b, 1.0f);
}

Q_REQUIRED_RESULT static inline QColor aiColorToQColor(const aiColor4D &color)
{
    return QColor::fromRgbF(color.r, color.g, color.b, color.a);
}

static QByteArray fromAiString(const aiString &string)
{
    const qsizetype length = string.length;
    return QByteArray(string.data, length);
}

struct NodeInfo
{
    using Type = QSSGSceneDesc::Node::Type;
    size_t index;
    Type type;
};

Q_DECLARE_TYPEINFO(NodeInfo, Q_PRIMITIVE_TYPE);

using NodeMap = QHash<const aiNode *, NodeInfo>;

using AnimationNodeMap = QHash<QByteArray, QSSGSceneDesc::Node *>;

[[nodiscard]] static inline bool isEqual(const aiUVTransform &a, const aiUVTransform &b)
{
    return (a.mTranslation == b.mTranslation && a.mScaling == b.mScaling && a.mRotation == b.mRotation);
};

struct TextureInfo
{
    aiTextureMapMode modes[3] {};
    aiTextureMapping mapping = aiTextureMapping::aiTextureMapping_UV;
    unsigned int minFilter { AI_GLTF_FILTER_LINEAR };
    unsigned int magFilter { AI_GLTF_FILTER_LINEAR };
    uint uvIndex { 0 };
    aiUVTransform transform;
};

bool operator==(const TextureInfo &a, const TextureInfo &b)
{
    return (a.mapping == b.mapping)
            && (std::memcmp(a.modes, b.modes, sizeof(a.modes)) == 0)
            && (a.minFilter == b.minFilter)
            && (a.magFilter == b.magFilter)
            && (a.uvIndex == b.uvIndex)
            && isEqual(a.transform, b.transform);
}

struct TextureEntry
{
    QByteArray name;
    TextureInfo info;
    QSSGSceneDesc::Texture *texture = nullptr;
};

size_t qHash(const TextureEntry &key, size_t seed)
{
    static_assert(std::is_same_v<decltype(key.info.transform), aiUVTransform>, "Unexpected type");
    const auto infoKey = quintptr(key.info.mapping)
                         ^ (quintptr(key.info.modes[0]) ^ quintptr(key.info.modes[1]) ^ quintptr(key.info.modes[2]))
                         ^ quintptr(key.info.minFilter ^ key.info.magFilter)
                         ^ quintptr(key.info.uvIndex)
                         ^ qHashBits(&key.info.transform, sizeof(aiUVTransform), seed);

    return qHash(key.name, seed) ^ infoKey;
}

bool operator==(const TextureEntry &a, const TextureEntry &b)
{
    return (a.name == b.name) && (a.info == b.info);
}

struct SceneInfo
{
    struct Options
    {
        bool gltfMode = false;
        bool fbxMode = false;
        bool binaryKeyframes = false;
        bool forceMipMapGeneration = false;
        bool useFloatJointIndices = false;
        bool designStudioWorkarounds = false;

        float globalScaleValue = 1.0;

        bool generateMeshLODs = false;
        float lodNormalMergeAngle = 60.0;
        float lodNormalSplitAngle = 25.0;
    };

    using MaterialMap = QVarLengthArray<QPair<const aiMaterial *, QSSGSceneDesc::Material *>>;
    using MeshMap = QVarLengthArray<QPair<const aiMesh *, QSSGSceneDesc::Mesh *>>;
    using EmbeddedTextureMap = QVarLengthArray<QSSGSceneDesc::TextureData *>;
    using TextureMap = QSet<TextureEntry>;

    struct skinData {
        aiBone **mBones;
        unsigned int mNumBones;
        QSSGSceneDesc::Skin *node;
    };
    using SkinMap = QVarLengthArray<skinData>;
    using Mesh2SkinMap = QVarLengthArray<qint16>;

    const aiScene &scene;
    MaterialMap &materialMap;
    MeshMap &meshMap;
    EmbeddedTextureMap &embeddedTextureMap;
    TextureMap &textureMap;
    SkinMap &skinMap;
    Mesh2SkinMap &mesh2skin;
    QDir workingDir;
    Options opt;
};

class ResourceIOStream : public Assimp::IOStream
{
public:
    ResourceIOStream(const char *pFile, const char *pMode);

    // IOStream interface
    size_t Read(void *pvBuffer, size_t pSize, size_t pCount) override;
    size_t Write(const void *pvBuffer, size_t pSize, size_t pCount) override;
    aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override;
    size_t Tell() const override;
    size_t FileSize() const override;
    void Flush() override;

private:
    QFile file;
};

ResourceIOStream::ResourceIOStream(const char *pFile, const char *pMode) : file(QString::fromStdString(pFile))
{
    QByteArray mode = QByteArray(pMode);
    QFile::OpenMode openMode = QFile::NotOpen;
    if (mode.startsWith("r"))
        openMode |= QFile::ReadOnly;
    else if (mode.startsWith("w"))
        openMode |= QFile::WriteOnly;
    if (mode.endsWith("t"))
        openMode |= QFile::Text;
    if (!file.open(openMode)) {
        qWarning("Failed to open file %s: %s",
                 qPrintable(file.fileName()), qPrintable(file.errorString()));
    }
}

size_t ResourceIOStream::Read(void *pvBuffer, size_t pSize, size_t pCount)
{
    size_t ret = 0;
    auto buffer = static_cast<char *>(pvBuffer);
    for (ret = 0; ret < pCount; ret++) {
        size_t read = file.read(buffer, pSize);
        if (read != pSize)
            return ret;
        buffer += read;
    }
    return ret;
}

size_t ResourceIOStream::Write(const void *pvBuffer, size_t pSize, size_t pCount)
{
    Q_UNUSED(pvBuffer);
    Q_UNUSED(pSize);
    Q_UNUSED(pCount);
    Q_UNIMPLEMENTED();
    return 0;
}

aiReturn ResourceIOStream::Seek(size_t pOffset, aiOrigin pOrigin)
{
    switch (pOrigin) {
    case aiOrigin_SET:
        file.seek(pOffset);
        break;
    case aiOrigin_CUR:
        file.seek(file.pos() + pOffset);
        break;
    case aiOrigin_END:
        file.seek(file.size() + pOffset);
        break;
    default:
        return aiReturn_FAILURE;
    }
    return aiReturn_SUCCESS;
}

size_t ResourceIOStream::Tell() const
{
    return file.pos();
}

size_t ResourceIOStream::FileSize() const
{
    return file.size();
}

void ResourceIOStream::Flush()
{
}

class ResourceIOSystem : public Assimp::IOSystem
{
public:
    ResourceIOSystem();
    // IOSystem interface
    bool Exists(const char *pFile) const override;
    char getOsSeparator() const override;
    Assimp::IOStream *Open(const char *pFile, const char *pMode) override;
    void Close(Assimp::IOStream *pFile) override;
};

ResourceIOSystem::ResourceIOSystem() : Assimp::IOSystem() { }

bool ResourceIOSystem::Exists(const char *pFile) const
{
    return QFile::exists(QString::fromStdString(pFile));
}

char ResourceIOSystem::getOsSeparator() const
{
    return QDir::separator().toLatin1();
}

Assimp::IOStream *ResourceIOSystem::Open(const char *pFile, const char *pMode)
{
    return new ResourceIOStream(pFile, pMode);
}

void ResourceIOSystem::Close(Assimp::IOStream *pFile)
{
    delete pFile;
}

static void setNodeProperties(QSSGSceneDesc::Node &target,
                              const aiNode &source,
                              const SceneInfo &sceneInfo,
                              aiMatrix4x4 *transformCorrection)
{
    // objectName
    if (target.name.isNull())
        target.name = fromAiString(source.mName);

    // Apply correction if necessary
    aiMatrix4x4 transformMatrix;
    if (transformCorrection)
        transformMatrix = source.mTransformation * *transformCorrection;
    else
        transformMatrix = source.mTransformation;

    // Decompose Transform Matrix to get properties
    aiVector3D scaling;
    aiQuaternion rotation;
    aiVector3D translation;
    transformMatrix.Decompose(scaling, rotation, translation);

    // translate
    if (!sceneInfo.opt.designStudioWorkarounds) {
        QSSGSceneDesc::setProperty(target, "position", &QQuick3DNode::setPosition, QVector3D { translation.x, translation.y, translation.z });
    } else {
        QSSGSceneDesc::setProperty(target, "x", &QQuick3DNode::setX, translation.x);
        QSSGSceneDesc::setProperty(target, "y", &QQuick3DNode::setY, translation.y);
        QSSGSceneDesc::setProperty(target, "z", &QQuick3DNode::setZ, translation.z);
    }


    // rotation
    const QQuaternion rot(rotation.w, rotation.x, rotation.y, rotation.z);
    QSSGSceneDesc::setProperty(target, "rotation", &QQuick3DNode::setRotation, rot);

    // scale
    QSSGSceneDesc::setProperty(target, "scale", &QQuick3DNode::setScale, QVector3D { scaling.x, scaling.y, scaling.z });
    // pivot

    // opacity

    // visible
}

static void setTextureProperties(QSSGSceneDesc::Texture &target, const TextureInfo &texInfo, const SceneInfo &sceneInfo)
{
    const bool forceMipMapGeneration = sceneInfo.opt.forceMipMapGeneration;

    if (texInfo.uvIndex > 0) {
        // Quick3D supports 2 tex coords.
        // According to gltf's khronos default implementation,
        // the index will be selected to the nearest one.
        QSSGSceneDesc::setProperty(target, "indexUV", &QQuick3DTexture::setIndexUV, 1);
    }

    // mapping
    if (texInfo.mapping == aiTextureMapping_UV) {
        // So we should be able to always hit this case by passing the right flags
        // at import.
        QSSGSceneDesc::setProperty(target, "mappingMode", &QQuick3DTexture::setMappingMode, QQuick3DTexture::MappingMode::UV);
        // It would be possible to use another channel than UV0 to map texture data
        // but for now we force everything to use UV0
        //int uvSource;
        //material->Get(AI_MATKEY_UVWSRC(textureType, index), uvSource);
    } // else (not supported)

    static const auto asQtTilingMode = [](aiTextureMapMode mode) {
        switch (mode) {
        case aiTextureMapMode_Wrap:
            return QQuick3DTexture::TilingMode::Repeat;
        case aiTextureMapMode_Clamp:
            return QQuick3DTexture::TilingMode::ClampToEdge;
        case aiTextureMapMode_Mirror:
            return QQuick3DTexture::TilingMode::MirroredRepeat;
        default:
            break;
        }

        return QQuick3DTexture::TilingMode::Repeat;
    };

    // mapping mode U
    QSSGSceneDesc::setProperty(target, "tilingModeHorizontal", &QQuick3DTexture::setHorizontalTiling, asQtTilingMode(texInfo.modes[0]));

    // mapping mode V
    QSSGSceneDesc::setProperty(target, "tilingModeVertical", &QQuick3DTexture::setVerticalTiling, asQtTilingMode(texInfo.modes[1]));

    const bool applyUvTransform = !isEqual(texInfo.transform, aiUVTransform());
    if (applyUvTransform) {
        // UV origins -
        //      glTF: 0, 1 (top left of texture)
        //      Assimp, Collada?, FBX?: 0.5, 0.5
        //      Quick3D: 0, 0 (bottom left of texture)
        // Assimp already tries to fix it but it's not correct.
        // So, we restore original values and then use pivot
        const auto &transform = texInfo.transform;
        float rotation = -transform.mRotation;
        float rotationUV = qRadiansToDegrees(rotation);
        float posU = transform.mTranslation.x;
        float posV = transform.mTranslation.y;
        if (sceneInfo.opt.gltfMode) {
            const float rcos = std::cos(rotation);
            const float rsin = std::sin(rotation);
            posU -= 0.5f * transform.mScaling.x * (-rcos + rsin + 1.0f);
            posV -= (0.5f * transform.mScaling.y * (rcos + rsin - 1.0f) + 1.0f - transform.mScaling.y);
            QSSGSceneDesc::setProperty(target, "pivotV", &QQuick3DTexture::setPivotV, 1.0f);
        } else {
            QSSGSceneDesc::setProperty(target, "pivotU", &QQuick3DTexture::setPivotV, 0.5f);
            QSSGSceneDesc::setProperty(target, "pivotV", &QQuick3DTexture::setPivotV, 0.5f);
        }

        QSSGSceneDesc::setProperty(target, "positionU", &QQuick3DTexture::setPositionU, posU);
        QSSGSceneDesc::setProperty(target, "positionV", &QQuick3DTexture::setPositionV, posV);
        QSSGSceneDesc::setProperty(target, "rotationUV", &QQuick3DTexture::setRotationUV, rotationUV);
        QSSGSceneDesc::setProperty(target, "scaleU", &QQuick3DTexture::setScaleU, transform.mScaling.x);
        QSSGSceneDesc::setProperty(target, "scaleV", &QQuick3DTexture::setScaleV, transform.mScaling.y);
    }
    // We don't make use of the data here, but there are additional flags
    // available for example the usage of the alpha channel
    // texture flags
    //int textureFlags;
    //material->Get(AI_MATKEY_TEXFLAGS(textureType, index), textureFlags);

    // Always generate and use mipmaps for imported assets
    bool generateMipMaps = forceMipMapGeneration;
    auto mipFilter = forceMipMapGeneration ? QQuick3DTexture::Filter::Linear : QQuick3DTexture::Filter::None;

    // magFilter
    auto filter = (texInfo.magFilter == AI_GLTF_FILTER_NEAREST) ? QQuick3DTexture::Filter::Nearest : QQuick3DTexture::Filter::Linear;
    QSSGSceneDesc::setProperty(target, "magFilter", &QQuick3DTexture::setMagFilter, filter);

    // minFilter
    if (texInfo.minFilter == AI_GLTF_FILTER_NEAREST) {
        filter = QQuick3DTexture::Filter::Nearest;
    } else if (texInfo.minFilter == AI_GLTF_FILTER_LINEAR) {
        filter = QQuick3DTexture::Filter::Linear;
    } else if (texInfo.minFilter == AI_GLTF_FILTER_NEAREST_MIPMAP_NEAREST) {
        filter = QQuick3DTexture::Filter::Nearest;
        mipFilter = QQuick3DTexture::Filter::Nearest;
    } else if (texInfo.minFilter == AI_GLTF_FILTER_LINEAR_MIPMAP_NEAREST) {
        filter = QQuick3DTexture::Filter::Linear;
        mipFilter = QQuick3DTexture::Filter::Nearest;
    } else if (texInfo.minFilter == AI_GLTF_FILTER_NEAREST_MIPMAP_LINEAR) {
        filter = QQuick3DTexture::Filter::Nearest;
        mipFilter = QQuick3DTexture::Filter::Linear;
    } else if (texInfo.minFilter == AI_GLTF_FILTER_LINEAR_MIPMAP_LINEAR) {
        filter = QQuick3DTexture::Filter::Linear;
        mipFilter = QQuick3DTexture::Filter::Linear;
    }
    QSSGSceneDesc::setProperty(target, "minFilter", &QQuick3DTexture::setMinFilter, filter);

    // mipFilter
    generateMipMaps = (mipFilter != QQuick3DTexture::Filter::None);

    if (generateMipMaps) {
        QSSGSceneDesc::setProperty(target, "generateMipmaps", &QQuick3DTexture::setGenerateMipmaps, true);
        QSSGSceneDesc::setProperty(target, "mipFilter", &QQuick3DTexture::setMipFilter, mipFilter);
    }
}

static void setMaterialProperties(QSSGSceneDesc::Material &target, const aiMaterial &source, const SceneInfo &sceneInfo, QSSGSceneDesc::Material::RuntimeType type)
{
    if (target.name.isNull()) {
        aiString materialName = source.GetName();
        target.name = fromAiString(materialName);
    }

    const auto createTextureNode = [&sceneInfo, &target](const aiMaterial &material, aiTextureType textureType, unsigned int index) {
        const auto &srcScene = sceneInfo.scene;
        QSSGSceneDesc::Texture *tex = nullptr;
        aiString texturePath;
        TextureInfo texInfo;

        if (material.GetTexture(textureType, index, &texturePath, &texInfo.mapping, &texInfo.uvIndex, nullptr, nullptr, texInfo.modes) == aiReturn_SUCCESS) {
            if (texturePath.length > 0) {
                aiUVTransform transform;
                if (material.Get(AI_MATKEY_UVTRANSFORM(textureType, index), transform) == aiReturn_SUCCESS)
                    texInfo.transform = transform;

                material.Get(AI_MATKEY_UVWSRC(textureType, index), texInfo.uvIndex);
                material.Get(AI_MATKEY_GLTF_MAPPINGFILTER_MIN(textureType, index), texInfo.minFilter);
                material.Get(AI_MATKEY_GLTF_MAPPINGFILTER_MAG(textureType, index), texInfo.magFilter);

                auto &textureMap = sceneInfo.textureMap;

                QByteArray texName = QByteArray(texturePath.C_Str(), texturePath.length);
                // Check if we already processed this texture
                const auto it = textureMap.constFind(TextureEntry{texName, texInfo});
                if (it != textureMap.cend()) {
                    Q_ASSERT(it->texture);
                    tex = it->texture;
                } else {
                    // Two types, externally referenced or embedded
                    // Use the source file name as the identifier, since that will hopefully be fairly stable for re-import.
                    tex = new QSSGSceneDesc::Texture(QSSGSceneDesc::Texture::RuntimeType::Image2D, texName);
                    textureMap.insert(TextureEntry{fromAiString(texturePath), texInfo, tex});
                    QSSGSceneDesc::addNode(target, *tex);
                    setTextureProperties(*tex, texInfo, sceneInfo); // both

                    auto aEmbeddedTex = srcScene.GetEmbeddedTextureAndIndex(texturePath.C_Str());
                    const auto &embeddedTexId = aEmbeddedTex.second;
                    if (embeddedTexId > -1) {
                        QSSGSceneDesc::TextureData *textureData = nullptr;
                        auto &embeddedTextures = sceneInfo.embeddedTextureMap;
                        textureData = embeddedTextures[embeddedTexId];
                        if (!textureData) {
                            const auto *sourceTexture = aEmbeddedTex.first;
                            Q_ASSERT(sourceTexture->pcData);
                            // Two cases of embedded textures, uncompress and compressed.
                            const bool isCompressed = (sourceTexture->mHeight == 0);

                            // For compressed textures this is the size of the image buffer (in bytes)
                            const qsizetype asize = (isCompressed) ? sourceTexture->mWidth : (sourceTexture->mHeight * sourceTexture->mWidth) * sizeof(aiTexel);
                            const QSize size = (!isCompressed) ? QSize(int(sourceTexture->mWidth), int(sourceTexture->mHeight)) : QSize();
                            QByteArray imageData { reinterpret_cast<const char *>(sourceTexture->pcData), asize };
                            const auto format = (isCompressed) ? QByteArray(sourceTexture->achFormatHint) : QByteArrayLiteral("rgba8888");
                            const quint8 flags = isCompressed ? quint8(QSSGSceneDesc::TextureData::Flags::Compressed) : 0;
                            textureData = new QSSGSceneDesc::TextureData(imageData, size, format, flags);
                            QSSGSceneDesc::addNode(*tex, *textureData);
                            embeddedTextures[embeddedTexId] = textureData;
                        }

                        if (textureData)
                            QSSGSceneDesc::setProperty(*tex, "textureData", &QQuick3DTexture::setTextureData, textureData);
                    } else {
                        auto relativePath = QString::fromUtf8(texturePath.C_Str());
                        // Replace Windows separator to Unix separator
                        // so that assets including Windows relative path can be converted on Unix.
                        relativePath.replace("\\","/");
                        const auto path = sceneInfo.workingDir.absoluteFilePath(relativePath);
                        QSSGSceneDesc::setProperty(*tex, "source", &QQuick3DTexture::setSource, QUrl{ path });
                    }
                }
            }
        }

        return tex;
    };

    aiReturn result;

    if (type == QSSGSceneDesc::Material::RuntimeType::PrincipledMaterial) {
        {
            aiColor4D baseColorFactor;
            result = source.Get(AI_MATKEY_BASE_COLOR, baseColorFactor);
            if (result == aiReturn_SUCCESS) {
                // Some special handling required since baseColorFactor's are stored as linear factors and need to be converted for Qt Quick
                const QColor sRGBBaseColorFactor = QSSGUtils::color::linearTosRGB(QVector4D(baseColorFactor.r, baseColorFactor.g, baseColorFactor.b, baseColorFactor.a));
                QSSGSceneDesc::setProperty(target, "baseColor", &QQuick3DPrincipledMaterial::setBaseColor, sRGBBaseColorFactor);
            } else {
                // Also try diffuse color as a fallback
                aiColor3D diffuseColor;
                result = source.Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target, "baseColor", &QQuick3DPrincipledMaterial::setBaseColor, aiColorToQColor(diffuseColor));
            }
        }

        if (auto baseColorTexture = createTextureNode(source, AI_MATKEY_BASE_COLOR_TEXTURE)) {
            QSSGSceneDesc::setProperty(target, "baseColorMap", &QQuick3DPrincipledMaterial::setBaseColorMap, baseColorTexture);
            QSSGSceneDesc::setProperty(target, "opacityChannel", &QQuick3DPrincipledMaterial::setOpacityChannel, QQuick3DPrincipledMaterial::TextureChannelMapping::A);
        } else if (auto diffuseMapTexture = createTextureNode(source, aiTextureType_DIFFUSE, 0)) {
            // Also try to legacy diffuse texture as an alternative
            QSSGSceneDesc::setProperty(target, "baseColorMap", &QQuick3DPrincipledMaterial::setBaseColorMap, diffuseMapTexture);
        }

        if (auto metalicRoughnessTexture = createTextureNode(source, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE)) {
            QSSGSceneDesc::setProperty(target, "metalnessMap", &QQuick3DPrincipledMaterial::setMetalnessMap, metalicRoughnessTexture);
            QSSGSceneDesc::setProperty(target, "metalnessChannel", &QQuick3DPrincipledMaterial::setMetalnessChannel, QQuick3DPrincipledMaterial::TextureChannelMapping::B);
            QSSGSceneDesc::setProperty(target, "roughnessMap", &QQuick3DPrincipledMaterial::setRoughnessMap, metalicRoughnessTexture);
            QSSGSceneDesc::setProperty(target, "roughnessChannel", &QQuick3DPrincipledMaterial::setRoughnessChannel, QQuick3DPrincipledMaterial::TextureChannelMapping::G);
        }

        {
            ai_real metallicFactor;
            result = source.Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "metalness", &QQuick3DPrincipledMaterial::setMetalness, float(metallicFactor));
        }

        {
            ai_real roughnessFactor;
            result = source.Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "roughness", &QQuick3DPrincipledMaterial::setRoughness, float(roughnessFactor));
        }

        if (auto normalTexture = createTextureNode(source, aiTextureType_NORMALS, 0)) {
            QSSGSceneDesc::setProperty(target, "normalMap", &QQuick3DPrincipledMaterial::setNormalMap, normalTexture);
            {
                ai_real normalScale;
                result = source.Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), normalScale);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target, "normalStrength", &QQuick3DPrincipledMaterial::setNormalStrength, float(normalScale));
            }
        }

        // Occlusion Textures are not implimented (yet)
        if (auto occlusionTexture = createTextureNode(source, aiTextureType_LIGHTMAP, 0)) {
            QSSGSceneDesc::setProperty(target, "occlusionMap", &QQuick3DPrincipledMaterial::setOcclusionMap, occlusionTexture);
            QSSGSceneDesc::setProperty(target, "occlusionChannel", &QQuick3DPrincipledMaterial::setOcclusionChannel, QQuick3DPrincipledMaterial::TextureChannelMapping::R);
            {
                ai_real occlusionAmount;
                result = source.Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_LIGHTMAP, 0), occlusionAmount);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target, "occlusionAmount", &QQuick3DPrincipledMaterial::setOcclusionAmount, float(occlusionAmount));
            }
        }

        if (auto emissiveTexture = createTextureNode(source, aiTextureType_EMISSIVE, 0))
            QSSGSceneDesc::setProperty(target, "emissiveMap", &QQuick3DPrincipledMaterial::setEmissiveMap, emissiveTexture);

        {
            aiColor3D emissiveColorFactor;
            result = source.Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColorFactor);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "emissiveFactor", &QQuick3DPrincipledMaterial::setEmissiveFactor, QVector3D { emissiveColorFactor.r, emissiveColorFactor.g, emissiveColorFactor.b });
        }

        {
            bool isDoubleSided;
            result = source.Get(AI_MATKEY_TWOSIDED, isDoubleSided);
            if (result == aiReturn_SUCCESS && isDoubleSided)
                QSSGSceneDesc::setProperty(target, "cullMode", &QQuick3DPrincipledMaterial::setCullMode, QQuick3DPrincipledMaterial::CullMode::NoCulling);
        }

        {
            aiString alphaMode;
            result = source.Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
            if (result == aiReturn_SUCCESS) {
                auto mode = QQuick3DPrincipledMaterial::AlphaMode::Default;
                if (QByteArrayView(alphaMode.C_Str()) == "OPAQUE")
                    mode = QQuick3DPrincipledMaterial::AlphaMode::Opaque;
                else if (QByteArrayView(alphaMode.C_Str()) == "MASK")
                    mode = QQuick3DPrincipledMaterial::AlphaMode::Mask;
                else if (QByteArrayView(alphaMode.C_Str()) == "BLEND")
                    mode = QQuick3DPrincipledMaterial::AlphaMode::Blend;

                if (mode != QQuick3DPrincipledMaterial::AlphaMode::Default) {
                    QSSGSceneDesc::setProperty(target, "alphaMode", &QQuick3DPrincipledMaterial::setAlphaMode, mode);
                    // If the mode is mask, we also need to force OpaquePrePassDepthDraw mode
                    if (mode == QQuick3DPrincipledMaterial::AlphaMode::Mask)
                        QSSGSceneDesc::setProperty(target, "depthDrawMode", &QQuick3DPrincipledMaterial::setDepthDrawMode, QQuick3DMaterial::OpaquePrePassDepthDraw);
                }
            }
        }

        {
            ai_real alphaCutoff;
            result = source.Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "alphaCutoff", &QQuick3DPrincipledMaterial::setAlphaCutoff, float(alphaCutoff));
        }

        {
            int shadingModel = 0;
            result = source.Get(AI_MATKEY_SHADING_MODEL, shadingModel);
            if (result == aiReturn_SUCCESS && shadingModel == aiShadingMode_Unlit)
                QSSGSceneDesc::setProperty(target, "lighting", &QQuick3DPrincipledMaterial::setLighting, QQuick3DPrincipledMaterial::Lighting::NoLighting);
        }


        {
            // Clearcoat Properties (KHR_materials_clearcoat)
            // factor
            {
                ai_real clearcoatFactor = 0.0f;
                result = source.Get(AI_MATKEY_CLEARCOAT_FACTOR, clearcoatFactor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target,
                                               "clearcoatAmount",
                                               &QQuick3DPrincipledMaterial::setClearcoatAmount,
                                               float(clearcoatFactor));
            }

            // roughness
            {
                ai_real clearcoatRoughnessFactor = 0.0f;
                result = source.Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, clearcoatRoughnessFactor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target,
                                               "clearcoatRoughnessAmount",
                                               &QQuick3DPrincipledMaterial::setClearcoatRoughnessAmount,
                                               float(clearcoatRoughnessFactor));
            }

            // texture
            if (auto clearcoatTexture = createTextureNode(source, AI_MATKEY_CLEARCOAT_TEXTURE))
                QSSGSceneDesc::setProperty(target, "clearcoatMap", &QQuick3DPrincipledMaterial::setClearcoatMap, clearcoatTexture);

            // roughness texture
            if (auto clearcoatRoughnessTexture = createTextureNode(source, AI_MATKEY_CLEARCOAT_ROUGHNESS_TEXTURE))
                QSSGSceneDesc::setProperty(target,
                                           "clearcoatRoughnessMap",
                                           &QQuick3DPrincipledMaterial::setClearcoatRoughnessMap,
                                           clearcoatRoughnessTexture);

            // normal texture
            if (auto clearcoatNormalTexture = createTextureNode(source, AI_MATKEY_CLEARCOAT_NORMAL_TEXTURE))
                QSSGSceneDesc::setProperty(target, "clearcoatNormalMap", &QQuick3DPrincipledMaterial::setClearcoatNormalMap, clearcoatNormalTexture);
        }

        {
            // Transmission Properties (KHR_materials_transmission)
            // factor
            {
                ai_real transmissionFactor = 0.0f;
                result = source.Get(AI_MATKEY_TRANSMISSION_FACTOR, transmissionFactor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target,
                                               "transmissionFactor",
                                               &QQuick3DPrincipledMaterial::setTransmissionFactor,
                                               float(transmissionFactor));
            }

            // texture
            {
                if (auto transmissionImage = createTextureNode(source, AI_MATKEY_TRANSMISSION_TEXTURE))
                    QSSGSceneDesc::setProperty(target,
                                               "transmissionMap",
                                               &QQuick3DPrincipledMaterial::setTransmissionMap,
                                               transmissionImage);
            }

        }

        {
            // Volume Properties (KHR_materials_volume) [only used with transmission]
            // thicknessFactor
            {
                ai_real thicknessFactor = 0.0f;
                result = source.Get(AI_MATKEY_VOLUME_THICKNESS_FACTOR, thicknessFactor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target, "thicknessFactor", &QQuick3DPrincipledMaterial::setThicknessFactor, float(thicknessFactor));
            }

            // thicknessMap
            {
                if (auto thicknessImage = createTextureNode(source, AI_MATKEY_VOLUME_THICKNESS_TEXTURE))
                    QSSGSceneDesc::setProperty(target, "thicknessMap", &QQuick3DPrincipledMaterial::setThicknessMap, thicknessImage);
            }

            // attenuationDistance
            {
                ai_real attenuationDistance = 0.0f;
                result = source.Get(AI_MATKEY_VOLUME_ATTENUATION_DISTANCE, attenuationDistance);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target,
                                               "attenuationDistance",
                                               &QQuick3DPrincipledMaterial::setAttenuationDistance,
                                               float(attenuationDistance));
            }

            // attenuationColor
            {
                aiColor3D attenuationColor;
                result = source.Get(AI_MATKEY_VOLUME_ATTENUATION_COLOR, attenuationColor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target,
                                               "attenuationColor",
                                               &QQuick3DPrincipledMaterial::setAttenuationColor,
                                               aiColorToQColor(attenuationColor));
            }
        }


        // KHR_materials_ior
        {
            ai_real ior = 0.0f;
            result = source.Get(AI_MATKEY_REFRACTI, ior);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target,
                                           "indexOfRefraction",
                                           &QQuick3DPrincipledMaterial::setIndexOfRefraction,
                                           float(ior));
        }

    } else if (type == QSSGSceneDesc::Material::RuntimeType::DefaultMaterial) { // Ver1
        int shadingModel = 0;
        auto material = &source;
        result = material->Get(AI_MATKEY_SHADING_MODEL, shadingModel);
        // lighting
        if (result == aiReturn_SUCCESS && (shadingModel == aiShadingMode_NoShading))
            QSSGSceneDesc::setProperty(target, "lighting", &QQuick3DDefaultMaterial::setLighting, QQuick3DDefaultMaterial::Lighting::NoLighting);

        if (auto diffuseMapTexture = createTextureNode(source, aiTextureType_DIFFUSE, 0)) {
            QSSGSceneDesc::setProperty(target, "diffuseMap", &QQuick3DDefaultMaterial::setDiffuseMap, diffuseMapTexture);
        } else {
            // For some reason the normal behavior is that either you have a diffuseMap[s] or a diffuse color
            // but no a mix of both... So only set the diffuse color if none of the diffuse maps are set:
            aiColor3D diffuseColor;
            result = material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "diffuseColor", &QQuick3DDefaultMaterial::setDiffuseColor, aiColorToQColor(diffuseColor));
        }

        if (auto emissiveTexture = createTextureNode(source, aiTextureType_EMISSIVE, 0))
            QSSGSceneDesc::setProperty(target, "emissiveMap", &QQuick3DDefaultMaterial::setEmissiveMap, emissiveTexture);

        // specularReflectionMap
        if (auto specularTexture = createTextureNode(source, aiTextureType_SPECULAR, 0))
            QSSGSceneDesc::setProperty(target, "specularMap", &QQuick3DDefaultMaterial::setSpecularMap, specularTexture);

        // opacity AI_MATKEY_OPACITY
        ai_real opacity;
        result = material->Get(AI_MATKEY_OPACITY, opacity);
        if (result == aiReturn_SUCCESS)
            QSSGSceneDesc::setProperty(target, "opacity", &QQuick3DDefaultMaterial::setOpacity, float(opacity));

        // opacityMap aiTextureType_OPACITY 0
        if (auto opacityTexture = createTextureNode(source, aiTextureType_OPACITY, 0))
            QSSGSceneDesc::setProperty(target, "opacityMap", &QQuick3DDefaultMaterial::setOpacityMap, opacityTexture);

        // bumpMap aiTextureType_HEIGHT 0
        if (auto bumpTexture = createTextureNode(source, aiTextureType_HEIGHT, 0)) {
            QSSGSceneDesc::setProperty(target, "bumpMap", &QQuick3DDefaultMaterial::setBumpMap, bumpTexture);
            // bumpAmount AI_MATKEY_BUMPSCALING
            ai_real bumpAmount;
            result = material->Get(AI_MATKEY_BUMPSCALING, bumpAmount);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "bumpAmount", &QQuick3DDefaultMaterial::setBumpAmount, float(bumpAmount));
        }

        // normalMap aiTextureType_NORMALS 0
        if (auto normalTexture = createTextureNode(source, aiTextureType_NORMALS, 0))
            QSSGSceneDesc::setProperty(target, "normalMap", &QQuick3DDefaultMaterial::setNormalMap, normalTexture);
    } else if (type == QSSGSceneDesc::Material::RuntimeType::SpecularGlossyMaterial) {
        {
            aiColor4D albedoFactor;
            result = source.Get(AI_MATKEY_COLOR_DIFFUSE, albedoFactor);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "albedoColor", &QQuick3DSpecularGlossyMaterial::setAlbedoColor, aiColorToQColor(albedoFactor));
        }

        if (auto albedoTexture = createTextureNode(source, aiTextureType_DIFFUSE, 0)) {
            QSSGSceneDesc::setProperty(target, "albedoMap", &QQuick3DSpecularGlossyMaterial::setAlbedoMap, albedoTexture);
            QSSGSceneDesc::setProperty(target, "opacityChannel", &QQuick3DSpecularGlossyMaterial::setOpacityChannel, QQuick3DSpecularGlossyMaterial::TextureChannelMapping::A);
        }

        if (auto specularGlossinessTexture = createTextureNode(source, aiTextureType_SPECULAR, 0)) {
            QSSGSceneDesc::setProperty(target, "specularMap", &QQuick3DSpecularGlossyMaterial::setSpecularMap, specularGlossinessTexture);
            QSSGSceneDesc::setProperty(target, "glossinessMap", &QQuick3DSpecularGlossyMaterial::setGlossinessMap, specularGlossinessTexture);
            QSSGSceneDesc::setProperty(target, "glossinessChannel", &QQuick3DSpecularGlossyMaterial::setGlossinessChannel, QQuick3DSpecularGlossyMaterial::TextureChannelMapping::A);
        }

        {
            aiColor4D specularColorFactor;
            result = source.Get(AI_MATKEY_COLOR_SPECULAR, specularColorFactor);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "specularColor", &QQuick3DSpecularGlossyMaterial::setSpecularColor, aiColorToQColor(specularColorFactor));
        }

        {
            ai_real glossinessFactor;
            result = source.Get(AI_MATKEY_GLOSSINESS_FACTOR, glossinessFactor);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "glossiness", &QQuick3DSpecularGlossyMaterial::setGlossiness, float(glossinessFactor));
        }

        if (auto normalTexture = createTextureNode(source, aiTextureType_NORMALS, 0)) {
            QSSGSceneDesc::setProperty(target, "normalMap", &QQuick3DSpecularGlossyMaterial::setNormalMap, normalTexture);
            {
                ai_real normalScale;
                result = source.Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), normalScale);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target, "normalStrength", &QQuick3DSpecularGlossyMaterial::setNormalStrength, float(normalScale));
            }
        }

        // Occlusion Textures are not implimented (yet)
        if (auto occlusionTexture = createTextureNode(source, aiTextureType_LIGHTMAP, 0)) {
            QSSGSceneDesc::setProperty(target, "occlusionMap", &QQuick3DSpecularGlossyMaterial::setOcclusionMap, occlusionTexture);
            QSSGSceneDesc::setProperty(target, "occlusionChannel", &QQuick3DSpecularGlossyMaterial::setOcclusionChannel, QQuick3DSpecularGlossyMaterial::TextureChannelMapping::R);
            {
                ai_real occlusionAmount;
                result = source.Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_LIGHTMAP, 0), occlusionAmount);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target, "occlusionAmount", &QQuick3DSpecularGlossyMaterial::setOcclusionAmount, float(occlusionAmount));
            }
        }

        if (auto emissiveTexture = createTextureNode(source, aiTextureType_EMISSIVE, 0))
            QSSGSceneDesc::setProperty(target, "emissiveMap", &QQuick3DSpecularGlossyMaterial::setEmissiveMap, emissiveTexture);

        {
            aiColor3D emissiveColorFactor;
            result = source.Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColorFactor);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "emissiveFactor", &QQuick3DSpecularGlossyMaterial::setEmissiveFactor, QVector3D { emissiveColorFactor.r, emissiveColorFactor.g, emissiveColorFactor.b });
        }

        {
            bool isDoubleSided;
            result = source.Get(AI_MATKEY_TWOSIDED, isDoubleSided);
            if (result == aiReturn_SUCCESS && isDoubleSided)
                QSSGSceneDesc::setProperty(target, "cullMode", &QQuick3DSpecularGlossyMaterial::setCullMode, QQuick3DSpecularGlossyMaterial::CullMode::NoCulling);
        }

        {
            aiString alphaMode;
            result = source.Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
            if (result == aiReturn_SUCCESS) {
                auto mode = QQuick3DSpecularGlossyMaterial::AlphaMode::Default;
                if (QByteArrayView(alphaMode.C_Str()) == "OPAQUE")
                    mode = QQuick3DSpecularGlossyMaterial::AlphaMode::Opaque;
                else if (QByteArrayView(alphaMode.C_Str()) == "MASK")
                    mode = QQuick3DSpecularGlossyMaterial::AlphaMode::Mask;
                else if (QByteArrayView(alphaMode.C_Str()) == "BLEND")
                    mode = QQuick3DSpecularGlossyMaterial::AlphaMode::Blend;

                if (mode != QQuick3DSpecularGlossyMaterial::AlphaMode::Default) {
                    QSSGSceneDesc::setProperty(target, "alphaMode", &QQuick3DSpecularGlossyMaterial::setAlphaMode, mode);
                    // If the mode is mask, we also need to force OpaquePrePassDepthDraw mode
                    if (mode == QQuick3DSpecularGlossyMaterial::AlphaMode::Mask)
                    QSSGSceneDesc::setProperty(target, "depthDrawMode", &QQuick3DSpecularGlossyMaterial::setDepthDrawMode, QQuick3DMaterial::OpaquePrePassDepthDraw);
                }
            }
        }

        {
            ai_real alphaCutoff;
            result = source.Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff);
            if (result == aiReturn_SUCCESS)
                QSSGSceneDesc::setProperty(target, "alphaCutoff", &QQuick3DSpecularGlossyMaterial::setAlphaCutoff, float(alphaCutoff));
        }

        {
            int shadingModel = 0;
            result = source.Get(AI_MATKEY_SHADING_MODEL, shadingModel);
            if (result == aiReturn_SUCCESS && shadingModel == aiShadingMode_Unlit)
                QSSGSceneDesc::setProperty(target, "lighting", &QQuick3DSpecularGlossyMaterial::setLighting, QQuick3DSpecularGlossyMaterial::Lighting::NoLighting);
        }


        {
            // Clearcoat Properties (KHR_materials_clearcoat)
            // factor
            {
                ai_real clearcoatFactor = 0.0f;
                result = source.Get(AI_MATKEY_CLEARCOAT_FACTOR, clearcoatFactor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target,
                                               "clearcoatAmount",
                                               &QQuick3DSpecularGlossyMaterial::setClearcoatAmount,
                                               float(clearcoatFactor));
            }

            // roughness
            {
                ai_real clearcoatRoughnessFactor = 0.0f;
                result = source.Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, clearcoatRoughnessFactor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target,
                                               "clearcoatRoughnessAmount",
                                               &QQuick3DSpecularGlossyMaterial::setClearcoatRoughnessAmount,
                                               float(clearcoatRoughnessFactor));
            }

            // texture
            if (auto clearcoatTexture = createTextureNode(source, AI_MATKEY_CLEARCOAT_TEXTURE))
                QSSGSceneDesc::setProperty(target, "clearcoatMap", &QQuick3DSpecularGlossyMaterial::setClearcoatMap, clearcoatTexture);

            // roughness texture
            if (auto clearcoatRoughnessTexture = createTextureNode(source, AI_MATKEY_CLEARCOAT_ROUGHNESS_TEXTURE))
                QSSGSceneDesc::setProperty(target,
                                           "clearcoatRoughnessMap",
                                           &QQuick3DSpecularGlossyMaterial::setClearcoatRoughnessMap,
                                           clearcoatRoughnessTexture);

            // normal texture
            if (auto clearcoatNormalTexture = createTextureNode(source, AI_MATKEY_CLEARCOAT_NORMAL_TEXTURE))
                QSSGSceneDesc::setProperty(target, "clearcoatNormalMap", &QQuick3DSpecularGlossyMaterial::setClearcoatNormalMap, clearcoatNormalTexture);
        }

        {
            // Transmission Properties (KHR_materials_transmission)
            // factor
            {
                ai_real transmissionFactor = 0.0f;
                result = source.Get(AI_MATKEY_TRANSMISSION_FACTOR, transmissionFactor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target,
                                               "transmissionFactor",
                                               &QQuick3DSpecularGlossyMaterial::setTransmissionFactor,
                                               float(transmissionFactor));
            }

            // texture
            {
                if (auto transmissionImage = createTextureNode(source, AI_MATKEY_TRANSMISSION_TEXTURE))
                    QSSGSceneDesc::setProperty(target,
                                               "transmissionMap",
                                               &QQuick3DSpecularGlossyMaterial::setTransmissionMap,
                                               transmissionImage);
            }

        }

        {
            // Volume Properties (KHR_materials_volume) [only used with transmission]
            // thicknessFactor
            {
                ai_real thicknessFactor = 0.0f;
                result = source.Get(AI_MATKEY_VOLUME_THICKNESS_FACTOR, thicknessFactor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target, "thicknessFactor", &QQuick3DSpecularGlossyMaterial::setThicknessFactor, float(thicknessFactor));
            }

            // thicknessMap
            {
                if (auto thicknessImage = createTextureNode(source, AI_MATKEY_VOLUME_THICKNESS_TEXTURE))
                    QSSGSceneDesc::setProperty(target, "thicknessMap", &QQuick3DSpecularGlossyMaterial::setThicknessMap, thicknessImage);
            }

            // attenuationDistance
            {
                ai_real attenuationDistance = 0.0f;
                result = source.Get(AI_MATKEY_VOLUME_ATTENUATION_DISTANCE, attenuationDistance);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target,
                                               "attenuationDistance",
                                               &QQuick3DSpecularGlossyMaterial::setAttenuationDistance,
                                               float(attenuationDistance));
            }

            // attenuationColor
            {
                aiColor3D attenuationColor;
                result = source.Get(AI_MATKEY_VOLUME_ATTENUATION_COLOR, attenuationColor);
                if (result == aiReturn_SUCCESS)
                    QSSGSceneDesc::setProperty(target,
                                               "attenuationColor",
                                               &QQuick3DSpecularGlossyMaterial::setAttenuationColor,
                                               aiColorToQColor(attenuationColor));
            }
        }
    }
}

static void setCameraProperties(QSSGSceneDesc::Camera &target, const aiCamera &source, const aiNode &sourceNode, const SceneInfo &sceneInfo)
{
    using namespace QSSGSceneDesc;

    // assimp does not have a camera type but it works for gltf2 format.
    target.runtimeType = (source.mHorizontalFOV == 0.0f) ? Node::RuntimeType::OrthographicCamera
                                                         : Node::RuntimeType::PerspectiveCamera;

    // We assume these default forward and up vectors, so if this isn't
    // the case we have to do additional transform
    aiMatrix4x4 correctionMatrix;
    bool needsCorrection = false;

    // Workaround For FBX,
    // assimp has a problem to set properties, mLookAt ans mUp
    // and it takes too much time for correction.
    // Quick3D will ignore these value and just use
    // the initial differences between FBX and Quick3D.
    if (sceneInfo.opt.fbxMode) {
        aiMatrix4x4::RotationY(ai_real(M_PI / 2), correctionMatrix);
        needsCorrection = true;
    } else {
        aiVector3D upQuick3D = aiVector3D(0, 1, 0);
        if (source.mLookAt != aiVector3D(0, 0, -1)) {
            aiMatrix4x4 lookAtCorrection;
            aiMatrix4x4::FromToMatrix(aiVector3D(0, 0, -1), source.mLookAt, lookAtCorrection);
            correctionMatrix *= lookAtCorrection;
            needsCorrection = true;
            upQuick3D *= lookAtCorrection;
        }
        if (source.mUp != upQuick3D) {
            aiMatrix4x4 upCorrection;
            aiMatrix4x4::FromToMatrix(upQuick3D, source.mUp, upCorrection);
            correctionMatrix = upCorrection * correctionMatrix;
            needsCorrection = true;
        }
    }

    setNodeProperties(target, sourceNode, sceneInfo, needsCorrection ? &correctionMatrix : nullptr);

    // clipNear and clipFar
    if (target.runtimeType == Node::RuntimeType::PerspectiveCamera) {
        setProperty(target, "clipNear", &QQuick3DPerspectiveCamera::setClipNear, source.mClipPlaneNear);
        setProperty(target, "clipFar", &QQuick3DPerspectiveCamera::setClipFar, source.mClipPlaneFar);
    } else { //OrthographicCamera
        setProperty(target, "clipNear", &QQuick3DOrthographicCamera::setClipNear, source.mClipPlaneNear);
        setProperty(target, "clipFar", &QQuick3DOrthographicCamera::setClipFar, source.mClipPlaneFar);
    }

    if (target.runtimeType == Node::RuntimeType::PerspectiveCamera) {
        // fieldOfView
        // mHorizontalFOV is defined as a half horizontal fov
        // in the assimp header but it seems not half now.
        const float fov = qRadiansToDegrees(source.mHorizontalFOV);
        setProperty(target, "fieldOfView", &QQuick3DPerspectiveCamera::setFieldOfView, fov);

        // isFieldOfViewHorizontal
        setProperty(target, "fieldOfViewOrientation", &QQuick3DPerspectiveCamera::setFieldOfViewOrientation,
                    QQuick3DPerspectiveCamera::FieldOfViewOrientation::Horizontal);
    } else { //OrthographicCamera
        const float width = source.mOrthographicWidth * 2.0f;
        const float height = width / source.mAspect;
        setProperty(target, "horizontalMagnification", &QQuick3DOrthographicCamera::setHorizontalMagnification, width);
        setProperty(target, "verticalMagnification", &QQuick3DOrthographicCamera::setVerticalMagnification, height);
    }
    // projectionMode

    // scaleMode

    // scaleAnchor

    // frustomScaleX

    // frustomScaleY
}

static void setLightProperties(QSSGSceneDesc::Light &target, const aiLight &source, const aiNode &sourceNode, const SceneInfo &sceneInfo)
{
    // We assume that the direction vector for a light is (0, 0, -1)
    // so if the direction vector is non-null, but not (0, 0, -1) we
    // need to correct the translation
    aiMatrix4x4 correctionMatrix;
    bool needsCorrection = false;
    if (source.mDirection != aiVector3D(0, 0, 0)) {
        if (source.mDirection != aiVector3D(0, 0, -1)) {
            aiMatrix4x4::FromToMatrix(aiVector3D(0, 0, -1), source.mDirection, correctionMatrix);
            needsCorrection = true;
        }
    }

    // lightType
    static const auto asQtLightType = [](aiLightSourceType type) {
        switch (type) {
        case aiLightSource_AMBIENT:
            Q_FALLTHROUGH();
        case aiLightSource_DIRECTIONAL:
            return QSSGSceneDesc::Light::RuntimeType::DirectionalLight;
        case aiLightSource_POINT:
            return QSSGSceneDesc::Light::RuntimeType::PointLight;
        case aiLightSource_SPOT:
            return QSSGSceneDesc::Light::RuntimeType::SpotLight;
        default:
            return QSSGSceneDesc::Light::RuntimeType::PointLight;
        }
    };

    target.runtimeType = asQtLightType(source.mType);

    setNodeProperties(target, sourceNode, sceneInfo, needsCorrection ? &correctionMatrix : nullptr);

    // brightness
    // Assimp has no property related to brightness or intensity.
    // They are multiplied to diffuse, ambient and specular colors.
    // For extracting the property value, we will check the maximum value of them.
    // (In most cases, Assimp uses the same specular values with diffuse values,
    // so we will compare just components of the diffuse and the ambient)
    float brightness = qMax(qMax(1.0f, source.mColorDiffuse.r),
                            qMax(source.mColorDiffuse.g, source.mColorDiffuse.b));

    // ambientColor
    if (source.mType == aiLightSource_AMBIENT) {
        brightness = qMax(qMax(brightness, source.mColorAmbient.r),
                          qMax(source.mColorAmbient.g, source.mColorAmbient.b));

        // We only want ambient light color if it is explicit
        const QColor ambientColor = QColor::fromRgbF(source.mColorAmbient.r / brightness,
                                                     source.mColorAmbient.g / brightness,
                                                     source.mColorAmbient.b / brightness);
        QSSGSceneDesc::setProperty(target, "ambientColor", &QQuick3DAbstractLight::setAmbientColor, ambientColor);
    }

    // diffuseColor
    const QColor diffuseColor = QColor::fromRgbF(source.mColorDiffuse.r / brightness,
                                                 source.mColorDiffuse.g / brightness,
                                                 source.mColorDiffuse.b / brightness);
    QSSGSceneDesc::setProperty(target, "color", &QQuick3DAbstractLight::setColor, diffuseColor);

    // describe brightness here
    QSSGSceneDesc::setProperty(target, "brightness", &QQuick3DAbstractLight::setBrightness, brightness);

    const bool isSpot = (source.mType == aiLightSource_SPOT);
    if (source.mType == aiLightSource_POINT || isSpot) {
        // constantFade
        // Some assets have this constant attenuation value as 0.0f and it makes light attenuation makes infinite at distance 0.
        // In that case, we will use the default constant attenuation, 1.0f.
        const bool hasAttConstant = !qFuzzyIsNull(source.mAttenuationConstant);

        if (isSpot) {
            if (hasAttConstant)
                QSSGSceneDesc::setProperty(target, "constantFade", &QQuick3DSpotLight::setConstantFade, source.mAttenuationConstant);
            QSSGSceneDesc::setProperty(target, "linearFade", &QQuick3DSpotLight::setLinearFade, source.mAttenuationLinear * 100.0f);
            QSSGSceneDesc::setProperty(target, "quadraticFade", &QQuick3DSpotLight::setQuadraticFade, source.mAttenuationQuadratic * 10000.0f);
            QSSGSceneDesc::setProperty(target, "coneAngle", &QQuick3DSpotLight::setConeAngle, qRadiansToDegrees(source.mAngleOuterCone) * 2.0f);
            QSSGSceneDesc::setProperty(target, "innerConeAngle", &QQuick3DSpotLight::setInnerConeAngle, qRadiansToDegrees(source.mAngleInnerCone) * 2.0f);
        } else {
            if (hasAttConstant)
                QSSGSceneDesc::setProperty(target, "constantFade", &QQuick3DPointLight::setConstantFade, source.mAttenuationConstant);
            QSSGSceneDesc::setProperty(target, "linearFade", &QQuick3DPointLight::setLinearFade, source.mAttenuationLinear * 100.0f);
            QSSGSceneDesc::setProperty(target, "quadraticFade", &QQuick3DPointLight::setQuadraticFade, source.mAttenuationQuadratic * 10000.0f);
        }
    }
    // castShadow

    // shadowBias

    // shadowFactor

    // shadowMapResolution

    // shadowMapFar

    // shadowMapFieldOfView

    // shadowFilter
}

using MorphAttributes = QQuick3DMorphTarget::MorphTargetAttributes;
struct MorphProperty {
    QByteArray name;
    MorphAttributes attrib;
    float weight;
};

static void setModelProperties(QSSGSceneDesc::Model &target, const aiNode &source, const SceneInfo &sceneInfo)
{
    if (source.mNumMeshes == 0)
        return;

    auto &targetScene = target.scene;
    const auto &srcScene = sceneInfo.scene;
    // TODO: Correction and scale
    setNodeProperties(target, source, sceneInfo, nullptr);

    auto &meshStorage = targetScene->meshStorage;
    auto &materialMap = sceneInfo.materialMap;
    auto &meshMap = sceneInfo.meshMap;
    auto &skinMap = sceneInfo.skinMap;
    auto &mesh2skin = sceneInfo.mesh2skin;

    QVarLengthArray<QSSGSceneDesc::Material *> materials;
    materials.reserve(source.mNumMeshes); // Assumig there's max one material per mesh.

    QString errorString;

    const auto ensureMaterial = [&](qsizetype materialIndex) {
        // Get the material for the mesh
        auto &material = materialMap[materialIndex];
        // Check if we need to create a new scene node for this material
        auto targetMat = material.second;
        if (targetMat == nullptr) {
            const aiMaterial *sourceMat = material.first;

            auto currentMaterialType = QSSGSceneDesc::Material::RuntimeType::PrincipledMaterial;
            ai_real glossinessFactor;
            aiReturn result = sourceMat->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossinessFactor);
            if (result == aiReturn_SUCCESS)
                currentMaterialType = QSSGSceneDesc::Material::RuntimeType::SpecularGlossyMaterial;

            targetMat = new QSSGSceneDesc::Material(currentMaterialType);
            QSSGSceneDesc::addNode(target, *targetMat);
            setMaterialProperties(*targetMat, *sourceMat, sceneInfo, currentMaterialType);
            material.second = targetMat;
        }

        Q_ASSERT(targetMat != nullptr && material.second != nullptr);
        // If these don't match then somethings broken...
        Q_ASSERT(srcScene.mMaterials[materialIndex] == material.first);
        materials.push_back(targetMat);
    };

    AssimpUtils::MeshList meshes;
    qint16 skinIdx = -1;
    // Combine all the meshes referenced by this model into a single MultiMesh file
    // For the morphing, the target mesh must have the same AnimMeshes.
    // It means if only one mesh has a morphing animation, the other sub-meshes will
    // get null target attributes. However this case might not be common.
    // These submeshes will animate with the same morphing weight!

    // If meshes have separate skins, they should not be combined. GLTF2 does not
    // seem to have problems related with this case, but When we use runtime asset
    // for other formats, this case must be checked again.
    // Here, we will use only the first skin in the mesh list
    const auto combineMeshes = [&](const aiNode &source, aiMesh **sceneMeshes) {
        for (qsizetype i = 0, end = source.mNumMeshes; i != end; ++i) {
            const aiMesh &mesh = *sceneMeshes[source.mMeshes[i]];
            ensureMaterial(mesh.mMaterialIndex);
            if (skinIdx == -1 && mesh.HasBones())
                skinIdx = mesh2skin[source.mMeshes[i]];
            meshes.push_back(&mesh);
        }
    };

    const auto createMeshNode = [&](const aiString &name) {
        auto meshData = AssimpUtils::generateMeshData(srcScene,
                                                      meshes,
                                                      sceneInfo.opt.useFloatJointIndices,
                                                      sceneInfo.opt.generateMeshLODs,
                                                      sceneInfo.opt.lodNormalMergeAngle,
                                                      sceneInfo.opt.lodNormalSplitAngle,
                                                      errorString);
        meshStorage.push_back(std::move(meshData));

        const auto idx = meshStorage.size() - 1;
        // For multimeshes we'll use the model name, but for single meshes we'll use the mesh name.
        return new QSSGSceneDesc::Mesh(fromAiString(name), idx);
    };

    QSSGSceneDesc::Mesh *meshNode = nullptr;

    const bool isMultiMesh = (source.mNumMeshes > 1);
    if (isMultiMesh) {
        // result is stored in 'meshes'
        combineMeshes(source, srcScene.mMeshes);
        Q_ASSERT(!meshes.isEmpty());
        meshNode = createMeshNode(source.mName);
        QSSGSceneDesc::addNode(target, *meshNode);
    } else { // single mesh (We shouldn't be here if there are no meshes...)
        Q_ASSERT(source.mNumMeshes == 1);
        auto &mesh = meshMap[*source.mMeshes];
        meshNode = mesh.second;
        if (meshNode == nullptr) {
            meshes = {mesh.first};
            if (mesh.first->HasBones())
                skinIdx = mesh2skin[*source.mMeshes];
            mesh.second = meshNode = createMeshNode(mesh.first->mName);
            QSSGSceneDesc::addNode(target, *meshNode); // We only add this the first time we create it.
        }
        ensureMaterial(mesh.first->mMaterialIndex);
        Q_ASSERT(meshNode != nullptr && mesh.second != nullptr);
    }

    if (meshNode)
        QSSGSceneDesc::setProperty(target, "source", &QQuick3DModel::setSource, QVariant::fromValue(meshNode));

    if (skinIdx != -1) {
        auto &skin = skinMap[skinIdx];
        skin.node = new QSSGSceneDesc::Skin;
        QSSGSceneDesc::setProperty(target, "skin", &QQuick3DModel::setSkin, skin.node);
        QSSGSceneDesc::addNode(target, *skin.node);
        // Skins' properties wil be set after all the nodes are processed
    }

    // materials
    // Note that we use a QVector/QList here instead of a QQmlListProperty, as that would be really inconvenient.
    // Since we don't create any runtime objects at this point, the list also contains the node type that corresponds with the
    // type expected to be in the list (this is ensured at compile-time).
    QSSGSceneDesc::setProperty(target, "materials", &QQuick3DModel::materials, materials);
}

static QSSGSceneDesc::Node *createSceneNode(const NodeInfo &nodeInfo,
                                            const aiNode &srcNode,
                                            QSSGSceneDesc::Node &parent,
                                            const SceneInfo &sceneInfo)
{
    QSSGSceneDesc::Node *node = nullptr;
    const auto &srcScene = sceneInfo.scene;
    switch (nodeInfo.type) {
    case QSSGSceneDesc::Node::Type::Camera:
    {
        const auto &srcType = *srcScene.mCameras[nodeInfo.index];
        // We set the initial rt-type to 'Custom', but we'll change it when updateing the properties.
        auto targetType = new QSSGSceneDesc::Camera(QSSGSceneDesc::Node::RuntimeType::CustomCamera);
        QSSGSceneDesc::addNode(parent, *targetType);
        setCameraProperties(*targetType, srcType, srcNode, sceneInfo);
        node = targetType;
    }
        break;
    case QSSGSceneDesc::Node::Type::Light:
    {
        const auto &srcType = *srcScene.mLights[nodeInfo.index];
        // Initial type is DirectonalLight, but will be change (if needed) when setting the properties.
        auto targetType = new QSSGSceneDesc::Light(QSSGSceneDesc::Node::RuntimeType::DirectionalLight);
        QSSGSceneDesc::addNode(parent, *targetType);
        setLightProperties(*targetType, srcType, srcNode, sceneInfo);
        node = targetType;
    }
        break;
    case QSSGSceneDesc::Node::Type::Model:
    {
        auto target = new QSSGSceneDesc::Model;
        QSSGSceneDesc::addNode(parent, *target);
        setModelProperties(*target, srcNode, sceneInfo);
        node = target;
    }
        break;
    case QSSGSceneDesc::Node::Type::Joint:
    {
        auto target = new QSSGSceneDesc::Joint;
        QSSGSceneDesc::addNode(parent, *target);
        setNodeProperties(*target, srcNode, sceneInfo, nullptr);
        QSSGSceneDesc::setProperty(*target, "index", &QQuick3DJoint::setIndex, qint32(nodeInfo.index));
        node = target;
    }
        break;
    case QSSGSceneDesc::Node::Type::Transform:
    {
        node = new QSSGSceneDesc::Node(QSSGSceneDesc::Node::Type::Transform, QSSGSceneDesc::Node::RuntimeType::Node);
        QSSGSceneDesc::addNode(parent, *node);
        // TODO: arguments for correction
        setNodeProperties(*node, srcNode, sceneInfo, nullptr);
    }
        break;
    default:
        break;
    }

    return node;
}

static void processNode(const SceneInfo &sceneInfo, const aiNode &source, QSSGSceneDesc::Node &parent, const NodeMap &nodeMap, AnimationNodeMap &animationNodes)
{
    QSSGSceneDesc::Node *node = nullptr;
    if (source.mNumMeshes != 0) {
        // Process morphTargets first and then add them to the modelNode
        using It = decltype(source.mNumMeshes);
        QVarLengthArray<MorphProperty> morphProps;
        for (It i = 0, end = source.mNumMeshes; i != end; ++i) {
            const auto &srcScene = sceneInfo.scene;
            const aiMesh &mesh = *srcScene.mMeshes[source.mMeshes[i]];
            const quint32 numMorphTargets = mesh.mNumAnimMeshes;
            if (numMorphTargets && mesh.mAnimMeshes) {
                morphProps.reserve(numMorphTargets);
                for (uint j = 0; j < numMorphTargets; ++j) {
                    const auto &animMesh = mesh.mAnimMeshes[j];
                    MorphAttributes attrib;
                    if (animMesh->HasPositions())
                        attrib |= QQuick3DMorphTarget::MorphTargetAttribute::Position;
                    if (animMesh->HasNormals())
                        attrib |= QQuick3DMorphTarget::MorphTargetAttribute::Normal;
                    if (animMesh->HasTangentsAndBitangents()) {
                        attrib |= QQuick3DMorphTarget::MorphTargetAttribute::Tangent;
                        attrib |= QQuick3DMorphTarget::MorphTargetAttribute::Binormal;
                    }
                    morphProps.append({ fromAiString(animMesh->mName),
                                           attrib,
                                           animMesh->mWeight });
                }
            }
        }
        node = createSceneNode(NodeInfo { 0, QSSGSceneDesc::Node::Type::Model }, source, parent, sceneInfo);
        if (!morphProps.isEmpty()) {
            const QString nodeName(source.mName.C_Str());
            QVarLengthArray<QSSGSceneDesc::MorphTarget *> morphTargets;
            morphTargets.reserve(morphProps.size());
            for (int i = 0, end = morphProps.size(); i != end; ++i) {
                const auto morphProp = morphProps.at(i);

                auto morphNode = new QSSGSceneDesc::MorphTarget;
                QSSGSceneDesc::addNode(*node, *morphNode);
                QSSGSceneDesc::setProperty(*morphNode, "weight", &QQuick3DMorphTarget::setWeight, morphProp.weight);
                QSSGSceneDesc::setProperty(*morphNode, "attributes", &QQuick3DMorphTarget::setAttributes, morphProp.attrib);
                morphNode->name = morphProp.name;
                morphTargets.push_back(morphNode);

                if (!animationNodes.isEmpty()) {
                    QString morphTargetName = nodeName + QStringLiteral("_morph") + QString::number(i);
                    const auto aNodeIt = animationNodes.find(morphTargetName.toUtf8());
                    if (aNodeIt != animationNodes.end() && aNodeIt.value() == nullptr)
                        *aNodeIt = morphNode;
                }
            }
            QSSGSceneDesc::setProperty(*node, "morphTargets", &QQuick3DModel::morphTargets, morphTargets);
        }
    }

    if (!node) {
        NodeInfo nodeInfo{ 0, QSSGSceneDesc::Node::Type::Transform };
        if (auto it = nodeMap.constFind(&source); it != nodeMap.constEnd())
            nodeInfo = (*it);
        node = createSceneNode(nodeInfo, source, parent, sceneInfo);
    }

    if (!node)
        node = &parent;

    Q_ASSERT(node->scene);

    // Check if this node is a target for an animation
    if (!animationNodes.isEmpty()) {
        const auto &nodeName = source.mName;
        auto aNodeIt = animationNodes.find(QByteArray{nodeName.C_Str(), qsizetype(nodeName.length)});
        if (aNodeIt != animationNodes.end() && aNodeIt.value() == nullptr)
            *aNodeIt = node;
    }

    // Process child nodes
    using It = decltype (source.mNumChildren);
    for (It i = 0, end = source.mNumChildren; i != end; ++i)
        processNode(sceneInfo, **(source.mChildren + i), *node, nodeMap, animationNodes);
}

static QSSGSceneDesc::Animation::KeyPosition toAnimationKey(const aiVectorKey &key, qreal freq) {
    const auto flag = quint16(QSSGSceneDesc::Animation::KeyPosition::KeyType::Time) | quint16(QSSGSceneDesc::Animation::KeyPosition::ValueType::Vec3);
    return QSSGSceneDesc::Animation::KeyPosition { QVector4D{ key.mValue.x, key.mValue.y, key.mValue.z, 0.0f }, float(key.mTime * freq), flag };
}

static QSSGSceneDesc::Animation::KeyPosition toAnimationKey(const aiQuatKey &key, qreal freq) {
    const auto flag = quint16(QSSGSceneDesc::Animation::KeyPosition::KeyType::Time) | quint16(QSSGSceneDesc::Animation::KeyPosition::ValueType::Quaternion);
    return QSSGSceneDesc::Animation::KeyPosition { QVector4D{ key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w }, float(key.mTime * freq), flag };
}

static QSSGSceneDesc::Animation::KeyPosition toAnimationKey(const aiMeshMorphKey &key, qreal freq, uint morphId) {
    const auto flag = quint16(QSSGSceneDesc::Animation::KeyPosition::KeyType::Time) | quint16(QSSGSceneDesc::Animation::KeyPosition::ValueType::Number);
    return QSSGSceneDesc::Animation::KeyPosition { QVector4D{ float(key.mWeights[morphId]), 0.0f, 0.0f, 0.0f }, float(key.mTime * freq), flag };
}

static bool checkBooleanOption(const QString &optionName, const QJsonObject &options)
{
    const auto it = options.constFind(optionName);
    const auto end = options.constEnd();
    QJsonValue value;
    if (it != end) {
        if (it->isObject())
            value = it->toObject().value("value");
        else
            value = it.value();
    }
    return value.toBool();
}

static qreal getRealOption(const QString &optionName, const QJsonObject &options)
{
    const auto it = options.constFind(optionName);
    const auto end = options.constEnd();
    QJsonValue value;
    if (it != end) {
        if (it->isObject())
            value = it->toObject().value("value");
        else
            value = it.value();
    }

    return value.toDouble();
}

#define demonPostProcessPresets ( \
    aiProcess_CalcTangentSpace              |  \
    aiProcess_JoinIdenticalVertices         |  \
    aiProcess_ImproveCacheLocality          |  \
    aiProcess_RemoveRedundantMaterials      |  \
    aiProcess_SplitLargeMeshes              |  \
    aiProcess_Triangulate                   |  \
    aiProcess_GenUVCoords                   |  \
    aiProcess_SortByPType                   |  \
    aiProcess_FindDegenerates               |  \
    aiProcess_FindInvalidData               |  \
    0 )

static aiPostProcessSteps processOptions(const QJsonObject &optionsObject, std::unique_ptr<Assimp::Importer> &importer) {
    aiPostProcessSteps postProcessSteps = aiPostProcessSteps(aiProcess_Triangulate | aiProcess_SortByPType);;

    // Setup import settings based given options
    // You can either pass the whole options object, or just the "options" object
    // so get the right scope.
    QJsonObject options = optionsObject;

    if (auto it = options.constFind("options"), end = options.constEnd(); it != end)
        options = it->toObject();

    if (options.isEmpty())
        return postProcessSteps;

    // parse the options list for values

    if (checkBooleanOption(QStringLiteral("calculateTangentSpace"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_CalcTangentSpace);

    if (checkBooleanOption(QStringLiteral("joinIdenticalVertices"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_JoinIdenticalVertices);

    if (checkBooleanOption(QStringLiteral("generateNormals"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_GenNormals);

    if (checkBooleanOption(QStringLiteral("generateSmoothNormals"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_GenSmoothNormals);

    if (checkBooleanOption(QStringLiteral("splitLargeMeshes"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_SplitLargeMeshes);

    if (checkBooleanOption(QStringLiteral("preTransformVertices"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_PreTransformVertices);

    if (checkBooleanOption(QStringLiteral("improveCacheLocality"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_ImproveCacheLocality);

    if (checkBooleanOption(QStringLiteral("removeRedundantMaterials"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_RemoveRedundantMaterials);

    if (checkBooleanOption(QStringLiteral("fixInfacingNormals"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_FixInfacingNormals);

    if (checkBooleanOption(QStringLiteral("findDegenerates"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_FindDegenerates);

    if (checkBooleanOption(QStringLiteral("findInvalidData"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_FindInvalidData);

    if (checkBooleanOption(QStringLiteral("transformUVCoordinates"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_TransformUVCoords);

    if (checkBooleanOption(QStringLiteral("findInstances"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_FindInstances);

    if (checkBooleanOption(QStringLiteral("optimizeMeshes"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_OptimizeMeshes);

    if (checkBooleanOption(QStringLiteral("optimizeGraph"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_OptimizeGraph);

    if (checkBooleanOption(QStringLiteral("dropNormals"), options))
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_DropNormals);

    aiComponent removeComponents = aiComponent(0);

    if (checkBooleanOption(QStringLiteral("removeComponentNormals"), options))
        removeComponents = aiComponent(removeComponents | aiComponent_NORMALS);

    if (checkBooleanOption(QStringLiteral("removeComponentTangentsAndBitangents"), options))
        removeComponents = aiComponent(removeComponents | aiComponent_TANGENTS_AND_BITANGENTS);

    if (checkBooleanOption(QStringLiteral("removeComponentColors"), options))
        removeComponents = aiComponent(removeComponents | aiComponent_COLORS);

    if (checkBooleanOption(QStringLiteral("removeComponentUVs"), options))
        removeComponents = aiComponent(removeComponents | aiComponent_TEXCOORDS);

    if (checkBooleanOption(QStringLiteral("removeComponentBoneWeights"), options))
        removeComponents = aiComponent(removeComponents | aiComponent_BONEWEIGHTS);

    if (checkBooleanOption(QStringLiteral("removeComponentAnimations"), options))
        removeComponents = aiComponent(removeComponents | aiComponent_ANIMATIONS);

    if (checkBooleanOption(QStringLiteral("removeComponentTextures"), options))
        removeComponents = aiComponent(removeComponents | aiComponent_TEXTURES);

    if (removeComponents != aiComponent(0)) {
        postProcessSteps = aiPostProcessSteps(postProcessSteps | aiProcess_RemoveComponent);
        importer->SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeComponents);
    }

    bool preservePivots = checkBooleanOption(QStringLiteral("fbxPreservePivots"), options);
    importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, preservePivots);

    return postProcessSteps;
}

static SceneInfo::Options processSceneOptions(const QJsonObject &optionsObject) {
    SceneInfo::Options sceneOptions;

    // Setup import settings based given options
    // You can either pass the whole options object, or just the "options" object
    // so get the right scope.
    QJsonObject options = optionsObject;

    if (auto it = options.constFind("options"), end = options.constEnd(); it != end)
        options = it->toObject();

    if (options.isEmpty())
        return sceneOptions;

    if (checkBooleanOption(QStringLiteral("globalScale"), options)) {
        sceneOptions.globalScaleValue = getRealOption(QStringLiteral("globalScaleValue"), options);
        if (sceneOptions.globalScaleValue == 0.0)
            sceneOptions.globalScaleValue = 1.0;
    }

    sceneOptions.designStudioWorkarounds = checkBooleanOption(QStringLiteral("designStudioWorkarounds"), options);
    sceneOptions.useFloatJointIndices = checkBooleanOption(QStringLiteral("useFloatJointIndices"), options);
    sceneOptions.forceMipMapGeneration = checkBooleanOption(QStringLiteral("generateMipMaps"), options);
    sceneOptions.binaryKeyframes = checkBooleanOption(QStringLiteral("useBinaryKeyframes"), options);
    sceneOptions.generateMeshLODs = checkBooleanOption(QStringLiteral("generateMeshLevelsOfDetail"), options);
    if (sceneOptions.generateMeshLODs) {
        bool recalculateLODNormals = checkBooleanOption(QStringLiteral("recalculateLodNormals"), options);
        if (recalculateLODNormals) {
            qreal mergeAngle = getRealOption(QStringLiteral("recalculateLodNormalsMergeAngle"), options);
            sceneOptions.lodNormalMergeAngle = qBound(0.0, mergeAngle, 270.0);
            qreal splitAngle = getRealOption(QStringLiteral("recalculateLodNormalsSplitAngle"), options);
            sceneOptions.lodNormalSplitAngle = qBound(0.0, splitAngle, 270.0);
        } else {
            sceneOptions.lodNormalMergeAngle = 0.0;
            sceneOptions.lodNormalSplitAngle = 0.0;
        }
    }
    return sceneOptions;
}

static QString importImp(const QUrl &url, const QJsonObject &options, QSSGSceneDesc::Scene &targetScene)
{
    auto filePath = url.path();

    const bool maybeLocalFile = QQmlFile::isLocalFile(url);
    if (maybeLocalFile && !QFileInfo::exists(filePath))
        filePath = QQmlFile::urlToLocalFileOrQrc(url);

    auto sourceFile = QFileInfo(filePath);
    if (!sourceFile.exists())
        return QLatin1String("File not found");
    targetScene.sourceDir = sourceFile.path();

    std::unique_ptr<Assimp::Importer> importer(new Assimp::Importer());

    // Setup import from Options
    aiPostProcessSteps postProcessSteps;
    if (options.isEmpty())
        postProcessSteps = aiPostProcessSteps(demonPostProcessPresets);
    else
        postProcessSteps = processOptions(options, importer);

    // Remove primitives that are not Triangles
    importer->SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
    importer->SetPropertyInteger(AI_CONFIG_IMPORT_COLLADA_USE_COLLADA_NAMES, 1);

    if (filePath.startsWith(":"))
        importer->SetIOHandler(new ResourceIOSystem);
#ifdef Q_OS_WASM
    // Always use QFile-based IO on wasm, so that URL schemes like
    // weblocalfile: are handled by Qt's file engine abstraction.
    else
        importer->SetIOHandler(new ResourceIOSystem);
#endif

    auto sourceScene = importer->ReadFile(filePath.toStdString(), postProcessSteps);
    if (!sourceScene) {
        // Scene failed to load, use logger to get the reason
        return QString::fromLocal8Bit(importer->GetErrorString());
    }

    // For simplicity, and convenience, we'll just use the file path as the id.
    // DO NOT USE it for anything else, once the scene is created there's no
    // real connection to the source asset file.
    targetScene.id = sourceFile.canonicalFilePath();

    // Assuming consistent type usage
    using It = decltype(sourceScene->mNumMeshes);

    // Before we can start processing the scene we start my mapping out the nodes
    // we can tell the type of.
    const auto &srcRootNode = *sourceScene->mRootNode;
    NodeMap nodeMap;
    // We need to know which nodes are animated so we can map _our_ animation data to
    // the target node (in Assimp this is string based mapping).
    AnimationNodeMap animatingNodes;
    {
        if (sourceScene->HasLights()) {
            for (It i = 0, end = sourceScene->mNumLights; i != end; ++i) {
                const auto &type = *sourceScene->mLights[i];
                if (auto node = srcRootNode.FindNode(type.mName))
                    nodeMap[node] = { i, NodeInfo::Type::Light };
            }
        }

        if (sourceScene->HasCameras()) {
            for (It i = 0, end = sourceScene->mNumCameras; i != end; ++i) {
                const auto &srcCam = *sourceScene->mCameras[i];
                if (auto node = srcRootNode.FindNode(srcCam.mName))
                    nodeMap[node] = { i, NodeInfo::Type::Camera };
            }
        }

        if (sourceScene->HasAnimations()) {
            for (It i = 0, end = sourceScene->mNumAnimations; i != end; ++i) {
                const auto &srcAnim = *sourceScene->mAnimations[i];
                const auto channelCount = srcAnim.mNumChannels;
                for (It cIdx = 0; cIdx != channelCount; ++cIdx) {
                    const auto &srcChannel = srcAnim.mChannels[cIdx];
                    const auto &nodeName = srcChannel->mNodeName;
                    if (nodeName.length > 0) {
                        // We'll update this once we've created the node!
                        QByteArray name(nodeName.C_Str(), qsizetype(nodeName.length));
                        if (!animatingNodes.contains(name))
                            animatingNodes.insert(name, nullptr);
                    }
                }
                const auto morphChannelCount = srcAnim.mNumMorphMeshChannels;
                for (It cIdx = 0; cIdx != morphChannelCount; ++cIdx) {
                    const auto &srcChannel = srcAnim.mMorphMeshChannels[cIdx];
                    const auto &nodeName = srcChannel->mName;
                    if (nodeName.length > 0) {
                        const auto morphKeys = srcChannel->mKeys;
                        const auto numMorphTargets = morphKeys[0].mNumValuesAndWeights;
                        // MorphTarget's animations are stored with a name,
                        // "<nodeName> + '_morph' + <targetNumber>"
                        for (It j = 0; j < numMorphTargets; ++j) {
                            QString morphTargetName(nodeName.C_Str());
                            morphTargetName += QStringLiteral("_morph") + QString::number(j);
                            animatingNodes.insert(morphTargetName.toUtf8(), nullptr);
                        }
                    }
                }
            }
        }
    }

    // We'll use these to ensure we don't re-create resources.
    const auto materialCount = sourceScene->mNumMaterials;
    SceneInfo::MaterialMap materials;
    materials.reserve(materialCount);

    const auto meshCount = sourceScene->mNumMeshes;
    SceneInfo::MeshMap meshes;
    meshes.reserve(meshCount);
    SceneInfo::Mesh2SkinMap mesh2skin;
    mesh2skin.reserve(meshCount);

    const auto embeddedTextureCount = sourceScene->mNumTextures;
    SceneInfo::EmbeddedTextureMap embeddedTextures;

    SceneInfo::SkinMap skins;

    for (It i = 0; i != materialCount; ++i)
        materials.push_back({sourceScene->mMaterials[i], nullptr});

    for (It i = 0; i != meshCount; ++i) {
        meshes.push_back({sourceScene->mMeshes[i], nullptr});
        if (sourceScene->mMeshes[i]->HasBones()) {
            mesh2skin.push_back(skins.size());
            const auto boneCount = sourceScene->mMeshes[i]->mNumBones;
            auto bones = sourceScene->mMeshes[i]->mBones;
            skins.push_back(SceneInfo::skinData{ bones, boneCount, nullptr });

            // For skinning, we need to get the joints list and their target nodes.
            // It is also done by the string based mapping and many of them will
            // be animated. So we will use existing AnimationNodeMap for the data.
            for (It j = 0; j != boneCount; ++j) {
                const auto &nodeName = bones[j]->mName;
                if (nodeName.length > 0) {
                    animatingNodes.insert(QByteArray{ nodeName.C_Str(),
                                                          qsizetype(nodeName.length) },
                                          nullptr);
                }
            }
        } else {
            mesh2skin.push_back(-1);
        }
    }

    for (It i = 0; i != embeddedTextureCount; ++i)
        embeddedTextures.push_back(nullptr);

    SceneInfo::TextureMap textureMap;

    if (!targetScene.root) {
        auto root = new QSSGSceneDesc::Node(QSSGSceneDesc::Node::Type::Transform, QSSGSceneDesc::Node::RuntimeType::Node);
        QSSGSceneDesc::addNode(targetScene, *root);
    }

    // Get Options
    auto opt = processSceneOptions(options);
    // check if the asset is GLTF format
    const auto extension = sourceFile.suffix().toLower();
    if (extension == QStringLiteral("gltf") || extension == QStringLiteral("glb"))
        opt.gltfMode = true;
    else if (extension == QStringLiteral("fbx"))
        opt.fbxMode = true;

    SceneInfo sceneInfo { *sourceScene, materials, meshes, embeddedTextures,
                          textureMap, skins, mesh2skin, sourceFile.dir(), opt };

    if (!qFuzzyCompare(opt.globalScaleValue, 1.0f) && !qFuzzyCompare(opt.globalScaleValue, 0.0f)) {
        const auto gscale = opt.globalScaleValue;
        QSSGSceneDesc::setProperty(*targetScene.root, "scale", &QQuick3DNode::setScale, QVector3D { gscale, gscale, gscale });
    }

    // Now lets go through the scene
    if (sourceScene->mRootNode)
        processNode(sceneInfo, *sourceScene->mRootNode, *targetScene.root, nodeMap, animatingNodes);
    // skins
    for (It i = 0, endI = skins.size(); i != endI; ++i) {
        const auto &skin = skins[i];

        // It is possible that an asset has a unused mesh with a skin
        if (!skin.node)
            continue;

        QList<QMatrix4x4> inverseBindPoses;
        QVarLengthArray<QSSGSceneDesc::Node *> joints;
        joints.reserve(skin.mNumBones);
        for (It j = 0, endJ = skin.mNumBones; j != endJ; ++j) {
            const auto &bone = *skin.mBones[j];
            const auto &nodeName = bone.mName;
            if (nodeName.length > 0) {
                auto targetNode = animatingNodes.value(QByteArray{ nodeName.C_Str(), qsizetype(nodeName.length) });
                joints.push_back(targetNode);
                const auto &osMat = bone.mOffsetMatrix;
                auto pose = QMatrix4x4(osMat[0][0], osMat[0][1], osMat[0][2], osMat[0][3],
                                       osMat[1][0], osMat[1][1], osMat[1][2], osMat[1][3],
                                       osMat[2][0], osMat[2][1], osMat[2][2], osMat[2][3],
                                       osMat[3][0], osMat[3][1], osMat[3][2], osMat[3][3]);
                inverseBindPoses.push_back(pose);
            }
        }
        QSSGSceneDesc::setProperty(*skin.node, "joints", &QQuick3DSkin::joints, joints);
        QSSGSceneDesc::setProperty(*skin.node, "inverseBindPoses", &QQuick3DSkin::setInverseBindPoses, inverseBindPoses);
    }

    static const auto fuzzyComparePos = [](const aiVectorKey *pos, const aiVectorKey *prev){
        if (!prev)
            return false;
        return qFuzzyCompare(pos->mValue.x, prev->mValue.x)
                && qFuzzyCompare(pos->mValue.y, prev->mValue.y)
                && qFuzzyCompare(pos->mValue.z, prev->mValue.z);
    };

    static const auto fuzzyCompareRot = [](const aiQuatKey *rot, const aiQuatKey *prev){
        if (!prev)
            return false;
        return qFuzzyCompare(rot->mValue.x, prev->mValue.x)
                && qFuzzyCompare(rot->mValue.y, prev->mValue.y)
                && qFuzzyCompare(rot->mValue.z, prev->mValue.z)
                && qFuzzyCompare(rot->mValue.w, prev->mValue.w);
    };

    static const auto createAnimation = [](QSSGSceneDesc::Scene &targetScene, const aiAnimation &srcAnim, const AnimationNodeMap &animatingNodes) {
        using namespace QSSGSceneDesc;
        Animation targetAnimation;
        auto &channels = targetAnimation.channels;
        qreal freq = qFuzzyIsNull(srcAnim.mTicksPerSecond) ? 1.0
                                        : 1000.0 / srcAnim.mTicksPerSecond;
        targetAnimation.framesPerSecond = srcAnim.mTicksPerSecond;
        targetAnimation.name = fromAiString(srcAnim.mName);
        // Process property channels
        for (It i = 0, end = srcAnim.mNumChannels; i != end; ++i) {
            const auto &srcChannel = *srcAnim.mChannels[i];

            const auto &nodeName = srcChannel.mNodeName;
            if (nodeName.length > 0) {
                const auto aNodeEnd = animatingNodes.cend();
                const auto aNodeIt = animatingNodes.constFind(QByteArray{ nodeName.C_Str(), qsizetype(nodeName.length) });
                if (aNodeIt != aNodeEnd && aNodeIt.value() != nullptr) {
                    auto targetNode = aNodeIt.value();
                    // Target propert[y|ies]

                    const auto currentPropertyValue = [targetNode](const char *propertyName) -> QVariant {
                        for (auto *p : targetNode->properties) {
                            if (!qstrcmp(propertyName, p->name))
                                return p->value;
                        }
                        return {};
                    };

                    { // Position
                        const auto posKeyEnd = srcChannel.mNumPositionKeys;
                        Animation::Channel targetChannel;
                        targetChannel.targetProperty = Animation::Channel::TargetProperty::Position;
                        targetChannel.target = targetNode;
                        const aiVectorKey *prevPos = nullptr;
                        for (It posKeyIdx = 0; posKeyIdx != posKeyEnd; ++posKeyIdx) {
                            const auto &posKey = srcChannel.mPositionKeys[posKeyIdx];
                            if (fuzzyComparePos(&posKey, prevPos))
                                continue;
                            targetChannel.keys.push_back(new Animation::KeyPosition(toAnimationKey(posKey, freq)));
                            prevPos = &posKey;
                        }

                        const auto isUnchanged = [&targetChannel, currentPropertyValue]() {
                            if (targetChannel.keys.count() != 1)
                                return false;
                            auto currentPos = currentPropertyValue("position").value<QVector3D>();
                            return qFuzzyCompare(targetChannel.keys[0]->value.toVector3D(), currentPos);
                        };
                        if (!targetChannel.keys.isEmpty()) {
                            if (!isUnchanged()) {
                                channels.push_back(new Animation::Channel(targetChannel));
                                float endTime = float(srcChannel.mPositionKeys[posKeyEnd - 1].mTime) * freq;
                                if (targetAnimation.length < endTime)
                                    targetAnimation.length = endTime;
                            } else {
                                // the keys will not be used.
                                qDeleteAll(targetChannel.keys);
                            }
                        }
                    }

                    { // Rotation
                        const auto rotKeyEnd = srcChannel.mNumRotationKeys;
                        Animation::Channel targetChannel;
                        targetChannel.targetProperty = Animation::Channel::TargetProperty::Rotation;
                        targetChannel.target = targetNode;
                        const aiQuatKey *prevRot = nullptr;
                        for (It rotKeyIdx = 0; rotKeyIdx != rotKeyEnd; ++rotKeyIdx) {
                            const auto &rotKey = srcChannel.mRotationKeys[rotKeyIdx];
                            if (fuzzyCompareRot(&rotKey, prevRot))
                                continue;
                            targetChannel.keys.push_back(new Animation::KeyPosition(toAnimationKey(rotKey, freq)));
                            prevRot = &rotKey;
                        }

                        const auto isUnchanged = [&targetChannel, currentPropertyValue]() {
                            if (targetChannel.keys.count() != 1)
                                return false;
                            auto currentVal = currentPropertyValue("rotation");
                            QQuaternion rot = currentVal.isValid() ? currentVal.value<QQuaternion>() : QQuaternion{};
                            return qFuzzyCompare(QQuaternion(targetChannel.keys[0]->value), rot);
                        };
                        if (!targetChannel.keys.isEmpty()) {
                            if (!isUnchanged()) {
                                channels.push_back(new Animation::Channel(targetChannel));
                                float endTime = float(srcChannel.mRotationKeys[rotKeyEnd - 1].mTime) * freq;
                                if (targetAnimation.length < endTime)
                                    targetAnimation.length = endTime;
                            } else {
                                // the keys will not be used.
                                qDeleteAll(targetChannel.keys);
                            }
                        }
                    }

                    { // Scale
                        const auto scaleKeyEnd = srcChannel.mNumScalingKeys;
                        Animation::Channel targetChannel;
                        targetChannel.targetProperty = Animation::Channel::TargetProperty::Scale;
                        targetChannel.target = targetNode;
                        const aiVectorKey *prevScale = nullptr;
                        for (It scaleKeyIdx = 0; scaleKeyIdx != scaleKeyEnd; ++scaleKeyIdx) {
                            const auto &scaleKey = srcChannel.mScalingKeys[scaleKeyIdx];
                            if (fuzzyComparePos(&scaleKey, prevScale))
                                continue;
                            targetChannel.keys.push_back(new Animation::KeyPosition(toAnimationKey(scaleKey, freq)));
                            prevScale = &scaleKey;
                        }

                        const auto isUnchanged = [&targetChannel, currentPropertyValue]() {
                            if (targetChannel.keys.count() != 1)
                                return false;
                            auto currentVal = currentPropertyValue("scale");
                            QVector3D scale = currentVal.isValid() ? currentVal.value<QVector3D>() : QVector3D{ 1, 1, 1 };
                            return qFuzzyCompare(targetChannel.keys[0]->value.toVector3D(), scale);
                        };

                        if (!targetChannel.keys.isEmpty()) {
                            if (!isUnchanged()) {
                                channels.push_back(new Animation::Channel(targetChannel));
                                float endTime = float(srcChannel.mScalingKeys[scaleKeyEnd - 1].mTime) * freq;
                                if (targetAnimation.length < endTime)
                                    targetAnimation.length = endTime;
                            } else {
                                // the keys will not be used.
                                qDeleteAll(targetChannel.keys);
                            }
                        }
                    }
                }
            }
        }
        // Morphing Animations
        for (It i = 0, end = srcAnim.mNumMorphMeshChannels; i != end; ++i) {
            const auto &srcMorphChannel = *srcAnim.mMorphMeshChannels[i];
            const QString nodeName(srcMorphChannel.mName.C_Str());
            const auto *morphKeys = srcMorphChannel.mKeys;
            const auto numMorphTargets = qMin(morphKeys[0].mNumValuesAndWeights, 8U);
            for (It targetId = 0; targetId != numMorphTargets; ++targetId) {
                QString morphTargetName = nodeName + QStringLiteral("_morph") + QString::number(targetId);
                const auto aNodeEnd = animatingNodes.cend();
                const auto aNodeIt = animatingNodes.constFind(morphTargetName.toUtf8());
                if (aNodeIt != aNodeEnd && aNodeIt.value() != nullptr) {
                    auto targetNode = aNodeIt.value();
                    const auto weightKeyEnd = srcMorphChannel.mNumKeys;
                    Animation::Channel targetChannel;
                    targetChannel.targetProperty = Animation::Channel::TargetProperty::Weight;
                    targetChannel.target = targetNode;
                    for (It wId = 0; wId != weightKeyEnd; ++wId) {
                        const auto &weightKey = srcMorphChannel.mKeys[wId];
                        const auto animationKey = new Animation::KeyPosition(toAnimationKey(weightKey, freq, targetId));
                        targetChannel.keys.push_back(animationKey);
                    }
                    if (!targetChannel.keys.isEmpty()) {
                        channels.push_back(new Animation::Channel(targetChannel));
                        float endTime = float(srcMorphChannel.mKeys[weightKeyEnd - 1].mTime) * freq;
                        if (targetAnimation.length < endTime)
                            targetAnimation.length = endTime;
                    }
                }
            }
        }

        // If we have data we need to make it persistent.
        if (!targetAnimation.channels.isEmpty())
            targetScene.animations.push_back(new Animation(targetAnimation));
    };

    // All scene nodes should now be created (and ready), so let's go through the animation data.
    if (sourceScene->HasAnimations()) {
        const auto animationCount = sourceScene->mNumAnimations;
        targetScene.animations.reserve(animationCount);
        for (It i = 0, end = animationCount; i != end; ++i) {
            const auto &srcAnim = *sourceScene->mAnimations[i];
            createAnimation(targetScene, srcAnim, animatingNodes);
        }
    }

    // TODO, FIX: Editing the scene after the import ought to be done by QSSGAssetImportManager
    // and not by the asset import plugin. However, the asset import module cannot use
    // the asset utils module because that would cause a circular dependency. This
    // needs a deeper architectural fix.

    QSSGQmlUtilities::applyEdit(&targetScene, options);

    return QString();
}

////////////////////////

QString AssimpImporter::import(const QUrl &url, const QJsonObject &options, QSSGSceneDesc::Scene &scene)
{
    // We'll simply use assimp to load the scene and then translate the Aassimp scene
    // into our own format.
    return importImp(url, options, scene);
}

QString AssimpImporter::import(const QString &sourceFile, const QDir &savePath, const QJsonObject &options, QStringList *generatedFiles)
{
    QString errorString;

    QSSGSceneDesc::Scene scene;

    // Load scene data
    auto sourceUrl = QUrl::fromLocalFile(sourceFile);
    errorString = importImp(sourceUrl, options, scene);

    if (!errorString.isEmpty())
        return errorString;

    // Write out QML + Resources
    QFileInfo sourceFileInfo(sourceFile);

    QString targetFileName = savePath.absolutePath() + QDir::separator() +
            QSSGQmlUtilities::qmlComponentName(sourceFileInfo.completeBaseName()) +
            QStringLiteral(".qml");
    QFile targetFile(targetFileName);
    if (!targetFile.open(QIODevice::WriteOnly)) {
        errorString += QString("Could not write to file: ") + targetFileName;
    } else {
        QTextStream output(&targetFile);
        QSSGQmlUtilities::writeQml(scene, output, savePath, options);
        if (generatedFiles)
            generatedFiles->append(targetFileName);
    }
    scene.cleanup();

    return errorString;
}

QT_END_NAMESPACE
