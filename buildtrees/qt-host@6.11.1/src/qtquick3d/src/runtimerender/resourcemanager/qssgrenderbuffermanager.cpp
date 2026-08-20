// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderbuffermanager_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderloadedtexture_p.h>

#include <QtQuick3DRuntimeRender/private/qssgruntimerenderlogging_p.h>
#include <QtQuick3DUtils/private/qssgmeshbvhbuilder_p.h>
#include <QtQuick3DUtils/private/qssgbounds3_p.h>
#include <QtQuick3DUtils/private/qssgassert_p.h>

#include <QtQuick/QSGTexture>

#include <QtCore/QDir>
#include <QtGui/private/qimage_p.h>
#include <QtQuick/private/qsgtexture_p.h>
#include <QtQuick/private/qsgcompressedtexture_p.h>
#include <QBuffer>

#include "../utils/qssgrenderbasetypes_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrendergeometry_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermodel_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendertexturedata_p.h>
#include "../qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssglightmapper_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderresourceloader_p.h>
#include <qtquick3d_tracepoints_p.h>
#include "../extensionapi/qssgrenderextensions.h"
#include "qssglightmapio_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

struct QSSGBufferManagerStat
{
enum class Level
{
    Debug = 0x1,
    Usage = 0x2,
    // 0x3 DebugAndUsage
};

#ifdef QT_DEBUG
static bool enabled(Level level)
{
    constexpr Level DebugAndUsage { quint8(Level::Debug) | quint8(Level::Usage) };
    static auto v = qEnvironmentVariableIntValue("QSSG_BUFFERMANAGER_DEBUG");
    return v && Level{v} <= DebugAndUsage && (v & quint8(level));
}
#else
static constexpr bool enabled(Level) { return false; }
#endif

}

Q_TRACE_POINT(qtquick3d, QSSG_textureLoad_entry);
Q_TRACE_POINT(qtquick3d, QSSG_textureLoad_exit);
Q_TRACE_POINT(qtquick3d, QSSG_meshLoad_entry);
Q_TRACE_POINT(qtquick3d, QSSG_meshLoad_exit);
Q_TRACE_POINT(qtquick3d, QSSG_meshLoadPath_entry, const QString &path);
Q_TRACE_POINT(qtquick3d, QSSG_meshLoadPath_exit);
Q_TRACE_POINT(qtquick3d, QSSG_textureUnload_entry);
Q_TRACE_POINT(qtquick3d, QSSG_textureUnload_exit);
Q_TRACE_POINT(qtquick3d, QSSG_meshUnload_entry);
Q_TRACE_POINT(qtquick3d, QSSG_meshUnload_exit);
Q_TRACE_POINT(qtquick3d, QSSG_customMeshLoad_entry);
Q_TRACE_POINT(qtquick3d, QSSG_customMeshLoad_exit);
Q_TRACE_POINT(qtquick3d, QSSG_customMeshUnload_entry);
Q_TRACE_POINT(qtquick3d, QSSG_customMeshUnload_exit);
Q_TRACE_POINT(qtquick3d, QSSG_textureLoadPath_entry, const QString &path);
Q_TRACE_POINT(qtquick3d, QSSG_textureLoadPath_exit);

struct MeshStorageRef
{
    QVector<QSSGMesh::Mesh> meshes;
    qsizetype ref = 0;
};
using AssetMeshMap = QHash<QString, MeshStorageRef>;

Q_GLOBAL_STATIC(AssetMeshMap, g_assetMeshMap)

// Returns !idx@asset_id
QString QSSGBufferManager::runtimeMeshSourceName(const QString &assetId, qsizetype meshId)
{
    return QString::fromUtf16(u"!%1@%2").arg(QString::number(meshId), assetId);
}

using MeshIdxNamePair = QPair<qsizetype, QString>;
static MeshIdxNamePair splitRuntimeMeshPath(const QSSGRenderPath &rpath)
{
    const auto &path = rpath.path();
    Q_ASSERT(path.startsWith(u'!'));
    const auto strings = path.mid(1).split(u'@');
    const bool hasData = (strings.size() == 2) && !strings[0].isEmpty() && !strings[1].isEmpty();
    qsizetype idx = -1;
    bool ok = false;
    if (hasData)
        idx = strings.at(0).toLongLong(&ok);

    return (ok) ? qMakePair(idx, strings.at(1)) : qMakePair(qsizetype(-1), QString());
}

namespace {
struct PrimitiveEntry
{
    // Name of the primitive as it will be in e.g., the QML file
    const char *primitive;
    // Name of the primitive file on the filesystem
    const char *file;
};
}

static const int nPrimitives = 5;
static const PrimitiveEntry primitives[nPrimitives] = {
    {"#Rectangle", "/Rectangle.mesh"},
    {"#Sphere","/Sphere.mesh"},
    {"#Cube","/Cube.mesh"},
    {"#Cone","/Cone.mesh"},
    {"#Cylinder","/Cylinder.mesh"},
};

static const char *primitivesDirectory = "res//primitives";

static constexpr QSize sizeForMipLevel(int mipLevel, const QSize &baseLevelSize)
{
    return QSize(qMax(1, baseLevelSize.width() >> mipLevel), qMax(1, baseLevelSize.height() >> mipLevel));
}

static QPair<QSSGMesh::Mesh, QString> loadFromLightmapFile(const QString &lightmapPath, const QString &lightmapKey)
{
    QPair<QSSGMesh::Mesh, QString> retVal;

    if (lightmapPath.isEmpty() || lightmapKey.isEmpty())
        return retVal;

    if (const auto io = QSSGLightmapLoader::open(lightmapPath)) {
        const QVariantMap metadata = io->readMap(lightmapKey, QSSGLightmapIODataTag::Metadata);
        if (metadata.isEmpty())
            return retVal;

        const QString meshKey = metadata[QStringLiteral("mesh_key")].toString();
        if (meshKey.isEmpty())
            return retVal;

        QByteArray meshData = io->readData(meshKey, QSSGLightmapIODataTag::Mesh);
        if (meshData.isEmpty())
            return retVal;

        QBuffer buffer(&meshData);
        buffer.open(QIODevice::ReadOnly);
        retVal.first = QSSGMesh::Mesh::loadMesh(&buffer, 1);
        retVal.second = QFileInfo(lightmapPath).fileName() + " ["_L1 + lightmapKey + u']';
    }

    return retVal;
}

QSSGBufferManager::QSSGBufferManager()
{
}

QSSGBufferManager::~QSSGBufferManager()
{
    clear();
    m_contextInterface = nullptr;
}

void QSSGBufferManager::setRenderContextInterface(QSSGRenderContextInterface *ctx)
{
    m_contextInterface = ctx;
}

void QSSGBufferManager::releaseCachedResources()
{
    clear();
}

void QSSGBufferManager::releaseResourcesForLayer(QSSGRenderLayer *layer)
{
    // frameResetIndex must be +1 since it's depending on being the
    // next frame and this is the cleanup after the final frame as
    // the layer is destroyed
    resetUsageCounters(frameResetIndex + 1, layer);
    cleanupUnreferencedBuffers(frameResetIndex + 1, layer);
}

QSSGRenderImageTexture QSSGBufferManager::loadRenderImage(const QSSGRenderImage *image,
                                                          MipMode inMipMode,
                                                          LoadRenderImageFlags flags)
{
    if (inMipMode == MipModeFollowRenderImage)
        inMipMode = image->m_generateMipmaps ? MipModeEnable : MipModeDisable;

    const auto &context = m_contextInterface->rhiContext();
    QSSGRenderImageTexture result;
    if (image->m_qsgTexture) {
        QRhi *rhi = context->rhi();
        QSGTexture *qsgTexture = image->m_qsgTexture;
        QRhiTexture *rhiTex = qsgTexture->rhiTexture(); // this may not be valid until commit and that's ok
        if (!rhiTex || rhiTex->rhi() == rhi) {
            // A QSGTexture from a textureprovider that is not a QSGDynamicTexture
            // needs to be pushed to get its content updated (or even to create a
            // QRhiTexture in the first place).
            QRhiResourceUpdateBatch *rub = rhi->nextResourceUpdateBatch();
            if (qsgTexture->isAtlasTexture()) {
                // This returns a non-atlased QSGTexture (or does nothing if the
                // extraction has already been done), the ownership of which stays with
                // the atlas. As we do not store (and do not own) qsgTexture below,
                // apart from using it as a cache key and querying its QRhiTexture
                // (which we again do not own), we can just pretend we got the
                // non-atlased QSGTexture in the first place.
                qsgTexture = qsgTexture->removedFromAtlas(rub);
            }
            qsgTexture->commitTextureOperations(rhi, rub);
            context->commandBuffer()->resourceUpdate(rub);
            auto theImage = qsgImageMap.find(qsgTexture);
            if (theImage == qsgImageMap.end())
                theImage = qsgImageMap.insert(qsgTexture, ImageData());
            theImage.value().renderImageTexture.m_texture = qsgTexture->rhiTexture();
            theImage.value().renderImageTexture.m_flags.setHasTransparency(qsgTexture->hasAlphaChannel());
            // If a texture from Qt Quick has an alpha channel, it is always pre-multiplied
            theImage.value().renderImageTexture.m_flags.setPreMultipliedAlpha(qsgTexture->hasAlphaChannel());
            theImage.value().usageCounts[currentLayer]++;
            result = theImage.value().renderImageTexture;
            // inMipMode is ignored completely when sourcing the texture from a
            // QSGTexture. Mipmap generation is not supported, whereas
            // attempting to use such a texture as a light probe will fail. (no
            // mip levels, no pre-filtering) In the latter case, print a warning
            // because that will definitely lead to visual problems in the result.
            if (inMipMode == MipModeBsdf)
                qWarning("Cannot use QSGTexture from Texture.sourceItem as light probe.");
        } else {
            qWarning("Cannot use QSGTexture (presumably from Texture.sourceItem) created in another "
                     "window that was using a different graphics device/context. "
                     "Avoid using View3D.importScene between multiple windows.");
        }

    } else if (image->m_rawTextureData) {
        Q_TRACE_SCOPE(QSSG_textureLoad);
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DTextureLoad);
        result = loadTextureData(image->m_rawTextureData, inMipMode);
        Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DTextureLoad, stats.imageDataSize, image->profilingId);
    } else if (!image->m_imagePath.isEmpty()) {

        const ImageCacheKey imageKey = { image->m_imagePath, inMipMode, int(image->type), QString() };
        auto foundIt = imageMap.find(imageKey);
        if (foundIt != imageMap.cend()) {
            result = foundIt.value().renderImageTexture;
        } else {
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DTextureLoad);
            QScopedPointer<QSSGLoadedTexture> theLoadedTexture;
            const auto &path = image->m_imagePath.path();
            const bool flipY = flags.testFlag(LoadWithFlippedY);
            Q_TRACE_SCOPE(QSSG_textureLoadPath, path);
            theLoadedTexture.reset(QSSGLoadedTexture::load(path, image->m_format, flipY));
            if (theLoadedTexture) {
                foundIt = imageMap.insert(imageKey, ImageData());
                CreateRhiTextureFlags rhiTexFlags = ScanForTransparency;
                if (image->type == QSSGRenderGraphObject::Type::ImageCube)
                    rhiTexFlags |= CubeMap;
                if (!setRhiTexture(foundIt.value().renderImageTexture, theLoadedTexture.data(), inMipMode, rhiTexFlags, QFileInfo(path).fileName())) {
                    foundIt.value() = ImageData();
                } else if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug)) {
                    qDebug() << "+ uploadTexture: " << image->m_imagePath.path() << currentLayer;
                }
                result = foundIt.value().renderImageTexture;
                increaseMemoryStat(result.m_texture);
            } else {
                // We want to make sure that bad path fails once and doesn't fail over and over
                // again
                // which could slow down the system quite a bit.
                foundIt = imageMap.insert(imageKey, ImageData());
                qCWarning(WARNING, "Failed to load image: %s", qPrintable(path));
            }
            Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DTextureLoad, stats.imageDataSize, path.toUtf8());
        }
        foundIt.value().usageCounts[currentLayer]++;
    } else if (image->m_extensionsSource) {
        auto it = renderExtensionTexture.find(image->m_extensionsSource);
        if (it != renderExtensionTexture.end()) {
            it->usageCounts[currentLayer]++;
            result = it->renderImageTexture;
        }
    }
    return result;
}

