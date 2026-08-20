// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGIMAGE_H
#define QSSGIMAGE_H

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

#include <QtQuick3D/qquick3dobject.h>
#include <QtQuick3D/QQuick3DTextureData>
#include <QtQuick/private/qquickitemchangelistener_p.h>
#include <QtQuick/private/qsgadaptationlayer_p.h>
#include <QtQuick/QQuickItem>
#include <QtQuick/QSGNode>
#include <QtCore/QUrl>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QSGLayer;
struct QSSGRenderImage;
class QQuick3DRenderExtension;
class QQuickWindow;

class Q_QUICK3D_EXPORT QQuick3DTexture : public QQuick3DObject, public QQuickItemChangeListener
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QQuickItem *sourceItem READ sourceItem WRITE setSourceItem NOTIFY sourceItemChanged)
    Q_PROPERTY(QQuick3DTextureData *textureData READ textureData WRITE setTextureData NOTIFY textureDataChanged)
    Q_PROPERTY(QQuick3DRenderExtension *textureProvider READ textureProvider WRITE setTextureProvider NOTIFY textureProviderChanged FINAL REVISION(6, 7))
    Q_PROPERTY(float scaleU READ scaleU WRITE setScaleU NOTIFY scaleUChanged)
    Q_PROPERTY(float scaleV READ scaleV WRITE setScaleV NOTIFY scaleVChanged)
    Q_PROPERTY(MappingMode mappingMode READ mappingMode WRITE setMappingMode NOTIFY mappingModeChanged)
    Q_PROPERTY(TilingMode tilingModeHorizontal READ horizontalTiling WRITE setHorizontalTiling NOTIFY horizontalTilingChanged)
    Q_PROPERTY(TilingMode tilingModeVertical READ verticalTiling WRITE setVerticalTiling NOTIFY verticalTilingChanged)
    Q_PROPERTY(TilingMode tilingModeDepth READ depthTiling WRITE setDepthTiling NOTIFY depthTilingChanged REVISION(6, 7))
    Q_PROPERTY(float rotationUV READ rotationUV WRITE setRotationUV NOTIFY rotationUVChanged)
    Q_PROPERTY(float positionU READ positionU WRITE setPositionU NOTIFY positionUChanged)
    Q_PROPERTY(float positionV READ positionV WRITE setPositionV NOTIFY positionVChanged)
    Q_PROPERTY(float pivotU READ pivotU WRITE setPivotU NOTIFY pivotUChanged)
    Q_PROPERTY(float pivotV READ pivotV WRITE setPivotV NOTIFY pivotVChanged)
    Q_PROPERTY(bool flipU READ flipU WRITE setFlipU NOTIFY flipUChanged)
    Q_PROPERTY(bool flipV READ flipV WRITE setFlipV NOTIFY flipVChanged)
    Q_PROPERTY(int indexUV READ indexUV WRITE setIndexUV NOTIFY indexUVChanged)
    Q_PROPERTY(Filter magFilter READ magFilter WRITE setMagFilter NOTIFY magFilterChanged)
    Q_PROPERTY(Filter minFilter READ minFilter WRITE setMinFilter NOTIFY minFilterChanged)
    Q_PROPERTY(Filter mipFilter READ mipFilter WRITE setMipFilter NOTIFY mipFilterChanged)
    Q_PROPERTY(bool generateMipmaps READ generateMipmaps WRITE setGenerateMipmaps NOTIFY generateMipmapsChanged)
    Q_PROPERTY(bool autoOrientation READ autoOrientation WRITE setAutoOrientation NOTIFY autoOrientationChanged REVISION(6, 2))

    QML_NAMED_ELEMENT(Texture)

public:
    enum MappingMode
    {
        UV = 0,
        Environment = 1,
        LightProbe = 2,
    };
    Q_ENUM(MappingMode)

    enum TilingMode // must match QSSGRenderTextureCoordOp
    {
        ClampToEdge = 1,
        MirroredRepeat,
        Repeat
    };
    Q_ENUM(TilingMode)

    enum Filter { // must match QSSGRenderTextureFilterOp
        None = 0,
        Nearest,
        Linear
    };
    Q_ENUM(Filter)

    explicit QQuick3DTexture(QQuick3DObject *parent = nullptr);
    ~QQuick3DTexture() override;

    QUrl source() const;
    QQuickItem *sourceItem() const;
    float scaleU() const;
    float scaleV() const;
    MappingMode mappingMode() const;
    TilingMode horizontalTiling() const;
    TilingMode verticalTiling() const;
    Q_REVISION(6, 7) TilingMode depthTiling() const;
    float rotationUV() const;
    float positionU() const;
    float positionV() const;
    float pivotU() const;
    float pivotV() const;
    bool flipU() const;
    bool flipV() const;
    int indexUV() const;
    Filter magFilter() const;
    Filter minFilter() const;
    Filter mipFilter() const;
    QQuick3DTextureData *textureData() const;
    bool generateMipmaps() const;
    bool autoOrientation() const;

    QSSGRenderImage *getRenderImage();

    Q_REVISION(6, 7) QQuick3DRenderExtension *textureProvider() const;
    Q_REVISION(6, 7) void setTextureProvider(QQuick3DRenderExtension *newRenderTexture);

    bool hasSourceData() const
    {
        return !m_source.isEmpty() || m_sourceItem || m_textureData;
    }

