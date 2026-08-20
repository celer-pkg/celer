// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtLottie/private/qbatchrenderer_p.h>

#include <QImage>
#include <QPainter>
#include <QHash>
#include <QMap>
#include <QMutexLocker>
#include <QLoggingCategory>
#include <QThread>

#include <QJsonDocument>
#include <QJsonArray>

#include <QtLottie/private/qlottieconstants_p.h>
#include <QtLottie/private/qlottiebase_p.h>
#include <QtLottie/private/qlottieflatlayers_p.h>
#include <QtLottie/private/qlottieroot_p.h>
#include <QtLottie/private/qlottielayer_p.h>

#include <QtLottie/private/qlottieanimation_p.h>
#include <QtLottie/private/qlottierasterrenderer_p.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcLottieQtLottieRenderThread, "qt.lottieqt.lottie.render.thread");

QBatchRenderer *QBatchRenderer::m_rendererInstance = nullptr;

QBatchRenderer::QBatchRenderer()
    : QThread()
{
    const QByteArray cacheStr = qgetenv("QLOTTIE_RENDER_CACHE_SIZE");
    int cacheSize = cacheStr.toInt();
    if (cacheSize > 0) {
        qCDebug(lcLottieQtLottieRenderThread) << "Setting frame cache size to" << cacheSize;
        m_cacheSize = cacheSize;
    }
}

QBatchRenderer::~QBatchRenderer()
{
    QMutexLocker mlocker(&m_mutex);

    for (Entry *entry : std::as_const(m_animData)) {
        qDeleteAll(entry->frameCache);
        delete entry->lottieTreeBlueprint;
        delete entry;
    }
}

QBatchRenderer *QBatchRenderer::instance()
{
    if (!m_rendererInstance)
        m_rendererInstance = new QBatchRenderer;

    return m_rendererInstance;
}

void QBatchRenderer::deleteInstance()
{
    delete m_rendererInstance;
    m_rendererInstance = nullptr;
}

void QBatchRenderer::registerAnimator(QLottieAnimation *animator)
{
    QMutexLocker mlocker(&m_mutex);

    qCDebug(lcLottieQtLottieRenderThread) << "Register Animator:"
                                       << static_cast<void*>(animator);

    Entry *&entry = m_animData[animator];
    if (entry) {
        qDeleteAll(entry->frameCache);
        delete entry->lottieTreeBlueprint;
        delete entry;
        entry = nullptr;
    }
    Q_ASSERT(entry == nullptr);
    entry = new Entry;
    entry->animator = animator;
    entry->startFrame = animator->startFrame();
    entry->endFrame = animator->endFrame();
    entry->currentFrame = animator->startFrame();
    entry->animDir = animator->direction();
    QLottieRoot *root = new QLottieRoot;
    int ret = root->parseSource(animator->jsonSource(), m_animData.keys().last()->source());
    if (ret == -1)
        quit();
    entry->lottieTreeBlueprint = root;
    m_waitCondition.wakeAll();
}

void QBatchRenderer::deregisterAnimator(QLottieAnimation *animator)
{
    QMutexLocker mlocker(&m_mutex);

    qCDebug(lcLottieQtLottieRenderThread) << "Deregister Animator:"
                                       << static_cast<void*>(animator);

    Entry *entry = m_animData.take(animator);
    if (entry) {
        qDeleteAll(entry->frameCache);
        delete entry->lottieTreeBlueprint;
        delete entry;
    }
}

bool QBatchRenderer::gotoFrame(QLottieAnimation *animator, int frame)
{
    QMutexLocker mlocker(&m_mutex);

    Entry *entry = m_animData.value(animator, nullptr);
    if (entry) {
        qCDebug(lcLottieQtLottieRenderThread) << "Animator:"
                                           << static_cast<void*>(animator)
                                           << "Goto frame:" << frame;
        entry->currentFrame = frame;
        entry->animDir = animator->direction();
        pruneFrameCache(entry);
        m_waitCondition.wakeAll();
        return true;
    }
    return false;
}