QSSGRenderImageTexture QSSGBufferManager::loadTextureData(QSSGRenderTextureData *data, MipMode inMipMode)
{
    QSSG_ASSERT(data != nullptr, return {});

    const CustomImageCacheKey imageKey = { data, data->size(), inMipMode };
    auto theImageData = customTextureMap.find(imageKey);
    if (theImageData == customTextureMap.end()) {
        theImageData = customTextureMap.insert(imageKey, ImageData{{}, {}, data->version()});
    } else if (data->version() == theImageData->version) {
        // Return the currently loaded texture
        theImageData.value().usageCounts[currentLayer]++;
        return theImageData.value().renderImageTexture;
    } else {
        // Optimization: If only the version number has changed, we can attempt to reuse the texture.
        // Just update the version number and let setRhiTexture handle the rest.
        theImageData->version = data->version();
    }

    // Load the texture
    QScopedPointer<QSSGLoadedTexture> theLoadedTexture;
    if (!data->textureData().isNull()) {
        theLoadedTexture.reset(QSSGLoadedTexture::loadTextureData(data));
        theLoadedTexture->ownsData = false;
        CreateRhiTextureFlags rhiTexFlags = {};
        if (theLoadedTexture->depth > 0)
            rhiTexFlags |= Texture3D;

        bool wasTextureCreated = false;

        if (setRhiTexture(theImageData.value().renderImageTexture, theLoadedTexture.data(), inMipMode, rhiTexFlags, data->debugObjectName, &wasTextureCreated)) {
            if (wasTextureCreated) {
                if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                    qDebug() << "+ uploadTexture: " << data << theImageData.value().renderImageTexture.m_texture << currentLayer;
                increaseMemoryStat(theImageData.value().renderImageTexture.m_texture);
            }
        } else {
            theImageData.value() = ImageData();
        }
    }

    theImageData.value().usageCounts[currentLayer]++;
    return theImageData.value().renderImageTexture;
}

QSSGRenderImageTexture QSSGBufferManager::loadLightmap(const QSSGRenderModel &model)
{
    Q_ASSERT(currentLayer);

    if (model.lightmapKey.isEmpty() || currentlyLightmapBaking || !validateLightmap())
        return {};

    Q_ASSERT(!lightmapSource.isEmpty());
    static const QSSGRenderTextureFormat format = QSSGRenderTextureFormat::RGBA16F;
    QSSGRenderImageTexture result;
    const ImageCacheKey imageKey = { QSSGRenderPath(lightmapSource), MipModeDisable, int(QSSGRenderGraphObject::Type::Image2D), model.lightmapKey };
    auto foundIt = imageMap.find(imageKey);
    if (foundIt != imageMap.end()) {
        result = foundIt.value().renderImageTexture;
    } else {
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DTextureLoad);
        Q_TRACE_SCOPE(QSSG_textureLoadPath, lightmapSource);
        QScopedPointer<QSSGLoadedTexture> theLoadedTexture;
        theLoadedTexture.reset(QSSGLoadedTexture::loadLightmapImage(lightmapSource, format, model.lightmapKey));
        if (!theLoadedTexture) {
            qCWarning(WARNING, "Failed to load lightmap image for %s", qPrintable(model.lightmapKey));
        }
        foundIt = imageMap.insert(imageKey, ImageData());
        if (theLoadedTexture) {
            const QString debugOjbectName = lightmapSource + QStringLiteral(" [%1]").arg(model.lightmapKey);
            if (!setRhiTexture(foundIt.value().renderImageTexture, theLoadedTexture.data(), MipModeDisable, {}, debugOjbectName))
                foundIt.value() = ImageData();
            else if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                qDebug() << "+ uploadTexture: " << debugOjbectName << currentLayer;
            result = foundIt.value().renderImageTexture;
        }
        increaseMemoryStat(result.m_texture);
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DTextureLoad, stats.imageDataSize, lightmapSource.toUtf8());
    }
    foundIt.value().usageCounts[currentLayer]++;
    return result;
}

QSSGRenderImageTexture QSSGBufferManager::loadSkinmap(QSSGRenderTextureData *skin)
{
    return loadTextureData(skin, MipModeDisable);
}

QSSGRenderMesh *QSSGBufferManager::getMeshForPicking(const QSSGRenderModel &model, const QString &lightmap) const
{
    if (!model.meshPath.isNull()) {
        auto key = std::make_pair(model.meshPath, lightmap);
        const auto foundIt = meshMap.constFind(key);
        if (foundIt != meshMap.constEnd())
            return foundIt->mesh;
    }

    if (model.geometry) {
        auto key = std::make_pair(model.geometry, lightmap);
        const auto foundIt = customMeshMap.constFind(key);
        if (foundIt != customMeshMap.constEnd())
            return foundIt->mesh;
    }

    return nullptr;
}

QRhiTexture::Format QSSGBufferManager::toRhiFormat(const QSSGRenderTextureFormat format)
{
    switch (format.format) {

    case QSSGRenderTextureFormat::RGBA8:
        return QRhiTexture::RGBA8;
    case QSSGRenderTextureFormat::R8:
        return QRhiTexture::R8;
    case QSSGRenderTextureFormat::Luminance16: //???
    case QSSGRenderTextureFormat::R16:
        return QRhiTexture::R16;
    case QSSGRenderTextureFormat::LuminanceAlpha8:
    case QSSGRenderTextureFormat::Luminance8:
    case QSSGRenderTextureFormat::Alpha8:
        return QRhiTexture::RED_OR_ALPHA8;
    case QSSGRenderTextureFormat::RGBA16F:
        return QRhiTexture::RGBA16F;
    case QSSGRenderTextureFormat::RGBA32F:
        return QRhiTexture::RGBA32F;
    case QSSGRenderTextureFormat::R16F:
        return QRhiTexture::R16F;
    case QSSGRenderTextureFormat::R32F:
        return QRhiTexture::R32F;
    case QSSGRenderTextureFormat::RGBE8:
        return QRhiTexture::RGBA8;
    case QSSGRenderTextureFormat::R32UI:
        return QRhiTexture::R32UI;
    case QSSGRenderTextureFormat::RGBA32UI:
        return QRhiTexture::RGBA32UI;
    case QSSGRenderTextureFormat::RGB_DXT1:
        return QRhiTexture::BC1;
    case QSSGRenderTextureFormat::RGBA_DXT3:
        return QRhiTexture::BC2;
    case QSSGRenderTextureFormat::RGBA_DXT5:
        return QRhiTexture::BC3;
    case QSSGRenderTextureFormat::RGBA8_ETC2_EAC:
        return QRhiTexture::ETC2_RGBA8;
    case QSSGRenderTextureFormat::RGBA_ASTC_4x4:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_4x4:
        return QRhiTexture::ASTC_4x4;
    case QSSGRenderTextureFormat::RGBA_ASTC_5x4:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_5x4:
        return QRhiTexture::ASTC_5x4;
    case QSSGRenderTextureFormat::RGBA_ASTC_5x5:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_5x5:
        return QRhiTexture::ASTC_5x5;
    case QSSGRenderTextureFormat::RGBA_ASTC_6x5:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_6x5:
        return QRhiTexture::ASTC_6x5;
    case QSSGRenderTextureFormat::RGBA_ASTC_6x6:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_6x6:
        return QRhiTexture::ASTC_6x6;
    case QSSGRenderTextureFormat::RGBA_ASTC_8x5:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_8x5:
        return QRhiTexture::ASTC_8x5;
    case QSSGRenderTextureFormat::RGBA_ASTC_8x6:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_8x6:
        return QRhiTexture::ASTC_8x6;
    case QSSGRenderTextureFormat::RGBA_ASTC_8x8:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_8x8:
        return QRhiTexture::ASTC_8x8;
    case QSSGRenderTextureFormat::RGBA_ASTC_10x5:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_10x5:
        return QRhiTexture::ASTC_10x5;
    case QSSGRenderTextureFormat::RGBA_ASTC_10x6:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_10x6:
        return QRhiTexture::ASTC_10x6;
    case QSSGRenderTextureFormat::RGBA_ASTC_10x8:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_10x8:
        return QRhiTexture::ASTC_10x8;
    case QSSGRenderTextureFormat::RGBA_ASTC_10x10:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_10x10:
        return QRhiTexture::ASTC_10x10;
    case QSSGRenderTextureFormat::RGBA_ASTC_12x10:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_12x10:
        return QRhiTexture::ASTC_12x10;
    case QSSGRenderTextureFormat::RGBA_ASTC_12x12:
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_12x12:
        return QRhiTexture::ASTC_12x12;

    case QSSGRenderTextureFormat::Depth16:
        return QRhiTexture::D16;
    case QSSGRenderTextureFormat::Depth24:
        return QRhiTexture::D24;
    case QSSGRenderTextureFormat::Depth32:
        return QRhiTexture::D32F;
    case QSSGRenderTextureFormat::Depth24Stencil8:
        return QRhiTexture::D24S8;

    case QSSGRenderTextureFormat::SRGB8A8:
        return QRhiTexture::RGBA8; // Note: user must keep track of color space manually

    default:
        qWarning() << "Unsupported texture format" << format.format;
        return QRhiTexture::UnknownFormat;
    }

}

// Vertex data for rendering environment cube map
static const float cube[] = {
    -1.0f,-1.0f,-1.0f,  // -X side
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,

    -1.0f,-1.0f,-1.0f,  // -Z side
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,

    -1.0f,-1.0f,-1.0f,  // -Y side
    1.0f,-1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,

    -1.0f, 1.0f,-1.0f,  // +Y side
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,

    1.0f, 1.0f,-1.0f,  // +X side
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,

    -1.0f, 1.0f, 1.0f,  // +Z side
    -1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,

    0.0f, 1.0f,  // -X side
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,

    1.0f, 1.0f,  // -Z side
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,  // -Y side
    1.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,  // +Y side
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,

    1.0f, 0.0f,  // +X side
    0.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 0.0f,  // +Z side
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
};