public Q_SLOTS:
    void setSource(const QUrl &source);
    void setSourceItem(QQuickItem *sourceItem);
    void setScaleU(float scaleU);
    void setScaleV(float scaleV);
    void setMappingMode(QQuick3DTexture::MappingMode mappingMode);
    void setHorizontalTiling(QQuick3DTexture::TilingMode tilingModeHorizontal);
    void setVerticalTiling(QQuick3DTexture::TilingMode tilingModeVertical);
    Q_REVISION(6, 7) void setDepthTiling(QQuick3DTexture::TilingMode tilingModeDepth);
    void setRotationUV(float rotationUV);
    void setPositionU(float positionU);
    void setPositionV(float positionV);
    void setPivotU(float pivotU);
    void setPivotV(float pivotV);
    void setFlipU(bool flipU);
    void setFlipV(bool flipV);
    void setIndexUV(int indexUV);
    void setMagFilter(QQuick3DTexture::Filter magFilter);
    void setMinFilter(QQuick3DTexture::Filter minFilter);
    void setMipFilter(QQuick3DTexture::Filter mipFilter);
    void setTextureData(QQuick3DTextureData * textureData);
    void setGenerateMipmaps(bool generateMipmaps);
    void setAutoOrientation(bool autoOrientation);

Q_SIGNALS:
    void sourceChanged();
    void sourceItemChanged();
    void scaleUChanged();
    void scaleVChanged();
    void mappingModeChanged();
    void horizontalTilingChanged();
    void verticalTilingChanged();
    Q_REVISION(6, 7) void depthTilingChanged();
    void rotationUVChanged();
    void positionUChanged();
    void positionVChanged();
    void pivotUChanged();
    void pivotVChanged();
    void flipUChanged();
    void flipVChanged();
    void indexUVChanged();
    void magFilterChanged();
    void minFilterChanged();
    void mipFilterChanged();
    void textureDataChanged();
    void generateMipmapsChanged();
    void autoOrientationChanged();
    Q_REVISION(6, 7) void textureProviderChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void markAllDirty() override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;

    void itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF &geometry) override;

    explicit QQuick3DTexture(QQuick3DObjectPrivate &dd, QQuick3DObject *parent = nullptr);

private Q_SLOTS:
    void sourceItemDestroyed(QObject *item);
    void sourceItemWindowChanged(QQuickWindow *window);

private:
    enum class DirtyFlag {
        TransformDirty = (1 << 0),
        SourceDirty = (1 << 1),
        IndexUVDirty = (1 << 2),
        TextureDataDirty = (1 << 3),
        SamplerDirty = (1 << 4),
        SourceItemDirty = (1 << 5),
        FlipVDirty = (1 << 6),
        ExtensionDirty = (1 << 7)
    };
    Q_DECLARE_FLAGS(DirtyFlags, DirtyFlag)
    void markDirty(DirtyFlag type, bool requestSecondaryUpdate = false);
    void trySetSourceParent();
    bool effectiveFlipV(const QSSGRenderImage &imageNode) const;

    QUrl m_source;
    QQuickItem *m_sourceItem = nullptr;
    bool m_sourceItemReparented = false;
    bool m_sourceItemRefed = false;
    QSGLayer *m_layer = nullptr;
    float m_scaleU = 1.0f;
    float m_scaleV = 1.0f;
    MappingMode m_mappingMode = UV;
    TilingMode m_tilingModeHorizontal = Repeat;
    TilingMode m_tilingModeVertical = Repeat;
    TilingMode m_tilingModeDepth = Repeat;
    float m_rotationUV = 0;
    float m_positionU = 0;
    float m_positionV = 0;
    float m_pivotU = 0;
    float m_pivotV = 0;
    bool m_flipU = false;
    bool m_flipV = false;
    int m_indexUV = 0;
    Filter m_magFilter = Linear;
    Filter m_minFilter = Linear;
    Filter m_mipFilter = None;
    DirtyFlags m_dirtyFlags = DirtyFlags(DirtyFlag::TransformDirty)
                              | DirtyFlags(DirtyFlag::SourceDirty)
                              | DirtyFlags(DirtyFlag::IndexUVDirty)
                              | DirtyFlags(DirtyFlag::TextureDataDirty);
    QMetaObject::Connection m_textureProviderConnection;
    QMetaObject::Connection m_textureUpdateConnection;
    QQuick3DSceneManager *m_sceneManagerForLayer = nullptr;
    QMetaObject::Connection m_sceneManagerWindowChangeConnection;
    QQuickItem *m_initializedSourceItem = nullptr;
    QSizeF m_initializedSourceItemSize;
    QHash<QByteArray, QMetaObject::Connection> m_connections;
    QMetaObject::Connection m_textureDataConnection;
    QQuick3DTextureData *m_textureData = nullptr;
    bool m_generateMipmaps = false;
    bool m_autoOrientation = true;
    QQuick3DRenderExtension *m_renderExtension = nullptr;
};

QT_END_NAMESPACE

#endif // QSSGIMAGE_H
