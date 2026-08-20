// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "topographicseries.h"

//! [0]
// Value used to encode height data as RGB value on PNG file
const float packingFactor = 11983.f;
//! [0]

TopographicSeries::TopographicSeries()
{
    setDrawMode(QSurface3DSeries::DrawSurface);
    setShading(QSurface3DSeries::Shading::Flat);
    setBaseColor(Qt::white);
}

TopographicSeries::~TopographicSeries() = default;

void TopographicSeries::setTopographyFile(const QString &file, float width, float height)
{
    //! [1]
    QImage heightMapImage(file);
    uchar *bits = heightMapImage.bits();
    int imageHeight = heightMapImage.height();
    int imageWidth = heightMapImage.width();
    int widthBits = imageWidth * 4;
    float stepX = width / float(imageWidth);
    float stepZ = height / float(imageHeight);

    QSurfaceDataArray dataArray;
    dataArray.reserve(imageHeight);
    for (int i = 0; i < imageHeight; ++i) {
        int p = i * widthBits;
        float z = height - float(i) * stepZ;
        QSurfaceDataRow newRow;
        newRow.reserve(imageWidth);
        for (int j = 0; j < imageWidth; ++j) {
            uchar aa = bits[p + 0];
            uchar rr = bits[p + 1];
            uchar gg = bits[p + 2];
            uint color = uint((gg << 16) + (rr << 8) + aa);
            float y = float(color) / packingFactor;
            newRow.append(QSurfaceDataItem(float(j) * stepX, y, z));
            p += 4;
        }
        dataArray.append(newRow);
    }

    dataProxy()->resetArray(dataArray);
    //! [1]

    m_sampleCountX = float(imageWidth);
    m_sampleCountZ = float(imageHeight);
}