bool QSSGBufferManager::createEnvironmentMap(const QSSGLoadedTexture *inImage, QSSGRenderImageTexture *outTexture, const QString &debugObjectName)
{
    // The objective of this method is to take the equirectangular texture
    // provided by inImage and create a cubeMap that contains both pre-filtered
    // specular environment maps, as well as a irradiance map for diffuse
    // operations.
    // To achieve this though we first convert convert the Equirectangular texture
    // to a cubeMap with genereated mip map levels (no filtering) to make the
    // process of creating the prefiltered and irradiance maps eaiser. This
    // intermediate texture as well as the original equirectangular texture are
    // destroyed after this frame completes, and all further associations with
    // the source lightProbe texture are instead associated with the final
    // generated environment map.
    // The intermediate environment cubemap is used to generate the final
    // cubemap. This cubemap will generate 6 mip levels for each face
    // (the remaining faces are unused).  This is what the contents of each
    // face mip level looks like:
    // 0: Pre-filtered with roughness 0 (basically unfiltered)
    // 1: Pre-filtered with roughness 0.25
    // 2: Pre-filtered with roughness 0.5
    // 3: Pre-filtered with roughness 0.75
    // 4: Pre-filtered with rougnness 1.0
    // 5: Irradiance map (ideally at least 16x16)
    // It would be better if we could use a separate cubemap for irradiance, but
    // right now there is a 1:1 association between texture sources on the front-
    // end and backend.
    const auto &context = m_contextInterface->rhiContext();
    auto *rhi = context->rhi();
    // Right now minimum face size needs to be 512x512 to be able to have 6 reasonably sized mips
    int suggestedSize = inImage->height * 0.5f;
    suggestedSize = qMax(512, suggestedSize);
    const QSize environmentMapSize(suggestedSize, suggestedSize);
    const bool isRGBE = inImage->format.format == QSSGRenderTextureFormat::Format::RGBE8;
    const QRhiTexture::Format sourceTextureFormat = toRhiFormat(inImage->format.format);
    // Check if we can use the source texture at all
    if (!rhi->isTextureFormatSupported(sourceTextureFormat))
        return false;

    QRhiTexture::Format cubeTextureFormat = inImage->format.isCompressedTextureFormat()
            ? QRhiTexture::RGBA16F // let's just assume that if compressed textures are available, then it's at least a GLES 3.0 level API
            : sourceTextureFormat;
#ifdef Q_OS_IOS
    // iOS doesn't support mip map filtering on RGBA32F textures
    if (cubeTextureFormat == QRhiTexture::RGBA32F)
        cubeTextureFormat = QRhiTexture::RGBA16F;
#endif

    const int colorSpace = inImage->isSRGB ? 1 : 0; // 0 Linear | 1 sRGB

    // Phase 1: Convert the Equirectangular texture to a Cubemap
    QRhiTexture *envCubeMap = rhi->newTexture(cubeTextureFormat, environmentMapSize, 1,
                                              QRhiTexture::RenderTarget | QRhiTexture::CubeMap | QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips);
    if (!envCubeMap->create()) {
        qWarning("Failed to create Environment Cube Map");
        return false;
    }
    envCubeMap->deleteLater();

    // Create a renderbuffer the size of a the cubeMap face
    QRhiRenderBuffer *envMapRenderBuffer = rhi->newRenderBuffer(QRhiRenderBuffer::Color, environmentMapSize);
    if (!envMapRenderBuffer->create()) {
        qWarning("Failed to create Environment Map Render Buffer");
        return false;
    }
    envMapRenderBuffer->deleteLater();

    const QByteArray rtName = debugObjectName.toLatin1();

    // Setup the 6 render targets for each cube face
    QVarLengthArray<QRhiTextureRenderTarget *, 6> renderTargets;
    QRhiRenderPassDescriptor *renderPassDesc = nullptr;
    for (const auto face : QSSGRenderTextureCubeFaces) {
        QRhiColorAttachment att(envCubeMap);
        att.setLayer(quint8(face));
        QRhiTextureRenderTargetDescription rtDesc;
        rtDesc.setColorAttachments({att});
        auto renderTarget = rhi->newTextureRenderTarget(rtDesc);
        renderTarget->setName(rtName + QByteArrayLiteral(" env cube face: ") + QSSGBaseTypeHelpers::displayName(face));
        renderTarget->setDescription(rtDesc);
        if (!renderPassDesc)
            renderPassDesc = renderTarget->newCompatibleRenderPassDescriptor();
        renderTarget->setRenderPassDescriptor(renderPassDesc);
        if (!renderTarget->create()) {
            qWarning("Failed to build env map render target");
            return false;
        }
        renderTarget->deleteLater();
        renderTargets << renderTarget;
    }
    renderPassDesc->deleteLater();

    // Setup the sampler for reading the equirectangular loaded texture
    QSize size(inImage->width, inImage->height);
    auto *sourceTexture = rhi->newTexture(sourceTextureFormat, size, 1);
    if (!sourceTexture->create()) {
        qWarning("failed to create source env map texture");
        return false;
    }
    sourceTexture->deleteLater();

    // Upload the equirectangular texture
    const auto desc = inImage->textureFileData.isValid()
            ? QRhiTextureUploadDescription(
                    { 0, 0, QRhiTextureSubresourceUploadDescription(inImage->textureFileData.getDataView().toByteArray()) })
            : QRhiTextureUploadDescription({ 0, 0, { inImage->data, inImage->dataSizeInBytes } });

    auto *rub = rhi->nextResourceUpdateBatch();
    rub->uploadTexture(sourceTexture, desc);

    const QSSGRhiSamplerDescription samplerDesc {
        QRhiSampler::Linear,
        QRhiSampler::Linear,
        QRhiSampler::None,
        QRhiSampler::ClampToEdge,
        QRhiSampler::ClampToEdge,
        QRhiSampler::Repeat
    };
    QRhiSampler *sampler = context->sampler(samplerDesc);

    // Load shader and setup render pipeline
    const auto &shaderCache = m_contextInterface->shaderCache();
    const auto &envMapShaderStages = shaderCache->getBuiltInRhiShaders().getRhiEnvironmentmapShader();

    // Vertex Buffer - Just a single cube that will be viewed from inside
    QRhiBuffer *vertexBuffer = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(cube));
    vertexBuffer->create();
    vertexBuffer->deleteLater();
    rub->uploadStaticBuffer(vertexBuffer, cube);

    // Uniform Buffer - 2x mat4
    int ubufElementSize = rhi->ubufAligned(128);
    QRhiBuffer *uBuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufElementSize * 6);
    uBuf->create();
    uBuf->deleteLater();

    int ubufEnvMapElementSize = rhi->ubufAligned(4);
    QRhiBuffer *uBufEnvMap = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufEnvMapElementSize * 6);
    uBufEnvMap->create();
    uBufEnvMap->deleteLater();

    // Shader Resource Bindings
    QRhiShaderResourceBindings *envMapSrb = rhi->newShaderResourceBindings();
    envMapSrb->setBindings({
                         QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage, uBuf, 128),
                         QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(2, QRhiShaderResourceBinding::FragmentStage, uBufEnvMap, ubufEnvMapElementSize),
                         QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, sourceTexture, sampler)
                     });
    envMapSrb->create();
    envMapSrb->deleteLater();

    // Pipeline
    QRhiGraphicsPipeline *envMapPipeline = rhi->newGraphicsPipeline();
    envMapPipeline->setCullMode(QRhiGraphicsPipeline::Front);
    envMapPipeline->setFrontFace(QRhiGraphicsPipeline::CCW);
    envMapPipeline->setShaderStages({
                            *envMapShaderStages->vertexStage(),
                            *envMapShaderStages->fragmentStage()
                        });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({
                                { 3 * sizeof(float) }
                            });
    inputLayout.setAttributes({
                                  { 0, 0, QRhiVertexInputAttribute::Float3, 0 }
                              });

    envMapPipeline->setVertexInputLayout(inputLayout);
    envMapPipeline->setShaderResourceBindings(envMapSrb);
    envMapPipeline->setRenderPassDescriptor(renderPassDesc);
    if (!envMapPipeline->create()) {
        qWarning("failed to create source env map pipeline state");
        return false;
    }
    envMapPipeline->deleteLater();

    // Do the actual render passes
    auto *cb = context->commandBuffer();
    cb->debugMarkBegin("Environment Cubemap Generation");
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
    Q_TRACE(QSSG_renderPass_entry, QStringLiteral("Environment Cubemap Generation"));
    const QRhiCommandBuffer::VertexInput vbufBinding(vertexBuffer, 0);

    // Set the Uniform Data
    QMatrix4x4 mvp = rhi->clipSpaceCorrMatrix();
    mvp.perspective(90.0f, 1.0f, 0.1f, 10.0f);

    auto lookAt = [](const QVector3D &eye, const QVector3D &center, const QVector3D &up) {
        QMatrix4x4 viewMatrix;
        viewMatrix.lookAt(eye, center, up);
        return viewMatrix;
    };
    QVarLengthArray<QMatrix4x4, 6> views;
    views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0, 0.0, 0.0), QVector3D(0.0f, -1.0f, 0.0f)));
    views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(-1.0, 0.0, 0.0), QVector3D(0.0f, -1.0f, 0.0f)));
    if (rhi->isYUpInFramebuffer()) {
        views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, 1.0, 0.0), QVector3D(0.0f, 0.0f, 1.0f)));
        views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, -1.0, 0.0), QVector3D(0.0f, 0.0f, -1.0f)));
    } else {
        views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, -1.0, 0.0), QVector3D(0.0f, 0.0f, -1.0f)));
        views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, 1.0, 0.0), QVector3D(0.0f, 0.0f, 1.0f)));
    }
    views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, 0.0, 1.0), QVector3D(0.0f, -1.0f, 0.0f)));
    views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, 0.0, -1.0), QVector3D(0.0f, -1.0f, 0.0f)));
    for (const auto face : QSSGRenderTextureCubeFaces) {
        rub->updateDynamicBuffer(uBuf, quint8(face) * ubufElementSize, 64, mvp.constData());
        rub->updateDynamicBuffer(uBuf, quint8(face) * ubufElementSize + 64, 64, views[quint8(face)].constData());
        rub->updateDynamicBuffer(uBufEnvMap, quint8(face) * ubufEnvMapElementSize, 4, &colorSpace);
    }
    cb->resourceUpdate(rub);

    for (const auto face : QSSGRenderTextureCubeFaces) {
        cb->beginPass(renderTargets[quint8(face)], QColor(0, 0, 0, 1), { 1.0f, 0 }, nullptr, context->commonPassFlags());
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
        QSSGRHICTX_STAT(context, beginRenderPass(renderTargets[quint8(face)]));

        // Execute render pass
        cb->setGraphicsPipeline(envMapPipeline);
        cb->setVertexInput(0, 1, &vbufBinding);
        cb->setViewport(QRhiViewport(0, 0, environmentMapSize.width(), environmentMapSize.height()));
        QVector<QPair<int, quint32>> dynamicOffset = {
            { 0, quint32(ubufElementSize * quint8(face)) },
            { 2, quint32(ubufEnvMapElementSize * quint8(face) )}
        };
        cb->setShaderResources(envMapSrb, 2, dynamicOffset.constData());
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderCall);
        cb->draw(36);
        QSSGRHICTX_STAT(context, draw(36, 1));
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderCall, 36llu | (1llu << 32), QByteArrayLiteral("environment_map"));

        cb->endPass();
        QSSGRHICTX_STAT(context, endRenderPass());
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QSSG_RENDERPASS_NAME("environment_map", 0, face));
    }
    cb->debugMarkEnd();
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("environment_cube_generation"));
    Q_TRACE(QSSG_renderPass_exit);

    if (!isRGBE) {
        // Generate mipmaps for envMap
        rub = rhi->nextResourceUpdateBatch();
        rub->generateMips(envCubeMap);
        cb->resourceUpdate(rub);
    }

    // Phase 2: Generate the pre-filtered environment cubemap
    cb->debugMarkBegin("Pre-filtered Environment Cubemap Generation");
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
    Q_TRACE(QSSG_renderPass_entry, QStringLiteral("Pre-filtered Environment Cubemap Generation"));
    QRhiTexture *preFilteredEnvCubeMap = rhi->newTexture(cubeTextureFormat, environmentMapSize, 1, QRhiTexture::RenderTarget | QRhiTexture::CubeMap| QRhiTexture::MipMapped);
    if (!preFilteredEnvCubeMap->create())
        qWarning("Failed to create Pre-filtered Environment Cube Map");
    preFilteredEnvCubeMap->setName(rtName);
    int mipmapCount = rhi->mipLevelsForSize(environmentMapSize);
    mipmapCount = qMin(mipmapCount, 6);  // don't create more than 6 mip levels
    QMap<int, QSize> mipLevelSizes;
    QMap<int, QVarLengthArray<QRhiTextureRenderTarget *, 6>> renderTargetsMap;
    QRhiRenderPassDescriptor *renderPassDescriptorPhase2 = nullptr;

    // Create a renderbuffer for each mip level
    for (int mipLevel = 0; mipLevel < mipmapCount; ++mipLevel) {
        const QSize levelSize = QSize(environmentMapSize.width() * std::pow(0.5, mipLevel),
                                      environmentMapSize.height() * std::pow(0.5, mipLevel));
        mipLevelSizes.insert(mipLevel, levelSize);
        // Setup Render targets (6 * mipmapCount)
        QVarLengthArray<QRhiTextureRenderTarget *, 6> renderTargets;
        for (const auto face : QSSGRenderTextureCubeFaces) {
            QRhiColorAttachment att(preFilteredEnvCubeMap);
            att.setLayer(quint8(face));
            att.setLevel(mipLevel);
            QRhiTextureRenderTargetDescription rtDesc;
            rtDesc.setColorAttachments({att});
            auto renderTarget = rhi->newTextureRenderTarget(rtDesc);
            renderTarget->setName(rtName + QByteArrayLiteral(" env prefilter mip/face: ")
                                  + QByteArray::number(mipLevel) + QByteArrayLiteral("/") + QSSGBaseTypeHelpers::displayName(face));
            renderTarget->setDescription(rtDesc);
            if (!renderPassDescriptorPhase2)
                renderPassDescriptorPhase2 = renderTarget->newCompatibleRenderPassDescriptor();
            renderTarget->setRenderPassDescriptor(renderPassDescriptorPhase2);
            if (!renderTarget->create())
                qWarning("Failed to build prefilter env map render target");
            renderTarget->deleteLater();
            renderTargets << renderTarget;
        }
        renderTargetsMap.insert(mipLevel, renderTargets);
        renderPassDescriptorPhase2->deleteLater();
    }

    // Load the prefilter shader stages
    const auto &prefilterShaderStages = shaderCache->getBuiltInRhiShaders().getRhienvironmentmapPreFilterShader(isRGBE);

    // Create a new Sampler
    const QSSGRhiSamplerDescription samplerMipMapDesc {
        QRhiSampler::Linear,
        QRhiSampler::Linear,
        QRhiSampler::Linear,
        QRhiSampler::ClampToEdge,
        QRhiSampler::ClampToEdge,
        QRhiSampler::Repeat
    };

    QRhiSampler *envMapCubeSampler = nullptr;
    // Only use mipmap interpoliation if not using RGBE
    if (!isRGBE)
        envMapCubeSampler = context->sampler(samplerMipMapDesc);
    else
        envMapCubeSampler = sampler;

    // Reuse Vertex Buffer from phase 1
    // Reuse UniformBuffer from phase 1 (for vertex shader)

    // UniformBuffer
    // float roughness;
    // float resolution;
    // float lodBias;
    // int sampleCount;
    // int distribution;

    int ubufPrefilterElementSize = rhi->ubufAligned(20);
    QRhiBuffer *uBufPrefilter = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufPrefilterElementSize * mipmapCount);
    uBufPrefilter->create();
    uBufPrefilter->deleteLater();

    // Shader Resource Bindings
    QRhiShaderResourceBindings *preFilterSrb = rhi->newShaderResourceBindings();
    preFilterSrb->setBindings({
                          QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage, uBuf, 128),
                          QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(2, QRhiShaderResourceBinding::FragmentStage, uBufPrefilter, 20),
                          QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, envCubeMap, envMapCubeSampler)
                      });
    preFilterSrb->create();
    preFilterSrb->deleteLater();

    // Pipeline
    QRhiGraphicsPipeline *prefilterPipeline = rhi->newGraphicsPipeline();
    prefilterPipeline->setCullMode(QRhiGraphicsPipeline::Front);
    prefilterPipeline->setFrontFace(QRhiGraphicsPipeline::CCW);
    prefilterPipeline->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
    prefilterPipeline->setShaderStages({
                            *prefilterShaderStages->vertexStage(),
                            *prefilterShaderStages->fragmentStage()
                        });
    // same as phase 1
    prefilterPipeline->setVertexInputLayout(inputLayout);
    prefilterPipeline->setShaderResourceBindings(preFilterSrb);
    prefilterPipeline->setRenderPassDescriptor(renderPassDescriptorPhase2);
    if (!prefilterPipeline->create()) {
        qWarning("failed to create pre-filter env map pipeline state");
        return false;
    }
    prefilterPipeline->deleteLater();

    // Uniform Data
    // set the roughness uniform buffer data
    rub = rhi->nextResourceUpdateBatch();
    const float resolution = environmentMapSize.width();
    const float lodBias = 0.0f;
    const int sampleCount = 1024;
    for (int mipLevel = 0; mipLevel < mipmapCount; ++mipLevel) {
        Q_ASSERT(mipmapCount - 2);
        const float roughness = float(mipLevel) / float(mipmapCount - 2);
        const int distribution = mipLevel == (mipmapCount - 1) ? 0 : 1; // last mip level is for irradiance
        rub->updateDynamicBuffer(uBufPrefilter, mipLevel * ubufPrefilterElementSize, 4, &roughness);
        rub->updateDynamicBuffer(uBufPrefilter, mipLevel * ubufPrefilterElementSize + 4, 4, &resolution);
        rub->updateDynamicBuffer(uBufPrefilter, mipLevel * ubufPrefilterElementSize + 4 + 4, 4, &lodBias);
        rub->updateDynamicBuffer(uBufPrefilter, mipLevel * ubufPrefilterElementSize + 4 + 4 + 4, 4, &sampleCount);
        rub->updateDynamicBuffer(uBufPrefilter, mipLevel * ubufPrefilterElementSize + 4 + 4 + 4 + 4, 4, &distribution);
    }

    cb->resourceUpdate(rub);

    // Render
    for (int mipLevel = 0; mipLevel < mipmapCount; ++mipLevel) {
        for (const auto face : QSSGRenderTextureCubeFaces) {
            cb->beginPass(renderTargetsMap[mipLevel][quint8(face)], QColor(0, 0, 0, 1), { 1.0f, 0 }, nullptr, context->commonPassFlags());
            QSSGRHICTX_STAT(context, beginRenderPass(renderTargetsMap[mipLevel][quint8(face)]));
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
            cb->setGraphicsPipeline(prefilterPipeline);
            cb->setVertexInput(0, 1, &vbufBinding);
            cb->setViewport(QRhiViewport(0, 0, mipLevelSizes[mipLevel].width(), mipLevelSizes[mipLevel].height()));
            QVector<QPair<int, quint32>> dynamicOffsets = {
                { 0, quint32(ubufElementSize * quint8(face)) },
                { 2, quint32(ubufPrefilterElementSize * mipLevel) }
            };
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderCall);

            cb->setShaderResources(preFilterSrb, 2, dynamicOffsets.constData());
            cb->draw(36);
            QSSGRHICTX_STAT(context, draw(36, 1));
            Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderCall, 36llu | (1llu << 32), QByteArrayLiteral("environment_map"));
            cb->endPass();
            QSSGRHICTX_STAT(context, endRenderPass());
            Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QSSG_RENDERPASS_NAME("environment_map", mipLevel, face));
        }
    }
    cb->debugMarkEnd();
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("environment_cube_prefilter"));
    Q_TRACE(QSSG_renderPass_exit);

    outTexture->m_texture = preFilteredEnvCubeMap;
    outTexture->m_mipmapCount = mipmapCount;
    return true;
}

