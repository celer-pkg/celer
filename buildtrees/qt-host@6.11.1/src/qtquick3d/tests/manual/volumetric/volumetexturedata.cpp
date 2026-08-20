// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "volumetexturedata.h"
#include <QSize>

QT_BEGIN_NAMESPACE

VolumeTextureData::VolumeTextureData()
{
    const int size = 256;
    setSize(QSize(size, size));
    setDepth(size);
    setFormat(Format::R8);

    QList<uchar> data(size * size * size);
    for (int j = 0; j < size; j++) {
        for (int k = 0; k < size; k++) {
            for (int i = 0; i < size; i++) {
                int color = 0;
                if (k < size / 2) color += 80;
                if (j < size / 2) color += 80;
                if (i < size / 2) color += 80;
                data[i + size * (k + size * j)] = color;
            }
        }
    }

    QByteArray imageData((const char *)data.constData(), data.size());

    setTextureData(imageData);
}

VolumeTextureData::~VolumeTextureData()
{
}

QT_END_NAMESPACE
