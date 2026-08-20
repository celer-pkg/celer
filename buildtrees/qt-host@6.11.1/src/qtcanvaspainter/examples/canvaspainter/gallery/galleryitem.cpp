// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "galleryitem.h"

//![0]
GalleryItem::GalleryItem(QQuickItem *parent)
:  QCanvasPainterItem(parent)
{
}

QCanvasPainterItemRenderer* GalleryItem::createItemRenderer() const
{
    return new GalleryItemRenderer();
}
//![0]
