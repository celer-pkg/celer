// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GALLERYITEM_H
#define GALLERYITEM_H

#include <QtCanvasPainter/qcanvaspainteritem.h>
#include <QQuickItem>
#include "galleryitemrenderer.h"

//![0]
class GalleryItem : public QCanvasPainterItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GalleryItem)
    Q_PROPERTY(int galleryView READ galleryView WRITE setGalleryView NOTIFY galleryViewChanged)
    Q_PROPERTY(float animationTime READ animationTime WRITE setAnimationTime NOTIFY animationTimeChanged)
    Q_PROPERTY(float animationSine READ animationSine WRITE setAnimationSine NOTIFY animationSineChanged)
    Q_PROPERTY(float animState READ animState WRITE setAnimState NOTIFY animStateChanged)
//![0]
public:

    GalleryItem(QQuickItem *parent = nullptr);

    QCanvasPainterItemRenderer *createItemRenderer() const override;

    int galleryView() const { return m_galleryView; }
    float animationTime() const { return m_animationTime; }
    float animationSine() const { return m_animationSine; }
    float animState() const { return m_animState; }

    void setGalleryView(int value) {
        if (m_galleryView == value)
            return;
        m_galleryView = value;
        Q_EMIT galleryViewChanged();
        update();
    }

    void setAnimationTime(float value) {
        if (qFuzzyCompare(value, m_animationTime))
            return;
        m_animationTime = value;
        Q_EMIT animationTimeChanged();
        update();
    }

    void setAnimationSine(float value) {
        if (qFuzzyCompare(value, m_animationSine))
            return;
        m_animationSine = value;
        Q_EMIT animationSineChanged();
        update();
    }

    void setAnimState(float value) {
        if (qFuzzyCompare(value, m_animState))
            return;
        m_animState = value;
        Q_EMIT animStateChanged();
        update();
    }

Q_SIGNALS:
    void galleryViewChanged();
    void animationTimeChanged();
    void animationSineChanged();
    void animStateChanged();

private:
    int m_galleryView = 0;
    float m_animationTime = 0.0f;
    float m_animationSine = 0.0f;
    float m_animState = 0.0f;
};

#endif // GALLERYITEM_H