bool QSSGBufferManager::setRhiTexture(QSSGRenderImageTexture &texture,
                                      const QSSGLoadedTexture *inTexture,
                                      MipMode inMipMode,
                                      CreateRhiTextureFlags inFlags,
                                      const QString &debugObjectName,
                                      bool *wasTextureCreated)
{
    Q_ASSERT(inMipMode != MipModeFollowRenderImage);
    QVarLengthArray<QRhiTextureUploadEntry, 16> textureUploads;
    int textureSampleCount = 1;
    QRhiTexture::Flags textureFlags;
    const bool checkTransp = inFlags.testFlag(ScanForTransparency);
    bool hasTransp = false;

    const auto &context = m_contextInterface->rhiContext();
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(context.get());
    auto *rhi = context->rhi();
    QRhiTexture::Format rhiFormat = toRhiFormat(inTexture->format.format);
    const QTextureFileData &texFileData = inTexture->textureFileData;
    QSize size = texFileData.isValid() ? texFileData.size() : QSize(inTexture->width, inTexture->height);
    int mipmapCount = texFileData.isValid() ? texFileData.numLevels() : 1;
    int depth = inFlags.testFlag(Texture3D) ? inTexture->depth : 0;
    bool generateMipmaps = false;

    if (size.isEmpty()) {
        qWarning() << "Could not use 0 sized texture";
        return false;
    } else if (!rhi->isTextureFormatSupported(rhiFormat)) {
        qWarning() << "Unsupported texture format" << rhiFormat;
        return false;
    }


    if (wasTextureCreated)
        *wasTextureCreated = false;

    if (texture.m_texture == nullptr) {
        if (inTexture->format.format == QSSGRenderTextureFormat::Format::RGBE8)
            texture.m_flags.setRgbe8(true);
        if (!inTexture->isSRGB)
            texture.m_flags.setLinear(true);
        if (inMipMode == MipModeBsdf && (inTexture->data || texFileData.isValid())) {
            // Before creating an environment map, check if the provided texture is a
            // pre-baked environment map
            if (texFileData.isValid() && texFileData.keyValueMetadata().contains("QT_IBL_BAKER_VERSION")) {
                Q_ASSERT(texFileData.numFaces() == 6);
                Q_ASSERT(texFileData.numLevels() >= 5);

                QRhiTexture *environmentCubeMap = rhi->newTexture(rhiFormat, size, 1, QRhiTexture::CubeMap | QRhiTexture::MipMapped);
                environmentCubeMap->setName(debugObjectName.toLatin1());
                environmentCubeMap->create();
                texture.m_texture = environmentCubeMap;
                rhiCtxD->registerTexture(texture.m_texture);
                if (wasTextureCreated)
                    *wasTextureCreated = true;
            // If we get this far then we need to create an environment map at runtime.
            } else if (createEnvironmentMap(inTexture, &texture, debugObjectName)) {
                rhiCtxD->registerTexture(texture.m_texture);
                if (wasTextureCreated)
                    *wasTextureCreated = true;
                return true;
            } else {
                qWarning() << "Failed to create environment map";
                return false;
            }
        } else {
            if (inMipMode == MipModeEnable && mipmapCount == 1) {
                textureFlags |= QRhiTexture::Flag::UsedWithGenerateMips;
                generateMipmaps = true;
                mipmapCount = rhi->mipLevelsForSize(size);
            }

            if (mipmapCount > 1)
                textureFlags |= QRhiTexture::Flag::MipMapped;

            if (inFlags.testFlag(CubeMap))
                textureFlags |= QRhiTexture::CubeMap;

            if (inFlags.testFlag(Texture3D) && depth > 0)
                texture.m_texture = rhi->newTexture(rhiFormat, size.width(), size.height(), depth, textureSampleCount, textureFlags);
            else
                texture.m_texture = rhi->newTexture(rhiFormat, size, textureSampleCount, textureFlags);

            texture.m_texture->setName(debugObjectName.toLatin1());
            texture.m_texture->create();
            rhiCtxD->registerTexture(texture.m_texture);
            if (wasTextureCreated)
                *wasTextureCreated = true;
        }
    }

    // Update resources
    if (inMipMode == MipModeBsdf && (inTexture->data || texFileData.isValid())) {
        if (texFileData.isValid() && texFileData.keyValueMetadata().contains("QT_IBL_BAKER_VERSION")) {
            const int faceCount = texFileData.numFaces();
            for (int layer = 0; layer < faceCount; ++layer) {
                for (int level = 0; level < mipmapCount; ++level) {
                    QRhiTextureSubresourceUploadDescription subDesc;
                    subDesc.setSourceSize(sizeForMipLevel(level, size));
                    subDesc.setData(texFileData.getDataView(level, layer).toByteArray());
                    textureUploads << QRhiTextureUploadEntry { layer, level, subDesc };
                }
            }

            QRhiTextureUploadDescription uploadDescription;
            uploadDescription.setEntries(textureUploads.cbegin(), textureUploads.cend());
            auto *rub = rhi->nextResourceUpdateBatch();
            rub->uploadTexture(texture.m_texture, uploadDescription);
            context->commandBuffer()->resourceUpdate(rub);
            texture.m_mipmapCount = mipmapCount;
            return true;
        }
    } else if (texFileData.isValid()) {
        int numFaces = 1;
        // Just having a container with 6 faces is not enough, we only treat it
        // as a cubemap if it was requested to be treated as such. Otherwise
        // only face 0 is used.
        if (texFileData.numFaces() == 6 && inFlags.testFlag(CubeMap))
            numFaces = 6;

        for (int level = 0; level < texFileData.numLevels(); ++level) {
            QRhiTextureSubresourceUploadDescription subDesc;
            subDesc.setSourceSize(sizeForMipLevel(level, size));
            for (int face = 0; face < numFaces; ++face) {
                subDesc.setData(texFileData.getDataView(level, face).toByteArray());
                textureUploads << QRhiTextureUploadEntry{ face, level, subDesc };
            }
        }
        if (checkTransp) {
            auto glFormat = texFileData.glInternalFormat() ? texFileData.glInternalFormat() : texFileData.glFormat();
            hasTransp = !QSGCompressedTexture::formatIsOpaque(glFormat);
        }
    } else if (inFlags.testFlag(Texture3D)) {
        // 3D textures are currently only setup via QQuick3DTextureData
        quint32 formatSize = (quint32)inTexture->format.getSizeofFormat();
        quint32 size2D = inTexture->width * inTexture->height * formatSize;
        if (inTexture->dataSizeInBytes >= (quint32)(size2D * inTexture->depth)) {
            for (int slice = 0; slice < inTexture->depth; ++slice) {
                QRhiTextureSubresourceUploadDescription sliceUpload((char *)inTexture->data + slice * size2D, size2D);
                textureUploads << QRhiTextureUploadEntry(slice, 0, sliceUpload);
            }
        } else {
            qWarning() << "Texture size set larger than the data";
        }
    } else {
        QRhiTextureSubresourceUploadDescription subDesc;
        if (!inTexture->image.isNull()) {
            subDesc.setImage(inTexture->image);
            if (checkTransp)
                hasTransp = QImageData::get(inTexture->image)->checkForAlphaPixels();
        } else if (inTexture->data) {
            QByteArray buf(static_cast<const char *>(inTexture->data), qMax(0, int(inTexture->dataSizeInBytes)));
            subDesc.setData(buf);
            if (checkTransp)
                hasTransp = inTexture->scanForTransparency();
        }
        subDesc.setSourceSize(size);
        if (!subDesc.data().isEmpty() || !subDesc.image().isNull())
            textureUploads << QRhiTextureUploadEntry{0, 0, subDesc};
    }

    static const auto textureSizeWarning = [](QSize requestedSize, qsizetype maxSize) {
        return QStringLiteral("Requested texture width and height (%1x%2) exceeds the maximum allowed size (%3)!")
                .arg(requestedSize.width()).arg(requestedSize.height()).arg(maxSize);
    };
    static auto maxTextureSize = rhi->resourceLimit(QRhi::ResourceLimit::TextureSizeMax);
    const auto validTexSize = size.width() <= maxTextureSize && size.height() <= maxTextureSize;
    QSSG_ASSERT_X(validTexSize, qPrintable(textureSizeWarning(size, maxTextureSize)), return false);

    QSSG_ASSERT(texture.m_texture != nullptr, return false);

    if (checkTransp)
        texture.m_flags.setHasTransparency(hasTransp);

    QRhiTextureUploadDescription uploadDescription;
    uploadDescription.setEntries(textureUploads.cbegin(), textureUploads.cend());
    auto *rub = rhi->nextResourceUpdateBatch(); // TODO: optimize
    rub->uploadTexture(texture.m_texture, uploadDescription);
    if (generateMipmaps)
        rub->generateMips(texture.m_texture);
    context->commandBuffer()->resourceUpdate(rub);

    texture.m_mipmapCount = mipmapCount;
    return true;
}

