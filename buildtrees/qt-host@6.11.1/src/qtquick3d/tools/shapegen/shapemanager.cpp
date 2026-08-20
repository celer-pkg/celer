// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "shapemanager.h"
#include <QtQuick3DParticles/private/qquick3dparticleshapedatautils_p.h>

#include <QtCore/qdir.h>
#include <QCborStreamWriter>
#include <QCborStreamReader>
#include <QRandomGenerator>

ShapeManager::ShapeManager(QObject *parent) : QObject(parent)
{

}

void ShapeManager::setImage(const QString &filename)
{
    if (m_imageFilename == filename)
        return;

    m_imageFilename = filename;
    loadImage();
}

void ShapeManager::setDepth(float depth)
{
    m_depth = depth;
}

void ShapeManager::setScale(float scale)
{
    m_scale = scale;
}

// Set the amount of positions CBOR should contain.
// When -1, then amount is same as the amount of pixels in the image.
// Default value -1.
void ShapeManager::setAmount(int amount)
{
    m_amount = amount;
}

void ShapeManager::setSortingMode(SortingMode mode)
{
    m_sortingMode = mode;
}

void ShapeManager::setSortingPosition(const QVector3D &position)
{
    m_sortingPosition = position;
}

bool ShapeManager::loadImage()
{
    QFileInfo fileInfo(m_imageFilename);

    // Is this a real file?
    if (!fileInfo.exists()) {
        qWarning() << "Imagefile not found:" << qPrintable(m_imageFilename);
        return false;
    }

    if (!m_image.load(m_imageFilename)) {
        qWarning() << "Not able to load image:" << qPrintable(m_imageFilename);
        return false;
    }

    // Make sure image is in proper format
    if (m_image.format() != QImage::Format_ARGB32 && m_image.format() != QImage::Format_ARGB32_Premultiplied)
        m_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    // Flip the image
    m_image.flip();

    return true;
}

bool ShapeManager::generateData()
{
    if (m_image.isNull()) {
        qWarning() << "Can't generate data, no image loaded";
        return false;
    }

    QRect r(0, 0, m_image.width(), m_image.height());
    // resample on the fly using 16-bit
    int sx = (m_image.width() << 16) / r.width();
    int sy = (m_image.height() << 16) / r.height();
    int w = r.width();
    int h = r.height();
    for (int y = 0; y < h; ++y) {
        const uint *sl = (const uint *) m_image.constScanLine((y * sy) >> 16);
        for (int x = 0; x < w; ++x) {
            if (sl[(x * sx) >> 16] & 0xff000000) {
                QVector3D pos(x - (w * 0.5f), y - (h * 0.5f), 0.0f);
                pos *= m_scale;
                // Get random z based on depth
                float posZ = float(QRandomGenerator::global()->generateDouble() - 0.5f) * m_depth * m_scale;
                pos.setZ(posZ);
                m_data << pos;
            }
        }
    }
    // Shuffle the data into random order
    auto rd = std::random_device{};
    std::shuffle(m_data.begin(),
                 m_data.end(),
                  std::default_random_engine(rd()));

    return true;
}

bool ShapeManager::saveShapeData(const QString &filename)
{
    m_outputData.clear();

    QFile dataFile(filename);
    if (!dataFile.open(QIODevice::WriteOnly)) {
        // Invalid file
        qWarning() << "Unable to open file:" << filename;
        return false;
    }

    QCborStreamWriter writer(&dataFile);
    QQuick3DParticleShapeDataUtils::writeShapeHeader(writer);

    // Start positions array
    writer.startArray();

    // Collect correct amount of positions
    if (m_amount < 0) {
        m_outputData = m_data;
    } else {
        // m_data is shuffled so we can just pick m_amount from there
        int index = 0;
        while (m_outputData.size() < m_amount) {
            m_outputData << m_data[index];
            index = index < (m_data.size() - 1) ? index + 1 : 0;
        }
    }

    // Sort
    if (m_sortingMode == SortingMode::DistanceClosestFirst) {
        std::sort(m_outputData.begin(),
                  m_outputData.end(),
                  [this](const QVector3D& lhs, const QVector3D& rhs) {
            return m_sortingPosition.distanceToPoint(lhs) < m_sortingPosition.distanceToPoint(rhs);
        });
    } else if (m_sortingMode == SortingMode::DistanceClosestLast) {
        std::sort(m_outputData.begin(),
                  m_outputData.end(),
                  [this](const QVector3D& lhs, const QVector3D& rhs) {
            return m_sortingPosition.distanceToPoint(rhs) < m_sortingPosition.distanceToPoint(lhs);
        });
    }

    // Write positions
    for (auto position : m_outputData)
        QQuick3DParticleShapeDataUtils::writeValue(writer, position);

    // Leave positions array
    writer.endArray();

    // Leave root array
    writer.endArray();

    return true;
}

void ShapeManager::dumpOutput()
{
    qDebug() << "Particle Shape";
    qDebug() << m_outputData;
    qDebug() << "Image positions:" << m_data.size();
    qDebug() << "Generated positions:" << m_outputData.size();
}
