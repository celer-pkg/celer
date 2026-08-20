// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGLIGHTMAPIO_H
#define QSSGLIGHTMAPIO_H

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

#include "qtquick3druntimerenderexports.h"

#include <QByteArray>
#include <QMap>
#include <QSharedPointer>
#include <QIODevice>
#include <QVariantMap>

QT_BEGIN_NAMESPACE

struct QSSGLoadedTexture;
struct QSSGRenderTextureFormat;

enum class QSSGLightmapIODataTag : quint32 {
    Unset = 0,
    Mask,
    Texture_Final,
    Texture_Direct,
    Texture_Indirect,
    Metadata,
    Mesh,
    SceneMetadata,
    // ...
    Count
};

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGLightmapLoader
{
    ~QSSGLightmapLoader();

    static QSharedPointer<QSSGLightmapLoader> open(const QSharedPointer<QIODevice> &stream);
    static QSharedPointer<QSSGLightmapLoader> open(const QString &path);

    QByteArray readF32Image(const QString &key, QSSGLightmapIODataTag tag) const;
    QByteArray readU32Image(const QString &key, QSSGLightmapIODataTag tag) const;
    QByteArray readData(const QString &key, QSSGLightmapIODataTag tag) const;
    QVariantMap readMap(const QString &key, QSSGLightmapIODataTag tag) const;

    QList<std::pair<QString, QSSGLightmapIODataTag>> getKeys() const;
    static QSSGLoadedTexture *createTexture(QSharedPointer<QIODevice> stream, const QSSGRenderTextureFormat &format, const QString &key);

private:
    QSSGLightmapLoader();
    Q_DISABLE_COPY(QSSGLightmapLoader)

    struct QSSGLightmapIOPrivate *d = nullptr;
};

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGLightmapWriter
{
    ~QSSGLightmapWriter();

    static QSharedPointer<QSSGLightmapWriter> open(const QSharedPointer<QIODevice> &stream);
    static QSharedPointer<QSSGLightmapWriter> open(const QString &path);

    bool writeF32Image(const QString &key, QSSGLightmapIODataTag tag, const QByteArray &imageFP32);
    bool writeU32Image(const QString &key, QSSGLightmapIODataTag tag, const QByteArray &imageU32);
    bool writeData(const QString &key, QSSGLightmapIODataTag tag, const QByteArray &buffer);
    bool writeMap(const QString &key, QSSGLightmapIODataTag tag, const QVariantMap &data);

    bool close() const;

private:
    Q_DISABLE_COPY(QSSGLightmapWriter)
    QSSGLightmapWriter();

    struct QSSGLightmapIOPrivate *d = nullptr;
};

QT_END_NAMESPACE

#endif // QSSGLIGHTMAPIO_H