QString QSSGBufferManager::primitivePath(const QString &primitive)
{
    QByteArray theName = primitive.toUtf8();
    for (size_t idx = 0; idx < nPrimitives; ++idx) {
        if (primitives[idx].primitive == theName) {
            QString pathBuilder = QString::fromLatin1(primitivesDirectory);
            pathBuilder += QLatin1String(primitives[idx].file);
            return pathBuilder;
        }
    }
    return {};
}

QMutex *QSSGBufferManager::meshUpdateMutex()
{
    return &meshBufferMutex;
}

QSSGMesh::Mesh QSSGBufferManager::loadPrimitive(const QString &inRelativePath)
{
    QString path = primitivePath(inRelativePath);
    const quint32 id = 1;
    QSharedPointer<QIODevice> device(QSSGInputUtil::getStreamForFile(path));
    if (device) {
        QSSGMesh::Mesh mesh = QSSGMesh::Mesh::loadMesh(device.data(), id);
        if (mesh.isValid())
            return mesh;
    }

    qCCritical(INTERNAL_ERROR, "Unable to find mesh primitive %s", qPrintable(path));
    return QSSGMesh::Mesh();
}

QSSGRenderMesh *QSSGBufferManager::loadMesh(const QSSGRenderModel &model)
{
    // When baking lightmaps we need to make sure that the original mesh is loaded instead of the already baked mesh.

    QSSGMeshProcessingOptions options;
    QSSGRenderMesh *theMesh = nullptr;

    if (model.hasLightmap() && !currentlyLightmapBaking && validateLightmap()) {
        options.lightmapPath = lightmapSource;
        options.lightmapKey = model.lightmapKey;
    }

    if (model.meshPath.isNull() && model.geometry) {
        theMesh = loadRenderMesh(model.geometry, options);
    } else {
        theMesh = loadRenderMesh(model.meshPath, options);
    }

    return theMesh;
}

QSSGMesh::Mesh QSSGBufferManager::loadLightmapMesh(const QSSGRenderModel &model)
{
    // When baking lightmaps we need to make sure that the original mesh is loaded instead of the already baked mesh.
    if (model.hasLightmap() && !currentlyLightmapBaking && validateLightmap() ) {
        auto [meshLightmap, _] = loadFromLightmapFile(lightmapSource, model.lightmapKey);
        if (meshLightmap.isValid())
            return meshLightmap;
    }

    return {};
}

QSSGBounds3 QSSGBufferManager::getModelBounds(const QSSGRenderModel *model)
{
    QSSGBounds3 retval;

    if (model->hasLightmap() && !currentlyLightmapBaking && validateLightmap()) {
        auto [meshLightmap, _] = loadFromLightmapFile(lightmapSource, model->lightmapKey);
        if (meshLightmap.isValid()) {
            const QVector<QSSGMesh::Mesh::Subset> subsets = meshLightmap.subsets();
            for (const QSSGMesh::Mesh::Subset &subset : std::as_const(subsets)) {
                retval.include(QSSGBounds3(subset.bounds.min, subset.bounds.max));
            }
            return retval;
        } else {
            qWarning() << "Could not load lightmap" << lightmapSource << model->lightmapKey;
        }
    }

    // Custom Geometry
    if (model->geometry) {
        retval = QSSGBounds3(model->geometry->boundsMin(), model->geometry->boundsMax());
    } else if (!model->meshPath.isNull()){
        // Check if the Mesh is already loaded
        QSSGRenderMesh *theMesh = nullptr;
        auto key = std::make_pair(model->meshPath, QString());
        auto meshItr = meshMap.constFind(key);
        if (meshItr != meshMap.cend())
            theMesh = meshItr.value().mesh;
        if (theMesh) {
            // The mesh was already loaded, so calculate the
            // bounds from subsets of the QSSGRenderMesh
            const auto &subSets = theMesh->subsets;
            for (const auto &subSet : subSets)
                retval.include(subSet.bounds);
        } else {
            // The model has not been loaded yet, load it without uploading the geometry
            // TODO: Try to do this without loading the whole mesh struct
            QSSGMesh::Mesh mesh = loadMeshData(model->meshPath);
            if (mesh.isValid()) {
               auto const &subsets = mesh.subsets();
               for (const auto &subset : subsets)
                   retval.include(QSSGBounds3(subset.bounds.min, subset.bounds.max));
            }
        }
    }
    return retval;
}

QSSGRenderMesh *QSSGBufferManager::createRenderMesh(const QSSGMesh::Mesh &mesh, const QString &debugObjectName)
{
    QSSGRenderMesh *newMesh = new QSSGRenderMesh(QSSGRenderDrawMode(mesh.drawMode()),
                                                 QSSGRenderWinding(mesh.winding()));
    const QSSGMesh::Mesh::VertexBuffer vertexBuffer = mesh.vertexBuffer();
    const QSSGMesh::Mesh::IndexBuffer indexBuffer = mesh.indexBuffer();
    const QSSGMesh::Mesh::TargetBuffer targetBuffer = mesh.targetBuffer();

    QSSGRenderComponentType indexBufComponentType = QSSGRenderComponentType::UnsignedInt16;
    QRhiCommandBuffer::IndexFormat rhiIndexFormat = QRhiCommandBuffer::IndexUInt16;
    if (!indexBuffer.data.isEmpty()) {
        indexBufComponentType = QSSGRenderComponentType(indexBuffer.componentType);
        const quint32 sizeofType = quint32(QSSGBaseTypeHelpers::getSizeOfType(indexBufComponentType));
        if (sizeofType == 2 || sizeofType == 4) {
            // Ensure type is unsigned; else things will fail in rendering pipeline.
            if (indexBufComponentType == QSSGRenderComponentType::Int16)
                indexBufComponentType = QSSGRenderComponentType::UnsignedInt16;
            if (indexBufComponentType == QSSGRenderComponentType::Int32)
                indexBufComponentType = QSSGRenderComponentType::UnsignedInt32;
            rhiIndexFormat = indexBufComponentType == QSSGRenderComponentType::UnsignedInt32
                    ? QRhiCommandBuffer::IndexUInt32 : QRhiCommandBuffer::IndexUInt16;
        } else {
            Q_ASSERT(false);
        }
    }

    struct {
        QSSGRhiBufferPtr vertexBuffer;
        QSSGRhiBufferPtr indexBuffer;
        QSSGRhiInputAssemblerState ia;
        QRhiTexture *targetsTexture = nullptr;
    } rhi;

    QRhiResourceUpdateBatch *rub = meshBufferUpdateBatch();
    const auto &context = m_contextInterface->rhiContext();
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(context.get());
    rhi.vertexBuffer = std::make_shared<QSSGRhiBuffer>(*context.get(),
                                                       QRhiBuffer::Static,
                                                       QRhiBuffer::VertexBuffer,
                                                       vertexBuffer.stride,
                                                       vertexBuffer.data.size());
    rhi.vertexBuffer->buffer()->setName(debugObjectName.toLatin1()); // this is what shows up in DebugView
    rub->uploadStaticBuffer(rhi.vertexBuffer->buffer(), vertexBuffer.data);

    if (!indexBuffer.data.isEmpty()) {
        rhi.indexBuffer = std::make_shared<QSSGRhiBuffer>(*context.get(),
                                                          QRhiBuffer::Static,
                                                          QRhiBuffer::IndexBuffer,
                                                          0,
                                                          indexBuffer.data.size(),
                                                          rhiIndexFormat);
        rub->uploadStaticBuffer(rhi.indexBuffer->buffer(), indexBuffer.data);
    }

    if (!targetBuffer.data.isEmpty()) {
        const int arraySize = targetBuffer.entries.size() * targetBuffer.numTargets;
        const int numTexels = (targetBuffer.data.size() / arraySize) >> 4; // byte size to vec4
        const int texWidth = qCeil(qSqrt(numTexels));
        const QSize texSize(texWidth, texWidth);
        if (!rhi.targetsTexture) {
            rhi.targetsTexture = context->rhi()->newTextureArray(QRhiTexture::RGBA32F, arraySize, texSize);
            rhi.targetsTexture->create();
            rhiCtxD->registerTexture(rhi.targetsTexture);
        } else if (rhi.targetsTexture->pixelSize() != texSize
                || rhi.targetsTexture->arraySize() != arraySize) {
            rhi.targetsTexture->setPixelSize(texSize);
            rhi.targetsTexture->setArraySize(arraySize);
            rhi.targetsTexture->create();
        }

        const quint32 layerSize = texWidth * texWidth * 4 * 4;
        for (int arrayId = 0; arrayId < arraySize; ++arrayId) {
            QRhiTextureSubresourceUploadDescription targetDesc(targetBuffer.data + arrayId * layerSize, layerSize);
            QRhiTextureUploadDescription desc(QRhiTextureUploadEntry(arrayId, 0, targetDesc));
            rub->uploadTexture(rhi.targetsTexture, desc);
        }

        for (quint32 entryIdx = 0, entryEnd = targetBuffer.entries.size(); entryIdx < entryEnd; ++entryIdx) {
            const char *nameStr = targetBuffer.entries[entryIdx].name.constData();
            if (!strcmp(nameStr, QSSGMesh::MeshInternal::getPositionAttrName())) {
                rhi.ia.targetOffsets[QSSGRhiInputAssemblerState::PositionSemantic] = entryIdx * targetBuffer.numTargets;
            } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getNormalAttrName())) {
                rhi.ia.targetOffsets[QSSGRhiInputAssemblerState::NormalSemantic] = entryIdx * targetBuffer.numTargets;
            } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getUV0AttrName())) {
                rhi.ia.targetOffsets[QSSGRhiInputAssemblerState::TexCoord0Semantic] = entryIdx * targetBuffer.numTargets;
            } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getUV1AttrName())) {
                rhi.ia.targetOffsets[QSSGRhiInputAssemblerState::TexCoord1Semantic] = entryIdx * targetBuffer.numTargets;
            } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getTexTanAttrName())) {
                rhi.ia.targetOffsets[QSSGRhiInputAssemblerState::TangentSemantic] = entryIdx * targetBuffer.numTargets;
            } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getTexBinormalAttrName())) {
                rhi.ia.targetOffsets[QSSGRhiInputAssemblerState::BinormalSemantic] = entryIdx * targetBuffer.numTargets;
            } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getColorAttrName())) {
                rhi.ia.targetOffsets[QSSGRhiInputAssemblerState::ColorSemantic] = entryIdx * targetBuffer.numTargets;
            }
        }
        rhi.ia.targetCount = targetBuffer.numTargets;
    } else if (rhi.targetsTexture) {
        rhiCtxD->releaseTexture(rhi.targetsTexture);
        rhi.targetsTexture = nullptr;
        rhi.ia.targetOffsets = { UINT8_MAX, UINT8_MAX, UINT8_MAX, UINT8_MAX,
                              UINT8_MAX, UINT8_MAX, UINT8_MAX };
        rhi.ia.targetCount = 0;
    }

    QVector<QSSGRenderVertexBufferEntry> entryBuffer;
    entryBuffer.resize(vertexBuffer.entries.size());
    for (quint32 entryIdx = 0, entryEnd = vertexBuffer.entries.size(); entryIdx < entryEnd; ++entryIdx)
        entryBuffer[entryIdx] = vertexBuffer.entries[entryIdx].toRenderVertexBufferEntry();

    QVarLengthArray<QRhiVertexInputAttribute, 4> inputAttrs;
    for (quint32 entryIdx = 0, entryEnd = entryBuffer.size(); entryIdx < entryEnd; ++entryIdx) {
        const QSSGRenderVertexBufferEntry &vbe(entryBuffer[entryIdx]);
        const int binding = 0;
        const int location = 0; // for now, will be resolved later, hence the separate inputLayoutInputNames list
        const QRhiVertexInputAttribute::Format format = QSSGRhiHelpers::toVertexInputFormat(vbe.m_componentType, vbe.m_numComponents);
        const int offset = int(vbe.m_firstItemOffset);

        bool ok = true;
        const char *nameStr = vbe.m_name.constData();
        if (!strcmp(nameStr, QSSGMesh::MeshInternal::getPositionAttrName())) {
            rhi.ia.inputs << QSSGRhiInputAssemblerState::PositionSemantic;
        } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getNormalAttrName())) {
            rhi.ia.inputs << QSSGRhiInputAssemblerState::NormalSemantic;
        } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getUV0AttrName())) {
            rhi.ia.inputs << QSSGRhiInputAssemblerState::TexCoord0Semantic;
        } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getUV1AttrName())) {
            rhi.ia.inputs << QSSGRhiInputAssemblerState::TexCoord1Semantic;
        } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getLightmapUVAttrName())) {
            rhi.ia.inputs << QSSGRhiInputAssemblerState::TexCoordLightmapSemantic;
        } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getTexTanAttrName())) {
            rhi.ia.inputs << QSSGRhiInputAssemblerState::TangentSemantic;
        } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getTexBinormalAttrName())) {
            rhi.ia.inputs << QSSGRhiInputAssemblerState::BinormalSemantic;
        } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getColorAttrName())) {
            rhi.ia.inputs << QSSGRhiInputAssemblerState::ColorSemantic;
        } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getJointAttrName())) {
            rhi.ia.inputs << QSSGRhiInputAssemblerState::JointSemantic;
        } else if (!strcmp(nameStr, QSSGMesh::MeshInternal::getWeightAttrName())) {
            rhi.ia.inputs << QSSGRhiInputAssemblerState::WeightSemantic;
        } else {
            qWarning("Unknown vertex input %s in mesh", nameStr);
            ok = false;
        }
        if (ok) {
            QRhiVertexInputAttribute inputAttr(binding, location, format, offset);
            inputAttrs.append(inputAttr);
        }
    }
    rhi.ia.inputLayout.setAttributes(inputAttrs.cbegin(), inputAttrs.cend());
    rhi.ia.inputLayout.setBindings({ vertexBuffer.stride });
    rhi.ia.topology = QSSGRhiHelpers::toTopology(QSSGRenderDrawMode(mesh.drawMode()));

    if (rhi.ia.topology == QRhiGraphicsPipeline::TriangleFan && !context->rhi()->isFeatureSupported(QRhi::TriangleFanTopology))
        qWarning("Mesh topology is TriangleFan but this is not supported with the active graphics API. Rendering will be incorrect.");

    QVector<QSSGMesh::Mesh::Subset> meshSubsets = mesh.subsets();
    for (quint32 subsetIdx = 0, subsetEnd = meshSubsets.size(); subsetIdx < subsetEnd; ++subsetIdx) {
        QSSGRenderSubset subset;
        const QSSGMesh::Mesh::Subset &source(meshSubsets[subsetIdx]);
        subset.bounds = QSSGBounds3(source.bounds.min, source.bounds.max);
        subset.count = source.count;
        subset.offset = source.offset;
        for (auto &lod : source.lods)
            subset.lods.append(QSSGRenderSubset::Lod({lod.count, lod.offset, lod.distance}));


        if (rhi.vertexBuffer) {
            subset.rhi.vertexBuffer = rhi.vertexBuffer;
            subset.rhi.ia = rhi.ia;
        }
        if (rhi.indexBuffer)
            subset.rhi.indexBuffer = rhi.indexBuffer;
        if (rhi.targetsTexture)
            subset.rhi.targetsTexture = rhi.targetsTexture;

        newMesh->subsets.push_back(subset);
    }

    if (!meshSubsets.isEmpty())
        newMesh->lightmapSizeHint = meshSubsets.first().lightmapSizeHint;

    return newMesh;
}

