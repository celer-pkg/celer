// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "imageinstancetable.h"
#include <math.h>
#include <QMatrix4x4>
#include <QColor>
#include <QImage>
#include <QQmlFile>

ImageInstanceTable::ImageInstanceTable(QQuick3DObject *parent) : QQuick3DInstancing(parent)
{
}

ImageInstanceTable::~ImageInstanceTable()
{
}

int ImageInstanceTable::gridSize() const
{
    return m_gridSize;
}

float ImageInstanceTable::gridSpacing() const
{
    return m_gridSpacing;
}

QString ImageInstanceTable::image() const
{
    return m_imageSource;
}

void ImageInstanceTable::setGridSize(int gridSize)
{
    if (m_gridSize == gridSize)
        return;

    m_gridSize = gridSize;
    emit gridSizeChanged(m_gridSize);
    markDirty();
    m_dirty = true;
}

void ImageInstanceTable::setGridSpacing(float gridSpacing)
{
    if (qFuzzyCompare(m_gridSpacing, gridSpacing))
        return;

    m_gridSpacing = gridSpacing;
    emit gridSpacingChanged(m_gridSpacing);
    markDirty();
    m_dirty = true;
}

void ImageInstanceTable::setImage(QString image)
{
    if (m_imageSource == image)
        return;

    m_imageSource = image;
    emit imageChanged(m_imageSource);
    markDirty();
    m_dirty = true;
}

QByteArray ImageInstanceTable::getInstanceBuffer(int *instanceCount)
{
    if (m_dirty) {
        QImage image(m_imageSource);
        if (image.isNull())
            qWarning() << "Could not load image" << m_imageSource;
        m_instanceData.resize(0);
        image = image.scaledToWidth(m_gridSize, Qt::SmoothTransformation);
        int instanceNumber = 0;
        for (int blockX = 0; blockX < image.width(); ++blockX) {
            float xPos = m_gridSpacing * (blockX - m_gridSize/2);
            for (int blockY = 0; blockY < image.width(); ++blockY) {
                QColor color = image.pixel(blockX, blockY);
                float yPos = -m_gridSpacing * (blockY - m_gridSize/2);

                float zPos = 10 * m_gridSpacing * (color.lightnessF() - 1.0);

                auto entry = calculateTableEntry({xPos,yPos,zPos}, {1.0, 1.0, 1.0}, {0,0,0}, color, {});
                m_instanceData.append(reinterpret_cast<const char *>(&entry), sizeof(entry));
                instanceNumber++;
            }
        }
        m_instanceCount = instanceNumber;
        m_dirty = false;
    }
    if (instanceCount)
        *instanceCount = m_instanceCount;

    return m_instanceData;
}
