// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssguserrenderpassmanager_p.h"
#include "graphobjects/qssgrenderuserpass_p.h"

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(QSSGUserRenderPassManagerLog, "qt.quick3d.runtimerender.userpassmanager")

QSSGUserRenderPassManager::QSSGUserRenderPassManager(Private)
{

}

QSSGUserRenderPassManager::~QSSGUserRenderPassManager()
{
    for (auto &rt : m_renderPassRenderTargets)
        rt.reset();

    const auto textures = m_trackedTextures.keys();
    qDeleteAll(textures);
}

void QSSGUserRenderPassManager::scheduleUserPass(QSSGRenderUserPass *userPasses)
{
    if (userPasses != nullptr) {
        auto it = std::find(m_scheduledUserPasses.begin(), m_scheduledUserPasses.end(), userPasses);
        if (it == m_scheduledUserPasses.end()) {
            m_scheduledUserPasses.push_back(userPasses);
            m_passlistDirty = true;
        }
    }
}

void QSSGUserRenderPassManager::unscheduleUserPass(const QSSGRenderUserPass *userPasses)
{
    if (userPasses != nullptr) {
        auto it = std::find(m_scheduledUserPasses.cbegin(), m_scheduledUserPasses.cend(), userPasses);
        if (it != m_scheduledUserPasses.cend()) {
            m_scheduledUserPasses.erase(it);
            m_passlistDirty = true;
        }
    }
}

QSSGRhiRenderableTextureV2Ptr QSSGUserRenderPassManager::getOrCreateRenderableTexture(const QSSGRenderUserPass &userPass)
{
    auto it = m_renderPassRenderTargets.find(&userPass);
    if (it != m_renderPassRenderTargets.end())
        return it.value();

    auto iter = m_renderPassRenderTargets.insert(&userPass, std::make_shared<QSSGRhiRenderableTextureV2>(shared_from_this(), QSSGRhiRenderableTextureV2::Private::Initialize));
    return iter.value();
}

QSSGRhiRenderableTextureV2Ptr QSSGUserRenderPassManager::getUserPassTexureResult(const QSSGRenderUserPass &userPass) const
{
    const auto foundIt = m_renderPassRenderTargets.find(&userPass);
    if (foundIt != m_renderPassRenderTargets.end())
        return foundIt.value();

    return {};
}

void QSSGUserRenderPassManager::updateUserPassOrder(bool forceUpdate)
{
    if (m_passlistDirty || forceUpdate) {
        std::sort(m_scheduledUserPasses.begin(), m_scheduledUserPasses.end(), [](const QSSGRenderUserPass *a, const QSSGRenderUserPass *b) {
            return a->m_dependencyIndex > b->m_dependencyIndex;
        });
    }

    m_passlistDirty = false;
}

bool QSSGUserRenderPassManager::derefTexture(QRhiTexture *texture)
{
    auto foundIt = m_trackedTextures.find(texture);
    const bool wasFound = foundIt != m_trackedTextures.constEnd();

    if (wasFound) {
        if (!(foundIt.value() > 1)) {
            m_trackedTextures.erase(foundIt);
            m_deferredReleaseTextures.insert(texture);
        } else {
            foundIt.value() -= 1;
        }
    } else {
        qWarning() << "QSSGUserRenderPassManager::derefTexture: Trying to deref a texture that is not tracked.";
    }

    return wasFound;
}

void QSSGUserRenderPassManager::refTexture(QRhiTexture *texture)
{
    auto it = m_trackedTextures.find(texture);
    if (it != m_trackedTextures.end())
        it.value() += 1;
    else
        m_trackedTextures.insert(texture, 1);
}

bool QSSGUserRenderPassManager::derefTexture(const std::unique_ptr<QRhiTexture> &texture)
{
    return derefTexture(texture.get());
}

void QSSGUserRenderPassManager::refTexture(const std::unique_ptr<QRhiTexture> &texture)
{
    refTexture(texture.get());
}

void QSSGUserRenderPassManager::unregisterManagedTexture(QSSGManagedRhiTexture *textureWrapper)
{
    QSSG_ASSERT(textureWrapper != nullptr, return);

    const auto foundIt = std::find(m_trackedTextureWrappers.cbegin(), m_trackedTextureWrappers.cend(), textureWrapper);
    if (foundIt != m_trackedTextureWrappers.cend()) {
        auto &texture = textureWrapper->texture();
        if (!derefTexture(texture))
            qWarning("QSSGSharedRhiTextureWrapper: Texture was not tracked by the manager, possible resource leak.");
        m_trackedTextureWrappers.erase(foundIt);
    }
}

void QSSGUserRenderPassManager::registerManagedTexture(QSSGManagedRhiTexture *textureWrapper)
{
    QSSG_ASSERT(textureWrapper != nullptr, return);

    const auto foundIt = std::find(m_trackedTextureWrappers.cbegin(), m_trackedTextureWrappers.cend(), textureWrapper);
    if (foundIt == m_trackedTextureWrappers.cend()) {
        refTexture(textureWrapper->texture());
        m_trackedTextureWrappers.push_back(textureWrapper);
    }
}

qsizetype QSSGUserRenderPassManager::acquireUserPassSlot()
{
    if (m_userPassSlotsUsed < maxUserPassSlots() - 1)
        return qsizetype(m_userPassSlotsUsed++);

    qWarning() << "Exceeded maximum number of user render passes(" << QSSGUserRenderPassManager::maxUserPassSlots() << ")";

    return -1;
}

void QSSGUserRenderPassManager::resetForFrame()
{
    qDeleteAll(m_deferredReleaseTextures.cbegin(), m_deferredReleaseTextures.cend());
    m_deferredReleaseTextures.clear();

    m_userPassSlotsUsed = 0;
}

void QSSGUserRenderPassManager::releaseAll()
{
    m_scheduledUserPasses.clear();

    // NOTE: We own these so we'll invalidate them, meaning releasing their internal resources.
    //       Any shared pointers to them held elsewhere will become invalid, as in they will
    //       get their internal QRhiTexture released and set to nullptr.
    for (auto &rt : m_renderPassRenderTargets)
        rt->invalidate();

    m_renderPassRenderTargets.clear();

    { // We make a copy since invalidate() will call back into us to unregister itself!
        auto cpy = m_trackedTextureWrappers;
        for (auto &tw : cpy)
            tw->invalidate();

        cpy.clear();
    }

    qDeleteAll(m_deferredReleaseTextures.cbegin(), m_deferredReleaseTextures.cend());
    m_deferredReleaseTextures.clear();

    Q_ASSERT(m_trackedTextureWrappers.size() == 0);

    if (m_trackedTextures.size() > 0) {
        qWarning() << "QSSGUserRenderPassManager::releaseAll: There are still tracked textures (" << m_trackedTextures.size() << ") when releasing all. This indicates a resource leak.";
        for (auto it = m_trackedTextures.constBegin(); it != m_trackedTextures.constEnd(); ++it)
            qCDebug(QSSGUserRenderPassManagerLog) << "Texture: " << it.key()->name() << ", has ref count: " << it.value();
    }
}

QT_END_NAMESPACE
