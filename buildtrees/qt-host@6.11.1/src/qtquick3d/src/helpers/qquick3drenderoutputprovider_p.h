// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DRENDEROUTPUTPROVIDER_P_H
#define QQUICK3DRENDEROUTPUTPROVIDER_P_H

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

#include <QtQuick3DHelpers/qtquick3dhelpersexports.h>
#include <QQuick3DTextureProviderExtension>
#include <QQmlEngine>

#include <QtQuick3D/private/qquick3drenderpass_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DHELPERS_EXPORT QQuick3DRenderOutputProvider : public QQuick3DTextureProviderExtension
{
    Q_OBJECT
    Q_PROPERTY(TextureSource textureSource READ textureSource WRITE setTextureSource NOTIFY textureSourceChanged FINAL)
    Q_PROPERTY(QQuick3DRenderPass *renderPass READ renderPass WRITE setRenderPass NOTIFY renderPassChanged FINAL)
    Q_PROPERTY(AttachmentSelector attachmentSelector READ attachmentSelector WRITE setAttachmentSelector NOTIFY attachmentSelectorChanged FINAL)
    QML_NAMED_ELEMENT(RenderOutputProvider)
    QML_ADDED_IN_VERSION(6, 11)
public:
    enum class TextureSource : quint32
    {
        None = 0,
        UserPassTexture = 1,
        AoTexture,
        DepthTexture,
        ScreenTexture,
        NormalTexture,
        MotionVectorTexture,
    };
    Q_ENUM(TextureSource)

    enum class AttachmentSelector : quint32
    {
        Attachment0 = 0,
        Attachment1 = 1,
        Attachment2 = 2,
        Attachment3 = 3,
    };
    Q_ENUM(AttachmentSelector)

    explicit QQuick3DRenderOutputProvider(QQuick3DObject *parent = nullptr);
    TextureSource textureSource() const;
    void setTextureSource(TextureSource newTextureSource);

    QQuick3DRenderPass *renderPass() const;
    void setRenderPass(QQuick3DRenderPass *newRenderPass);

    AttachmentSelector attachmentSelector() const;
    void setAttachmentSelector(AttachmentSelector newAttachmentSelector);

signals:
    void textureSourceChanged();
    void renderPassChanged();

    void attachmentSelectorChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void markAllDirty() override;

private:
    enum DirtyType : quint32 {
        TextureSourceDirty = 0x1,
        UserPassTextureDirty = 0x2,
    };

    using DirtyTypeT = std::underlying_type_t<DirtyType>;
    static constexpr DirtyTypeT AllDirty = 0xffffffff;

    QQuick3DRenderPass *m_renderPass = nullptr;

    TextureSource m_textureSource = TextureSource::None;
    DirtyTypeT m_dirtyAttributes = AllDirty;
    void markDirty(DirtyType type);
    QString m_userPassKey;
    AttachmentSelector m_attachmentSelector { AttachmentSelector::Attachment0 };
};

QT_END_NAMESPACE

#endif // QQUICK3DRENDEROUTPUTPROVIDER_P_H
