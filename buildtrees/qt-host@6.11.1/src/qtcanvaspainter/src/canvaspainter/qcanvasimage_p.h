// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASIMAGE_P_H
#define QCANVASIMAGE_P_H

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

#include <QtCore/qshareddata.h>
#include <qcolor.h>

QT_BEGIN_NAMESPACE

class QCanvasImagePrivate : public QSharedData
{
public:
    QCanvasImagePrivate();

    static QCanvasImagePrivate *get(QCanvasImage *image) { return image->d.get(); }
    static const QCanvasImagePrivate *get(const QCanvasImage *image) { return image->d.get(); }

    enum class DataType {
        Unknown,
        GradientTextureFromImage,
        TextureFromImage,
        ImportedTexture
    };

    int id;
    int width;
    int height;
    DataType type;
    qsizetype sizeInBytes;
    QColor tintColor;
};

QT_END_NAMESPACE

#endif // QCANVASIMAGE_P_H