void QBatchRenderer::pruneFrameCache(Entry* e)
{
    QHash<int, QLottieBase*>::iterator removeCandidate = e->frameCache.end();
    if (e->frameCache.size() == m_cacheSize &&
            !e->frameCache.contains(e->currentFrame))
        removeCandidate = e->frameCache.begin();

    QHash<int, QLottieBase*>::iterator it = e->frameCache.begin();
    while (it != e->frameCache.end()) {
        int frame = it.key();
        if ((frame - e->currentFrame) * e->animDir >= 0) { // same frame or same direction
            if (removeCandidate != e->frameCache.end() &&
                    (removeCandidate.key() - frame) * e->animDir < 0)
                removeCandidate = it;
            ++it;
        } else {
            qCDebug(lcLottieQtLottieRenderThread) << "Animator:" << static_cast<void*>(e->animator)
                                                     << "Remove frame from cache" << frame;
            delete it.value();
            it = e->frameCache.erase(it);
            removeCandidate = e->frameCache.end();
        }
    }
    if (removeCandidate != e->frameCache.end()) {
        qCDebug(lcLottieQtLottieRenderThread) << "Animator:"
                                                 << static_cast<void*>(e->animator)
                                                 << "Remove frame from cache"
                                                 << removeCandidate.key()
                                                 << "(Reason - cache is full)";
        e->frameCache.erase(removeCandidate);
    }
    m_lastRenderedFrame = -1;
}

QLottieBase *QBatchRenderer::getFrame(QLottieAnimation *animator, int frameNumber)
{
    QMutexLocker mlocker(&m_mutex);

    Entry *entry = m_animData.value(animator, nullptr);
    if (entry)
        return entry->frameCache.value(frameNumber, nullptr);
    else
        return nullptr;
}

void QBatchRenderer::prerender(Entry *animEntry)
{
    while (animEntry->frameCache.size() < m_cacheSize) {
        if (m_lastRenderedFrame == animEntry->currentFrame)
            animEntry->currentFrame += animEntry->animDir;

        QLottieBase *&lottieTree = animEntry->frameCache[animEntry->currentFrame];
        if (lottieTree == nullptr) {
            lottieTree = new QLottieBase(*animEntry->lottieTreeBlueprint);

            for (QLottieBase *elem : lottieTree->children()) {
                if (elem->active(animEntry->currentFrame))
                    elem->updateProperties( animEntry->currentFrame);
            }
        }

        qCDebug(lcLottieQtLottieRenderThread) << "Animator:"
                                           << static_cast<void*>(animEntry->animator)
                                           << "Frame drawn to cache. FN:"
                                           << animEntry->currentFrame;
        emit frameReady(animEntry->animator,  animEntry->currentFrame);

        animEntry->currentFrame += animEntry->animDir;

        if (animEntry->currentFrame > animEntry->endFrame) {
            animEntry->currentFrame = animEntry->startFrame;
        } else if (animEntry->currentFrame < animEntry->startFrame) {
            animEntry->currentFrame = animEntry->endFrame;
        }
    }
}

void QBatchRenderer::frameRendered(QLottieAnimation *animator, int frameNumber)
{
    QMutexLocker mlocker(&m_mutex);

    Entry *entry = m_animData.value(animator, nullptr);
    if (entry) {
        qCDebug(lcLottieQtLottieRenderThread) << "Animator:" << static_cast<void*>(animator)
                                           << "Remove frame from cache" << frameNumber;

        QLottieBase *root = entry->frameCache.take(frameNumber);
        if (root != nullptr) {
            delete root;
            m_waitCondition.wakeAll();
        }
        m_lastRenderedFrame = frameNumber;
    }
}

void QBatchRenderer::run()
{
    qCDebug(lcLottieQtLottieRenderThread) << "rendering thread" << QThread::currentThread();

    while (!isInterruptionRequested()) {
        QMutexLocker mlocker(&m_mutex);

        for (Entry *e : std::as_const(m_animData))
            prerender(e);

        m_waitCondition.wait(&m_mutex);
    }
}

QT_END_NAMESPACE
