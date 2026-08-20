// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGUSERRENDERPASSMANAGER_P_H
#define QSSGUSERRENDERPASSMANAGER_P_H

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

#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>

#include <QtCore/qlist.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QSSGRenderUserPass;

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGUserRenderPassManager : public std::enable_shared_from_this<QSSGUserRenderPassManager>
{
    enum class Private { Initialize };
public:
    // A set of user passes (no duplicates)
    using UserPassSet = QList<QSSGRenderUserPass *>;

    explicit QSSGUserRenderPassManager(Private);
    ~QSSGUserRenderPassManager();

    [[nodiscard]] static std::shared_ptr<QSSGUserRenderPassManager> create()
    {
        return std::make_shared<QSSGUserRenderPassManager>(Private::Initialize);
    }

    void scheduleUserPass(QSSGRenderUserPass *userPasses);
    void unscheduleUserPass(const QSSGRenderUserPass *userPasses);

    QSSGRhiRenderableTextureV2Ptr getOrCreateRenderableTexture(const QSSGRenderUserPass &userPass);
    QSSGRhiRenderableTextureV2Ptr getUserPassTexureResult(const QSSGRenderUserPass &userPass) const;

    // Should be called before scheduledUserPasses is called.
    void updateUserPassOrder(bool forceUpdate = false);

    const UserPassSet &scheduledUserPasses() const { return m_scheduledUserPasses; }

    void releaseAll();

    void unregisterManagedTexture(QSSGManagedRhiTexture *textureWrapper);
    void registerManagedTexture(QSSGManagedRhiTexture *textureWrapper);

    static constexpr size_t maxUserPassSlots() { return 16; }
    qsizetype acquireUserPassSlot();

    void resetForFrame();

private:
    Q_DISABLE_COPY_MOVE(QSSGUserRenderPassManager)

    friend class QSSGBufferManager;

    using TextureWrappers = std::vector<QSSGManagedRhiTexture *>;
    using DeferredReleaseTextures = QSet<QRhiTexture *>;

    // If return false the texture is not tracked and should be released by caller!!!
    [[nodiscard]] bool derefTexture(QRhiTexture *texture);
    void refTexture(QRhiTexture *texture);

    [[nodiscard]] bool derefTexture(const std::unique_ptr<QRhiTexture> &texture);
    void refTexture(const std::unique_ptr<QRhiTexture> &texture);

    QHash<const QSSGRenderUserPass *, QSSGRhiRenderableTextureV2Ptr> m_renderPassRenderTargets;
    UserPassSet m_scheduledUserPasses;

    QHash<QRhiTexture *, size_t> m_trackedTextures;

    // We keep a list of these so we can notify them that it's
    // time to release their textures.
    TextureWrappers m_trackedTextureWrappers;

    DeferredReleaseTextures m_deferredReleaseTextures;

    size_t m_userPassSlotsUsed = 0;

    bool m_passlistDirty = false;
};

using QSSGUserRenderPassManagerPtr = std::shared_ptr<QSSGUserRenderPassManager>;

QT_END_NAMESPACE

#endif // QSSGUSERRENDERPASSMANAGER_P_H
