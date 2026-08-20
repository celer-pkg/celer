// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_BUFFER_MANAGER_H
#define QSSG_RENDER_BUFFER_MANAGER_H

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

#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimagetexture_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermesh_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendererutil_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssguserrenderpassmanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderuserpass_p.h>
#include <QtQuick3DUtils/private/qssgmesh_p.h>

#include <QtQuick3DUtils/private/qquick3dprofiler_p.h>

#include <QtCore/QMutex>
#include <QtCore/qhash.h>
#include <QtCore/qsize.h>

QT_BEGIN_NAMESPACE

struct QSSGRenderMesh;
struct QSSGLoadedTexture;
class QSSGRhiContext;
class QSSGMeshBVH;
class QSGTexture;
class QSSGRenderGeometry;
class QSSGRenderTextureData;
struct QSSGRenderModel;
struct QSSGRenderImage;
struct QSSGRenderResourceLoader;
struct QSSGRenderLayer;
struct QSSGRenderSkin;

// There is one QSSGBufferManager per QSSGRenderContextInterface, and so per
// QQuickWindow, and by extension, per scenegraph render thread. This is
// essential here because graphics resources (vertex/index buffers, textures)
// are always specific to one render thread, they cannot be used and shared
// between different threads (and so windows). This is ensured by design, by
// having a dedicated BufferManager for each render thread (window).

class QSSGRenderContextInterface;
class QQuick3DRenderExtension;

struct QSSGMeshProcessingOptions
{
    QString lightmapPath;
    QString lightmapKey;

