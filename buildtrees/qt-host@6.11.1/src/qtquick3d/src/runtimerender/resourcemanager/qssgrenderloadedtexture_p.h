// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_LOADED_TEXTURE_H
#define QSSG_RENDER_LOADED_TEXTURE_H

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

#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>
#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>

#include <QtGui/QImage>

#include <private/qtexturefiledata_p.h>

QT_BEGIN_NAMESPACE
class QSSGRenderTextureData;

struct QSSGTextureData
{
    void *data = nullptr;
    quint32 dataSizeInBytes = 0;
    QSSGRenderTextureFormat format = QSSGRenderTextureFormat::Unknown;
};

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGInputUtil
{
public:
    enum FileType { UnknownFile, ImageFile, TextureFile, HdrFile };
    static QSharedPointer<QIODevice> getStreamForFile(const QString &inPath,
                                                      bool inQuiet = false,
                                                      QString *outPath = nullptr);
    static QSharedPointer<QIODevice> getStreamForTextureFile(const QString &inPath,
                                                             bool inQuiet = false,
                                                             QString *outPath = nullptr,
                                                             FileType *outFileType = nullptr);
};


// Utility class used for loading image data from disk.
struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGLoadedTexture
{
public:
    qint32 width = 0;
    qint32 height = 0;
    qint32 depth = 0;
    qint32 components = 0;
    void *data = nullptr;
    bool ownsData = true;
    QTextureFileData textureFileData;
    QImage image;
    quint32 dataSizeInBytes = 0;
    QSSGRenderTextureFormat format = QSSGRenderTextureFormat::RGBA8;
    // #TODO: There should be more ways to influence this (hints on the texture)
    bool isSRGB = false;

    ~QSSGLoadedTexture();
    void setFormatFromComponents()
    {
        switch (components) {
        case 1: // undefined, but in this context probably luminance
            format = QSSGRenderTextureFormat::R8;
            break;
        case 2:
            format = QSSGRenderTextureFormat::RG8;
            break;
        case 3:
            format = QSSGRenderTextureFormat::RGB8;
            break;

        default:
            // fallthrough intentional
        case 4:
            format = QSSGRenderTextureFormat::RGBA8;
            break;
        }
    }

    // Returns true if this image has a pixel less than 255.
    bool scanForTransparency() const;

    static QSSGLoadedTexture *load(const QString &inPath,
                                   const QSSGRenderTextureFormat &inFormat,
                                   bool inFlipY = true);
    static QSSGLoadedTexture *loadQImage(const QString &inPath, qint32 flipVertical);
    static QSSGLoadedTexture *loadCompressedImage(const QString &inPath);
    static QSSGLoadedTexture *loadHdrImage(const QSharedPointer<QIODevice> &source, const QSSGRenderTextureFormat &inFormat);
    static QSSGLoadedTexture *loadTextureData(QSSGRenderTextureData *textureData);
    static QSSGLoadedTexture *loadLightmapImage(const QString &inPath, const QSSGRenderTextureFormat &inFormat, const QString &key);
};
QT_END_NAMESPACE

#endif
