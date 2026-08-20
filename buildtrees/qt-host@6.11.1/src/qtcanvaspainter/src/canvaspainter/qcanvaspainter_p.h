// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


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

#ifndef QCANVASPAINTER_P_H
#define QCANVASPAINTER_P_H

#include "qcanvaspainter.h"
#include "qcanvasimage.h"
#include <QtGui/qfont.h>
#include <QtCore/qstring.h>
#include <QtCore/qhash.h>
#include <QtCore/qset.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

class QCPainterEngine;
class QCPainterRhiRenderer;

Q_DECLARE_LOGGING_CATEGORY(QC_INFO)

// Keeps count of the texture id's and the total size of textures.
// This doesn't cache the actual texture data.
// Only used for QCanvasImages created from QImage (gradient or user-supplied).
// Does not track QCanvasImages created from native textures (offscreen canvas or user-supplied).
class QCanvasImageTracker
{
public:
    bool contains(qint64 key) const { return m_data.contains(key); }
    QCanvasImage image(qint64 key) const { return m_data.value(key); }
    int textureId(qint64 key) const { return m_data.value(key).id(); }
    void insert(qint64 key, const QCanvasImage &image)
    {
        m_dataAmount += image.sizeInBytes();
        m_data.insert(key, image);
    }
    qsizetype dataAmount() const { return m_dataAmount; }
    qsizetype size() const { return m_data.size(); }
    void removeTemporaryResources();
    void removeTextureId(int imageId);
    void handleRemoveTextures();
    void markTextureIdUsed(int imageId);
    void clear();

private:
    friend class QCanvasPainterPrivate;
    QCanvasPainterPrivate *m_painterPrivate = nullptr;
    QHash<qint64, QCanvasImage> m_data;
    QList<QCanvasImage> m_cleanupTextures;
    QList<int> m_usedTextureIDs;
    qsizetype m_dataAmount = 0;
    bool m_doingResourcesRemoval = false;
};

class QRhi;

class QCanvasPainterPrivate
{
public:
    QCanvasPainterPrivate();
    ~QCanvasPainterPrivate();
    QCPainterEngine *engine() const;

    static QCanvasPainterPrivate *get(QCanvasPainter *painter) { return painter->d_func(); }
    static const QCanvasPainterPrivate *get(const QCanvasPainter *painter) { return painter->d_func(); }

    qint64 generateImageKey(const QImage &image, QCanvasPainter::ImageFlags flags) const;
    QCanvasImage getQCanvasImage(const QImage &image, QCanvasPainter::ImageFlags flags, qint64 imageKey = 0);
    QCanvasImage getQCanvasImage(QRhiTexture *texture, QCanvasPainter::ImageFlags flags);
    QCanvasImage getQCanvasImage(const QCanvasOffscreenCanvas &canvas, QCanvasPainter::ImageFlags flags);
    void handleCleanupTextures();
    void clearTextureCache();
    void markTextureIdUsed(int imageId);

    void setFont(const QFont &font);
    void fillText(const QString &text, float x, float y, float maxWidth = -1);
    void fillText(const QString &text, const QRectF &rect);
    QRectF textBoundingBox(const QString &text, float x, float y, float maxWidth = -1);
    QRectF textBoundingBox(const QString &text, const QRectF &rect);
    void drawBoxShadow(QCanvasPainter *painter, const QCanvasBoxShadow &shadow);

    QCanvasImageTracker m_imageTracker;
    QHash<quint64, QCanvasImage> m_nativeTextureCache; // QRhiTexture::globalResourceId -> QCanvasImage
    QSet<int> m_pendingNativeTextureDelete;
    float m_devicePixelRatio = 1.0f;
    QCPainterRhiRenderer *m_renderer = nullptr;
    QCPainterEngine *m_e = nullptr;
    int m_maxTextures = 0;
    bool m_trackingDisabled = false;
};

QT_END_NAMESPACE

#endif // QCANVASPAINTER_P_H