    inline bool isCompatible(const QSSGMeshProcessingOptions &other) const
    {
        // if lightmap UVs are wanted, the request can only use other's data
        // if the lightmap key is identical
        return lightmapPath == other.lightmapPath && lightmapKey == other.lightmapKey;
    }
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGBufferManager
{
    Q_DISABLE_COPY(QSSGBufferManager)
public:
    enum MipMode : quint8 {
        MipModeFollowRenderImage,
        MipModeEnable,
        MipModeDisable,
        MipModeBsdf
    };

    enum LoadRenderImageFlag {
        LoadWithFlippedY = 0x01
    };
    Q_DECLARE_FLAGS(LoadRenderImageFlags, LoadRenderImageFlag)

    struct ImageCacheKey {
        QSSGRenderPath path;
        int mipMode;
        int type;
        QString key;
    };

    struct CustomImageCacheKey {
        QSSGRenderTextureData *data;
        QSize pixelSize;
        MipMode mipMode;
    };

    struct ImageData {
        QSSGRenderImageTexture renderImageTexture;
        QHash<QSSGRenderLayer*, uint32_t> usageCounts;
        uint32_t version = 0;
    };

    struct MeshData {
        QSSGRenderMesh *mesh = nullptr;
        QHash<QSSGRenderLayer*, uint32_t> usageCounts;
        uint32_t generationId = 0;
        QSSGMeshProcessingOptions options;
    };

    struct MemoryStats {
        quint64 meshDataSize = 0;
        quint64 imageDataSize = 0;
    };

    QSSGBufferManager();
    ~QSSGBufferManager();

    void setRenderContextInterface(QSSGRenderContextInterface *ctx);

    void releaseCachedResources();
    // called on the destuction of a layer to release its referenced resources
    void releaseResourcesForLayer(QSSGRenderLayer *layer);

    QSSGRenderImageTexture loadRenderImage(const QSSGRenderImage *image,
                                           MipMode inMipMode = MipModeFollowRenderImage,
                                           LoadRenderImageFlags flags = LoadWithFlippedY);
    QSSGRenderImageTexture loadTextureData(QSSGRenderTextureData *data, MipMode inMipMode);
    QSSGRenderImageTexture loadLightmap(const QSSGRenderModel &model);
    QSSGRenderImageTexture loadSkinmap(QSSGRenderTextureData *skin);

    QSSGRenderMesh *getMeshForPicking(const QSSGRenderModel &model, const QString &lightmap) const;
    QSSGBounds3 getModelBounds(const QSSGRenderModel *model);

    QSSGRenderMesh *loadMesh(const QSSGRenderModel &model);
    QSSGMesh::Mesh loadLightmapMesh(const QSSGRenderModel &model);

    // Called at the end of the frame to release unreferenced geometry and textures
    void cleanupUnreferencedBuffers(quint32 frameId, QSSGRenderLayer *layer);
    void resetUsageCounters(quint32 frameId, QSSGRenderLayer *layer);

    void releaseGeometry(QSSGRenderGeometry *geometry, const QString &lightmapSource = QString());
    void releaseTextureData(const QSSGRenderTextureData *data);
    void releaseTextureData(const CustomImageCacheKey &key);
    void releaseExtensionResult(const QSSGRenderExtension &rext);
    void releaseUserRenderPass(const QSSGRenderUserPass &upass);

    void commitBufferResourceUpdates();

    void processResourceLoader(const QSSGRenderResourceLoader *loader);

    static std::unique_ptr<QSSGMeshBVH> loadMeshBVH(const QSSGRenderPath &inSourcePath);
    static std::unique_ptr<QSSGMeshBVH> loadMeshBVH(QSSGRenderGeometry *geometry);
    static std::unique_ptr<QSSGMeshBVH> loadMeshBVH(const QSSGMesh::Mesh &mesh);

    static QSSGMesh::Mesh loadMeshData(const QSSGRenderPath &inSourcePath);
    QSSGMesh::Mesh loadMeshData(const QSSGRenderGeometry *geometry);

    void registerExtensionResult(const QSSGRenderExtension &extensions, QRhiTexture *texture);

    static QRhiTexture::Format toRhiFormat(const QSSGRenderTextureFormat format);

    static void registerMeshData(const QString &assetId, const QVector<QSSGMesh::Mesh> &meshData);
    static void unregisterMeshData(const QString &assetId);
    static QString runtimeMeshSourceName(const QString &assetId, qsizetype meshId);
    static QString primitivePath(const QString &primitive);

    QMutex *meshUpdateMutex();

    void increaseMemoryStat(QRhiTexture *texture);
    void decreaseMemoryStat(QRhiTexture *texture);
    void increaseMemoryStat(QSSGRenderMesh *mesh);
    void decreaseMemoryStat(QSSGRenderMesh *mesh);

    // Views for testing
    const QHash<ImageCacheKey, ImageData> &getImageMap() const { return imageMap; }
    const QHash<CustomImageCacheKey, ImageData> &getCustomTextureMap() const { return customTextureMap; }
    const QHash<QSGTexture *, ImageData> &getSGImageMap() const { return qsgImageMap; }
    const QHash<std::pair<QSSGRenderPath, QString>, MeshData> &getMeshMap() const { return meshMap; }
    const QHash<std::pair<QSSGRenderGeometry *, QString>, MeshData> &getCustomMeshMap() const { return customMeshMap; }

    void setLightmapSource(const QString &source);
    void setCurrentlyLightmapBaking(bool value);

    void registerUserRenderPassManager(const QSSGUserRenderPassManagerPtr &userPassManager);

private:
    void clear();
    QRhiResourceUpdateBatch *meshBufferUpdateBatch();

    static QSSGMesh::Mesh loadPrimitive(const QString &inRelativePath);
    enum CreateRhiTextureFlag {
        ScanForTransparency = 0x01,
        CubeMap = 0x02,
        Texture3D = 0x04
    };
    Q_DECLARE_FLAGS(CreateRhiTextureFlags, CreateRhiTextureFlag)
    bool setRhiTexture(QSSGRenderImageTexture &texture,
                       const QSSGLoadedTexture *inTexture,
                       MipMode inMipMode,
                       CreateRhiTextureFlags inFlags,
                       const QString &debugObjectName,
                       bool *wasTextureCreated = nullptr);

    QSSGRenderMesh *loadRenderMesh(const QSSGRenderPath &inSourcePath, QSSGMeshProcessingOptions options);
    QSSGRenderMesh *loadRenderMesh(QSSGRenderGeometry *geometry, QSSGMeshProcessingOptions options);

    QSSGRenderMesh *createRenderMesh(const QSSGMesh::Mesh &mesh, const QString &debugObjectName = {});
    bool createEnvironmentMap(const QSSGLoadedTexture *inImage, QSSGRenderImageTexture *outTexture, const QString &debugObjectName);

    void releaseMesh(const QSSGRenderPath &inSourcePath);
    void releaseImage(const ImageCacheKey &key);

    QSSGRenderContextInterface *m_contextInterface = nullptr; // ContextInterfaces owns BufferManager

    // These store the actual buffer handles
    QHash<ImageCacheKey, ImageData> imageMap;                   // Textures (specificed by path)
    QHash<CustomImageCacheKey, ImageData> customTextureMap;     // Textures (QQuick3DTextureData)
    QHash<QSGTexture *, ImageData> qsgImageMap;                 // Textures (from Qt Quick)
    QHash<const QSSGRenderExtension *, ImageData> renderExtensionTexture; // Textures (from QQuick3DRenderExtension)
    QHash<std::pair<QSSGRenderPath, QString>, MeshData> meshMap; // Meshes (specififed by path)
    QHash<std::pair<QSSGRenderGeometry *, QString>, MeshData> customMeshMap; // Meshes (QQuick3DGeometry)

    using QSSGUserRenderPassManagerWeakPtr = std::weak_ptr<QSSGUserRenderPassManager>;
    std::vector<QSSGUserRenderPassManagerWeakPtr> userRenderPassManagers;

    QRhiResourceUpdateBatch *meshBufferUpdates = nullptr;
    QMutex meshBufferMutex;

    quint32 frameCleanupIndex = 0;
    quint32 frameResetIndex = 0;
    QSSGRenderLayer *currentLayer = nullptr;
    MemoryStats stats;

    QString lightmapSource;
    bool lightmapFileValid = false;
    bool lightmapSourceDirty = true;
    bool currentlyLightmapBaking = false;
    bool validateLightmap();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSSGBufferManager::LoadRenderImageFlags)

inline size_t qHash(const QSSGBufferManager::ImageCacheKey &k, size_t seed) Q_DECL_NOTHROW
{
    return qHash(k.path, seed) ^ k.mipMode ^ k.type ^ qHash(k.key, seed);
}

inline bool operator==(const QSSGBufferManager::ImageCacheKey &a, const QSSGBufferManager::ImageCacheKey &b) Q_DECL_NOTHROW
{
    return a.path == b.path && a.mipMode == b.mipMode && a.type == b.type && a.key == b.key;
}

size_t qHash(const QSSGBufferManager::CustomImageCacheKey &k, size_t seed) noexcept;

inline bool operator==(const QSSGBufferManager::CustomImageCacheKey &a, const QSSGBufferManager::CustomImageCacheKey &b) Q_DECL_NOTHROW
{
    return a.data == b.data && a.pixelSize == b.pixelSize && a.mipMode == b.mipMode;
}

QT_END_NAMESPACE

#endif