void QSSGBufferManager::releaseGeometry(QSSGRenderGeometry *geometry, const QString &lightmapSource)
{
    QMutexLocker meshMutexLocker(&meshBufferMutex);
    auto key = std::make_pair(geometry, lightmapSource);
    const auto meshItr = customMeshMap.constFind(key);
    if (meshItr != customMeshMap.cend()) {
        if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
            qDebug() << "- releaseGeometry: " << geometry << currentLayer;
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DCustomMeshLoad);
        Q_TRACE_SCOPE(QSSG_customMeshUnload);
        decreaseMemoryStat(meshItr.value().mesh);
        QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(m_contextInterface->rhiContext().get());
        rhiCtxD->releaseMesh(meshItr.value().mesh);
        customMeshMap.erase(meshItr);
        Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DCustomMeshLoad,
                                           stats.meshDataSize, geometry->profilingId);
    }
}

void QSSGBufferManager::releaseTextureData(const QSSGRenderTextureData *data)
{
    QVarLengthArray<CustomImageCacheKey, 4> keys;
    for (auto it = customTextureMap.cbegin(), end = customTextureMap.cend(); it != end; ++it) {
        if (it.key().data == data)
            keys.append(it.key());
    }
    for (const CustomImageCacheKey &key : keys)
        releaseTextureData(key);
}

void QSSGBufferManager::releaseTextureData(const CustomImageCacheKey &key)
{
    const auto textureDataItr = customTextureMap.constFind(key);
    if (textureDataItr != customTextureMap.cend()) {
        auto rhiTexture = textureDataItr.value().renderImageTexture.m_texture;
        if (rhiTexture) {
            if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                qDebug() << "- releaseTextureData: " << rhiTexture << currentLayer;
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DTextureLoad);
            Q_TRACE_SCOPE(QSSG_textureUnload);
            decreaseMemoryStat(rhiTexture);
            QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(m_contextInterface->rhiContext().get());
            rhiCtxD->releaseTexture(rhiTexture);
            Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DTextureLoad,
                                               stats.imageDataSize, 0);

        }
        customTextureMap.erase(textureDataItr);
    }
}

void QSSGBufferManager::releaseExtensionResult(const QSSGRenderExtension &rext)
{
    renderExtensionTexture.remove(&rext);
}

void QSSGBufferManager::releaseUserRenderPass(const QSSGRenderUserPass &upass)
{
    // Go through the user pass managers and tell them to unschedule this pass
    std::vector<QSSGUserRenderPassManagerWeakPtr> activeManagers;
    activeManagers.reserve(userRenderPassManagers.size());
    for (const auto &wmptr : std::as_const(userRenderPassManagers)) {
        if (const auto &mng = wmptr.lock()) {
            mng->unscheduleUserPass(&upass);
            activeManagers.push_back(wmptr);
        }
    }

    userRenderPassManagers = std::move(activeManagers);
}

void QSSGBufferManager::releaseMesh(const QSSGRenderPath &inSourcePath)
{
    QMutexLocker meshMutexLocker(&meshBufferMutex);
    auto key = std::make_pair(inSourcePath, lightmapSource);
    const auto meshItr = meshMap.constFind(key);
    if (meshItr != meshMap.cend()) {
        if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
            qDebug() << "- releaseMesh: " << inSourcePath.path() << currentLayer;
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DMeshLoad);
        Q_TRACE_SCOPE(QSSG_meshUnload);
        decreaseMemoryStat(meshItr.value().mesh);
        QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(m_contextInterface->rhiContext().get());
        rhiCtxD->releaseMesh(meshItr.value().mesh);
        meshMap.erase(meshItr);
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DMeshLoad,
                                           stats.meshDataSize, inSourcePath.path().toUtf8());
    }
}

void QSSGBufferManager::releaseImage(const ImageCacheKey &key)
{
    const auto imageItr = imageMap.constFind(key);
    if (imageItr != imageMap.cend()) {
        auto rhiTexture = imageItr.value().renderImageTexture.m_texture;
        if (rhiTexture) {
            if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                qDebug() << "- releaseTexture: " << key.path.path() << currentLayer;
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DTextureLoad);
            Q_TRACE_SCOPE(QSSG_textureUnload);
            decreaseMemoryStat(rhiTexture);
            QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(m_contextInterface->rhiContext().get());
            rhiCtxD->releaseTexture(rhiTexture);
            Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DTextureLoad,
                                               stats.imageDataSize, key.path.path().toUtf8());
        }
        imageMap.erase(imageItr);
    }
}

bool QSSGBufferManager::validateLightmap()
{
    if (lightmapSourceDirty) {
        lightmapSourceDirty = false;
        if (lightmapSource.isEmpty()) {
            lightmapFileValid = false;
        } else {
            QSharedPointer<QSSGLightmapLoader> loader = QSSGLightmapLoader::open(lightmapSource);
            lightmapFileValid = loader != nullptr;
            if (!lightmapFileValid)
                qCWarning(WARNING, "Lightmaps are disabled.");
        }
    }
    return lightmapFileValid;
}

