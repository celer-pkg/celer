// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QBATCHRENDERER_H
#define QBATCHRENDERER_H

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

#include <QHash>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

QT_BEGIN_NAMESPACE

class QLottieBase;
class QImage;
class QLottieAnimation;

class QBatchRenderer : public QThread
{
    Q_OBJECT

    struct Entry
    {
        QLottieAnimation* animator = nullptr;
        QLottieBase *lottieTreeBlueprint = nullptr;
        int startFrame = 0;
        int endFrame = 0;
        int currentFrame = 0;
        int animDir = 1;
        QHash<int, QLottieBase*> frameCache;
    };

public:
    ~QBatchRenderer() override;

    QBatchRenderer(QBatchRenderer const &) = delete;
    void operator=(QBatchRenderer const&) = delete;

    static QBatchRenderer *instance();
    static void deleteInstance();

    QLottieBase *getFrame(QLottieAnimation *animator, int frameNumber);

Q_SIGNALS:
    void frameReady(QLottieAnimation *animator, int frameNumber);

public Q_SLOTS:
    void registerAnimator(QLottieAnimation *animator);
    void deregisterAnimator(QLottieAnimation *animator);

    bool gotoFrame(QLottieAnimation *animator, int frame);

    void frameRendered(QLottieAnimation *animator, int frameNumber);

protected:
    void run() override;

    void prerender(Entry *animEntry);

private:
    QBatchRenderer();

    void pruneFrameCache(Entry* e);

private:
    static QBatchRenderer *m_rendererInstance;

    QMutex m_mutex;
    QWaitCondition m_waitCondition;

    int m_cacheSize = 2;
    QHash<QLottieAnimation *, Entry *> m_animData;
    int m_lastRenderedFrame = -1;
};

QT_END_NAMESPACE

#endif // QBATCHRENDERER_H