void QSSGBufferManager::cleanupUnreferencedBuffers(quint32 frameId, QSSGRenderLayer *currentLayer)
{
    Q_UNUSED(currentLayer);

    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(m_contextInterface->rhiContext().get());

    // Don't cleanup if
    if (frameId == frameCleanupIndex)
        return;

    auto isUnused = [] (const QHash<QSSGRenderLayer*, uint32_t> &usages) -> bool {
        for (const auto &value : std::as_const(usages))
            if (value != 0)
                return false;
        return true;
    };

    {
        QMutexLocker meshMutexLocker(&meshBufferMutex);
        // Meshes (by path)
        auto meshIterator = meshMap.cbegin();
        while (meshIterator != meshMap.cend()) {
            if (isUnused(meshIterator.value().usageCounts)) {
                if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                    qDebug() << "- releaseGeometry: " << meshIterator.key().first.path() << currentLayer;
                decreaseMemoryStat(meshIterator.value().mesh);
                rhiCtxD->releaseMesh(meshIterator.value().mesh);
                meshIterator = meshMap.erase(meshIterator);
            } else {
                ++meshIterator;
            }
        }

        // Meshes (custom)
        auto customMeshIterator = customMeshMap.cbegin();
        while (customMeshIterator != customMeshMap.cend()) {
            if (isUnused(customMeshIterator.value().usageCounts)) {
                if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                   qDebug() << "- releaseGeometry: " << customMeshIterator.key() << currentLayer;
                decreaseMemoryStat(customMeshIterator.value().mesh);
                rhiCtxD->releaseMesh(customMeshIterator.value().mesh);
                customMeshIterator = customMeshMap.erase(customMeshIterator);
            } else {
                ++customMeshIterator;
            }
        }
    }

    // SG Textures
    auto sgIterator = qsgImageMap.cbegin();
    while (sgIterator != qsgImageMap.cend()) {
        if (isUnused(sgIterator.value().usageCounts)) {
            // Texture is no longer uses, so stop tracking
            // We do not need to delete/release the texture
            // because we don't own it.
            sgIterator = qsgImageMap.erase(sgIterator);
        } else {
            ++sgIterator;
        }
    }

    // Images
    auto imageKeyIterator = imageMap.cbegin();
    while (imageKeyIterator != imageMap.cend()) {
        if (isUnused(imageKeyIterator.value().usageCounts)) {
            auto rhiTexture = imageKeyIterator.value().renderImageTexture.m_texture;
            if (rhiTexture) {
                if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                   qDebug() << "- releaseTexture: " << imageKeyIterator.key().path.path() << currentLayer;
                decreaseMemoryStat(rhiTexture);
                rhiCtxD->releaseTexture(rhiTexture);
            }
            imageKeyIterator = imageMap.erase(imageKeyIterator);
        } else {
            ++imageKeyIterator;
        }
    }

    // Custom Texture Data
    auto textureDataIterator = customTextureMap.cbegin();
    while (textureDataIterator != customTextureMap.cend()) {
        if (isUnused(textureDataIterator.value().usageCounts)) {
            auto rhiTexture = textureDataIterator.value().renderImageTexture.m_texture;
            if (rhiTexture) {
                if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                   qDebug() << "- releaseTextureData: " << rhiTexture << currentLayer;
                decreaseMemoryStat(rhiTexture);
                rhiCtxD->releaseTexture(rhiTexture);
            }
            textureDataIterator = customTextureMap.erase(textureDataIterator);
        } else {
            ++textureDataIterator;
        }
    }

    // Textures from render extensions
    auto renderExtensionTextureKeyIterator = renderExtensionTexture.cbegin();
    while (renderExtensionTextureKeyIterator != renderExtensionTexture.cend()) {
        // We do not own the textures, so we just keep track of usage, but
        // if the texture is no longer valid it means that the extension
        // has unregistered it, so we can remove it from the map.
        auto rhiTexture = renderExtensionTextureKeyIterator.value().renderImageTexture.m_texture;
        if (!rhiTexture)
            renderExtensionTextureKeyIterator = renderExtensionTexture.erase(renderExtensionTextureKeyIterator);
        else
            ++renderExtensionTextureKeyIterator;
    }

    // Resource Tracking Debug Code
    frameCleanupIndex = frameId;
    if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Usage)) {
        qDebug() << "QSSGBufferManager::cleanupUnreferencedBuffers()" << this << "frame:" << frameCleanupIndex << currentLayer;
        qDebug() << "Textures(by path): " << imageMap.count();
        qDebug() << "Textures(custom):  " << customTextureMap.count();
        qDebug() << "Textures(Extension)" << renderExtensionTexture.count();
        qDebug() << "Textures(qsg):     " << qsgImageMap.count();
        qDebug() << "Geometry(by path): " << meshMap.count();
        qDebug() << "Geometry(custom):  " << customMeshMap.count();
    }
}

void QSSGBufferManager::resetUsageCounters(quint32 frameId, QSSGRenderLayer *layer)
{
    currentLayer = layer;
    if (frameResetIndex == frameId)
        return;

    // SG Textures
    for (auto &imageData : qsgImageMap)
        imageData.usageCounts[layer] = 0;

    // Images
    for (auto &imageData : imageMap)
        imageData.usageCounts[layer] = 0;

    // TextureDatas
    for (auto &imageData : customTextureMap)
        imageData.usageCounts[layer] = 0;
    // Meshes
    for (auto &meshData : meshMap)
        meshData.usageCounts[layer] = 0;

    // Meshes (custom)
    for (auto &meshData : customMeshMap)
        meshData.usageCounts[layer] = 0;

    // Textures from render extensions
    // NOTE: We retain the usage count as 1 as long as there is a texture
    // registered by a extension.
    for (auto &retData : renderExtensionTexture) {
        const bool hasTexture = (retData.renderImageTexture.m_texture != nullptr);
        retData.usageCounts[layer] = uint32_t(hasTexture) * 1;
    }

    frameResetIndex = frameId;
}

void QSSGBufferManager::registerMeshData(const QString &assetId, const QVector<QSSGMesh::Mesh> &meshData)
{
    auto it = g_assetMeshMap->find(assetId);
    if (it != g_assetMeshMap->end())
        ++it->ref;
    else
        g_assetMeshMap->insert(assetId, { meshData, 1 });
}

void QSSGBufferManager::unregisterMeshData(const QString &assetId)
{
    auto it = g_assetMeshMap->find(assetId);
    if (it != g_assetMeshMap->end() && (--it->ref == 0))
        g_assetMeshMap->erase(AssetMeshMap::const_iterator(it));
}

QSSGRenderMesh *QSSGBufferManager::loadRenderMesh(const QSSGRenderPath &inMeshPath, QSSGMeshProcessingOptions options)
{
    if (inMeshPath.isNull())
        return nullptr;

    auto key = std::make_pair(inMeshPath, options.lightmapPath);

    // check if it is already loaded
    auto meshItr = meshMap.find(key);
    if (meshItr != meshMap.cend()) {
        if (options.isCompatible(meshItr.value().options)) {
            meshItr.value().usageCounts[currentLayer]++;
            return meshItr.value().mesh;
        } else {
            // Re-Insert the mesh with a new name and a "zero" usage count, this will cause the
            // mesh to be released before the next frame starts.
            auto *mesh = meshItr->mesh;
            meshMap.erase(meshItr);
            auto keyReaped = std::make_pair(QSSGRenderPath(inMeshPath.path() + u"@reaped"), QString());
            meshMap.insert(keyReaped, { mesh, { { currentLayer, 0 } }, 0, { } });
        }
    }

    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DMeshLoad);
    Q_TRACE_SCOPE(QSSG_meshLoadPath, inMeshPath.path());

    auto [mesh, debugObjectName] = loadFromLightmapFile(options.lightmapPath, options.lightmapKey);

    if (!mesh.isValid()) {
        mesh = loadMeshData(inMeshPath);
        debugObjectName = QFileInfo(inMeshPath.path()).fileName();
    }

    if (!mesh.isValid()) {
        qCWarning(WARNING, "Failed to load mesh: %s", qPrintable(inMeshPath.path()));
        Q_QUICK3D_PROFILE_END_WITH_PAYLOAD(QQuick3DProfiler::Quick3DMeshLoad,
                                           stats.meshDataSize);
        return nullptr;
    }
    if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
        qDebug() << "+ uploadGeometry: " << inMeshPath.path() << currentLayer;

    auto ret = createRenderMesh(mesh, debugObjectName);
    meshMap.insert(key, { ret, { { currentLayer, 1 } }, 0, options });
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(m_contextInterface->rhiContext().get());
    rhiCtxD->registerMesh(ret);
    increaseMemoryStat(ret);
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DMeshLoad,
                                       stats.meshDataSize, inMeshPath.path().toUtf8());
    return ret;
}

QSSGRenderMesh *QSSGBufferManager::loadRenderMesh(QSSGRenderGeometry *geometry, QSSGMeshProcessingOptions options)
{
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(m_contextInterface->rhiContext().get());
    auto key = std::make_pair(geometry, options.lightmapPath);
    auto meshIterator = customMeshMap.find(key);
    if (meshIterator == customMeshMap.end()) {
        meshIterator = customMeshMap.insert(key, MeshData());
    } else if (geometry->generationId() != meshIterator->generationId || !options.isCompatible(meshIterator->options)) {
        // Release old data
        releaseGeometry(geometry, options.lightmapPath);
        meshIterator = customMeshMap.insert(key, MeshData());
    } else {
        // An up-to-date mesh was found
        meshIterator.value().usageCounts[currentLayer]++;
        return meshIterator.value().mesh;
    }

    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DCustomMeshLoad);
    Q_TRACE_SCOPE(QSSG_customMeshLoad);

    auto [mesh, debugObjectName] = loadFromLightmapFile(options.lightmapPath, options.lightmapKey);

    // We have either a valid lightmap mesh or a geometry with data
    if (!geometry->meshData().m_vertexBuffer.isEmpty() || mesh.isValid()) {
        QString error;

        if (!mesh.isValid()) {
            mesh = QSSGMesh::Mesh::fromRuntimeData(geometry->meshData(), &error);
            debugObjectName = geometry->debugObjectName;
        }

        if (mesh.isValid()) {
            if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                qDebug() << "+ uploadGeometry: " << geometry << currentLayer;
            meshIterator->mesh = createRenderMesh(mesh, debugObjectName);
            meshIterator->usageCounts[currentLayer] = 1;
            meshIterator->generationId = geometry->generationId();
            meshIterator->options = options;
            rhiCtxD->registerMesh(meshIterator->mesh);
            increaseMemoryStat(meshIterator->mesh);
        } else {
            qWarning("Mesh building failed: %s", qPrintable(error));
        }
    }
    // else an empty mesh is not an error, leave the QSSGRenderMesh null, it will not be rendered then

    Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DCustomMeshLoad,
                                       stats.meshDataSize, geometry->profilingId);
    return meshIterator->mesh;
}

std::unique_ptr<QSSGMeshBVH> QSSGBufferManager::loadMeshBVH(const QSSGRenderPath &inSourcePath)
{
    const QSSGMesh::Mesh mesh = loadMeshData(inSourcePath);
    if (!mesh.isValid()) {
        qCWarning(WARNING, "Failed to load mesh: %s", qPrintable(inSourcePath.path()));
        return nullptr;
    }
    QSSGMeshBVHBuilder meshBVHBuilder(mesh);
    return meshBVHBuilder.buildTree();
}

std::unique_ptr<QSSGMeshBVH> QSSGBufferManager::loadMeshBVH(QSSGRenderGeometry *geometry)
{
    if (!geometry)
        return nullptr;

    // We only support generating a BVH with Triangle primitives
    if (geometry->primitiveType() != QSSGMesh::Mesh::DrawMode::Triangles)
        return nullptr;

    // Build BVH
    bool hasIndexBuffer = false;
    QSSGRenderComponentType indexBufferFormat = QSSGRenderComponentType::UnsignedInt32;
    bool hasUV = false;
    int uvOffset = -1;
    int posOffset = -1;

    for (int i = 0; i < geometry->attributeCount(); ++i) {
        auto attribute = geometry->attribute(i);
        if (attribute.semantic == QSSGMesh::RuntimeMeshData::Attribute::PositionSemantic) {
            posOffset = attribute.offset;
        } else if (attribute.semantic == QSSGMesh::RuntimeMeshData::Attribute::TexCoord0Semantic) {
            hasUV = true;
            uvOffset = attribute.offset;
        } else if (!hasUV && attribute.semantic == QSSGMesh::RuntimeMeshData::Attribute::TexCoord1Semantic) {
            hasUV = true;
            uvOffset = attribute.offset;
        } else if (attribute.semantic == QSSGMesh::RuntimeMeshData::Attribute::IndexSemantic) {
            hasIndexBuffer = true;
            indexBufferFormat = attribute.componentType;
            if (indexBufferFormat != QSSGRenderComponentType::Int16
                && indexBufferFormat != QSSGRenderComponentType::Int32
                && indexBufferFormat != QSSGRenderComponentType::UnsignedInt16
                && indexBufferFormat != QSSGRenderComponentType::UnsignedInt32) {
                qWarning() << "Unsupported index buffer format for geometry";
                return nullptr;
            }
        }
    }

    QSSGMeshBVHBuilder meshBVHBuilder(geometry->vertexBuffer(),
                                      geometry->stride(),
                                      posOffset,
                                      hasUV,
                                      uvOffset,
                                      hasIndexBuffer,
                                      geometry->indexBuffer(),
                                      indexBufferFormat);
    return meshBVHBuilder.buildTree();
}

std::unique_ptr<QSSGMeshBVH> QSSGBufferManager::loadMeshBVH(const QSSGMesh::Mesh &mesh)
{
    QSSGMeshBVHBuilder meshBVHBuilder(mesh);
    return meshBVHBuilder.buildTree();
}

QSSGMesh::Mesh QSSGBufferManager::loadMeshData(const QSSGRenderPath &inMeshPath)
{
    QSSGMesh::Mesh result;

    // check to see if this is a primitive mesh
    if (inMeshPath.path().startsWith(QChar::fromLatin1('#')))
        result = loadPrimitive(inMeshPath.path());

    // check if this is an imported mesh. Expected path format: !name@path_to_asset
    if (!result.isValid() && inMeshPath.path().startsWith(u'!')) {
        const auto &[idx, assetId] = splitRuntimeMeshPath(inMeshPath);
        if (idx >= 0) {
            const auto ait = g_assetMeshMap->constFind(assetId);
            if (ait != g_assetMeshMap->constEnd()) {
                const auto &meshes = ait->meshes;
                if (idx < meshes.size())
                    result = ait->meshes.at(idx);
            }
        } else {
            qWarning("Unexpected mesh path!");
        }
    }

    // Attempt a load from the filesystem otherwise.
    if (!result.isValid()) {
        QString pathBuilder = inMeshPath.path();
        int poundIndex = pathBuilder.lastIndexOf(QChar::fromLatin1('#'));
        quint32 id = 0;
        if (poundIndex != -1) {
            id = QStringView(pathBuilder).mid(poundIndex + 1).toUInt();
            pathBuilder = pathBuilder.left(poundIndex);
        }
        if (!pathBuilder.isEmpty()) {
            QSharedPointer<QIODevice> device(QSSGInputUtil::getStreamForFile(pathBuilder));
            if (device) {
                QSSGMesh::Mesh mesh = QSSGMesh::Mesh::loadMesh(device.data(), id);
                if (mesh.isValid())
                    result = mesh;
            }
        }
    }

    return result;
}

QSSGMesh::Mesh QSSGBufferManager::loadMeshData(const QSSGRenderGeometry *geometry)
{
    QString error;
    QSSGMesh::Mesh mesh = QSSGMesh::Mesh::fromRuntimeData(geometry->meshData(), &error);
    if (!mesh.isValid())
        qWarning("loadMeshDataForCustomMeshUncached failed: %s", qPrintable(error));

    return mesh;
}

void QSSGBufferManager::registerExtensionResult(const QSSGRenderExtension &extensions,
                                                QRhiTexture *texture)
{
    if (texture) {
        const bool isMipMapped = texture->flags().testFlag(QRhiTexture::Flag::MipMapped);
        const auto mipLevels = isMipMapped ? QRhi::mipLevelsForSize(texture->pixelSize()) : 0;
        QSSGRenderImageTextureFlags flags;
        const bool isSRGB = texture->flags().testFlag(QRhiTexture::Flag::sRGB);
        flags.setLinear(!isSRGB);
        const bool isRGBA8 = (texture->format() == QRhiTexture::Format::RGBA8);
        flags.setRgbe8(isRGBA8);
        renderExtensionTexture.insert(&extensions, ImageData { QSSGRenderImageTexture{ texture, mipLevels, flags }, {}, 0 /* version */ });
    } else {
        renderExtensionTexture.insert(&extensions, {});
    }
}

void QSSGBufferManager::clear()
{
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(m_contextInterface->rhiContext().get());

    if (meshBufferUpdates) {
        meshBufferUpdates->release();
        meshBufferUpdates = nullptr;
    }

    {
        QMutexLocker meshMutexLocker(&meshBufferMutex);
        // Meshes (by path)
        auto meshMapCopy = meshMap;
        meshMapCopy.detach();
        for (auto iter = meshMapCopy.begin(), end = meshMapCopy.end(); iter != end; ++iter) {
            QSSGRenderMesh *theMesh = iter.value().mesh;
            if (theMesh) {
                if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                    qDebug() << "- releaseGeometry: " << iter.key().first.path() << currentLayer;
                decreaseMemoryStat(theMesh);
                rhiCtxD->releaseMesh(theMesh);
            }
        }
        meshMap.clear();

        // Meshes (custom)
        auto customMeshMapCopy = customMeshMap;
        customMeshMapCopy.detach();
        for (auto iter = customMeshMapCopy.begin(), end = customMeshMapCopy.end(); iter != end; ++iter) {
            QSSGRenderMesh *theMesh = iter.value().mesh;
            if (theMesh) {
                if (QSSGBufferManagerStat::enabled(QSSGBufferManagerStat::Level::Debug))
                    qDebug() << "- releaseGeometry: " << iter.key() << currentLayer;
                decreaseMemoryStat(theMesh);
                rhiCtxD->releaseMesh(theMesh);
            }
        }
        customMeshMap.clear();
    }

    // Textures (by path)
    for (auto it = imageMap.constBegin(), end = imageMap.constEnd(); it != end; ++it)
        releaseImage(it.key());

    imageMap.clear();

    // Textures (custom)
    for (auto it = customTextureMap.cbegin(), end = customTextureMap.cend(); it != end; ++it)
        releaseTextureData(it.key());

    customTextureMap.clear();

    // Textures (QSG)
    // these don't have any owned objects to release so just clearing is fine.
    qsgImageMap.clear();

    // To allow trying to read the lightmap file again
    lightmapSourceDirty = true;
}

QRhiResourceUpdateBatch *QSSGBufferManager::meshBufferUpdateBatch()
{
    if (!meshBufferUpdates)
        meshBufferUpdates = m_contextInterface->rhiContext()->rhi()->nextResourceUpdateBatch();
    return meshBufferUpdates;
}

void QSSGBufferManager::commitBufferResourceUpdates()
{
    if (meshBufferUpdates) {
        m_contextInterface->rhiContext()->commandBuffer()->resourceUpdate(meshBufferUpdates);
        meshBufferUpdates = nullptr;
    }
}

void QSSGBufferManager::processResourceLoader(const QSSGRenderResourceLoader *loader)
{
    for (auto &mesh : std::as_const(loader->meshes))
        loadRenderMesh(mesh, {});

    for (auto customMesh : std::as_const(loader->geometries))
        loadRenderMesh(static_cast<QSSGRenderGeometry*>(customMesh), {});

    for (auto texture : std::as_const(loader->textures)) {
        const auto image = static_cast<QSSGRenderImage *>(texture);
        loadRenderImage(image);
    }

    // Make sure the uploads occur
    commitBufferResourceUpdates();
}

static inline quint32 textureFormatSize(QRhiTexture::Format format)
{
    switch (format) {
    case QRhiTexture::UnknownFormat:
        return 0;
    case QRhiTexture::RGBA8:
        return 4;
    case QRhiTexture::BGRA8:
        return 4;
    case QRhiTexture::R8:
        return 1;
    case QRhiTexture::RG8:
        return 2;
    case QRhiTexture::R16:
        return 2;
    case QRhiTexture::RG16:
        return 4;
    case QRhiTexture::RED_OR_ALPHA8:
        return 1;

    case QRhiTexture::RGBA16F:
        return 8;
    case QRhiTexture::RGBA32F:
        return 16;
    case QRhiTexture::R16F:
        return 2;
    case QRhiTexture::R32F:
        return 4;

    case QRhiTexture::RGB10A2:
        return 4;

    case QRhiTexture::R8SI:
        return 1;
    case QRhiTexture::R32SI:
        return 4;
    case QRhiTexture::RG32SI:
        return 8;
    case QRhiTexture::RGBA32SI:
        return 16;

    case QRhiTexture::R8UI:
        return 1;
    case QRhiTexture::R32UI:
        return 4;
    case QRhiTexture::RG32UI:
        return 8;
    case QRhiTexture::RGBA32UI:
        return 16;

    case QRhiTexture::D16:
        return 2;
    case QRhiTexture::D24:
        return 4;
    case QRhiTexture::D24S8:
        return 4;
    case QRhiTexture::D32F:
        return 4;
    case QRhiTexture::D32FS8:
        return 8;

    case QRhiTexture::BC1:
        return 8;
    case QRhiTexture::BC2:
        return 16;
    case QRhiTexture::BC3:
        return 16;
    case QRhiTexture::BC4:
        return 8;
    case QRhiTexture::BC5:
        return 16;
    case QRhiTexture::BC6H:
        return 16;
    case QRhiTexture::BC7:
        return 16;

    case QRhiTexture::ETC2_RGB8:
        return 8;
    case QRhiTexture::ETC2_RGB8A1:
        return 8;
    case QRhiTexture::ETC2_RGBA8:
        return 16;

    case QRhiTexture::ASTC_4x4:
    case QRhiTexture::ASTC_5x4:
    case QRhiTexture::ASTC_5x5:
    case QRhiTexture::ASTC_6x5:
    case QRhiTexture::ASTC_6x6:
    case QRhiTexture::ASTC_8x5:
    case QRhiTexture::ASTC_8x6:
    case QRhiTexture::ASTC_8x8:
    case QRhiTexture::ASTC_10x5:
    case QRhiTexture::ASTC_10x6:
    case QRhiTexture::ASTC_10x8:
    case QRhiTexture::ASTC_10x10:
    case QRhiTexture::ASTC_12x10:
    case QRhiTexture::ASTC_12x12:
        return 16;
    }
    Q_UNREACHABLE_RETURN(0);
}

static inline bool isCompressedTextureFormat(QRhiTexture::Format format)
{
    return (format >= QRhiTexture::BC1);
}

static inline quint64 textureMemorySize(QRhiTexture *texture)
{
    quint64 s = 0;
    if (!texture)
        return s;

    auto format = texture->format();
    if (format == QRhiTexture::UnknownFormat)
        return 0;

    s = texture->pixelSize().width() * texture->pixelSize().height();

    const quint32 bytesPerPixel = textureFormatSize(format);
    QSSG_ASSERT_X(bytesPerPixel > 0, "Invalid texture format size", return 0);

    if (!isCompressedTextureFormat(format)) {
        s *= bytesPerPixel;
    } else {
        s /= bytesPerPixel;
    }

    if (texture->flags() & QRhiTexture::MipMapped)
        s += s / 4;
    if (texture->flags() & QRhiTexture::CubeMap)
        s *= 6;
    return s;
}

static inline quint64 bufferMemorySize(const QSSGRhiBufferPtr &buffer)
{
    quint64 s = 0;
    if (!buffer)
        return s;
    s = buffer->buffer()->size();
    return s;
}

void QSSGBufferManager::increaseMemoryStat(QRhiTexture *texture)
{
    stats.imageDataSize += textureMemorySize(texture);
    QSSGRhiContextStats::get(*m_contextInterface->rhiContext()).imageDataSizeChanges(stats.imageDataSize);
}

void QSSGBufferManager::decreaseMemoryStat(QRhiTexture *texture)
{
    stats.imageDataSize = qMax(0u, stats.imageDataSize - textureMemorySize(texture));
    QSSGRhiContextStats::get(*m_contextInterface->rhiContext()).imageDataSizeChanges(stats.imageDataSize);
}

void QSSGBufferManager::increaseMemoryStat(QSSGRenderMesh *mesh)
{
    stats.meshDataSize += bufferMemorySize(mesh->subsets.at(0).rhi.vertexBuffer)
            + bufferMemorySize(mesh->subsets.at(0).rhi.indexBuffer);
    QSSGRhiContextStats::get(*m_contextInterface->rhiContext()).meshDataSizeChanges(stats.meshDataSize);
}

void QSSGBufferManager::decreaseMemoryStat(QSSGRenderMesh *mesh)
{
    quint64 s = 0;
    if (mesh)
    s = bufferMemorySize(mesh->subsets.at(0).rhi.vertexBuffer)
            + bufferMemorySize(mesh->subsets.at(0).rhi.indexBuffer);
    stats.meshDataSize = qMax(0u, stats.meshDataSize - s);
    QSSGRhiContextStats::get(*m_contextInterface->rhiContext()).meshDataSizeChanges(stats.meshDataSize);
}

void QSSGBufferManager::setLightmapSource(const QString &source)
{
    if (lightmapSource != source) {
        lightmapSource = source;
        lightmapSourceDirty = true;
    }
}

void QSSGBufferManager::setCurrentlyLightmapBaking(bool value)
{
    currentlyLightmapBaking = value;
}

void QSSGBufferManager::registerUserRenderPassManager(const QSSGUserRenderPassManagerPtr &userPassManager)
{
    userRenderPassManagers.push_back(userPassManager);
}

size_t qHash(const QSSGBufferManager::CustomImageCacheKey &k, size_t seed) noexcept
{
    // NOTE: The data pointer should never be null, as null data pointers shouldn't be inserted into
    //       the cached (make sure to check that before inserting!!!)
    using MipMap_t = std::underlying_type_t<QSSGBufferManager::MipMode>;
    return qHash(*k.data, seed) ^ MipMap_t(k.mipMode);
}

QT_END_NAMESPACE
