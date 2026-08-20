// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "gradienttexture.h"

#include <QtCore/QSize>
#include <QtGui/QGradient>

GradientTexture::GradientTexture()
{
    updateTexture();
}

int GradientTexture::height() const
{
    return m_height;
}

int GradientTexture::width() const
{
    return m_width;
}

QColor GradientTexture::startColor() const
{
    return m_startColor;
}

QColor GradientTexture::endColor() const
{
    return m_endColor;
}

void GradientTexture::setHeight(int height)
{
    if (m_height == height)
        return;

    m_height = height;
    emit heightChanged(m_height);
    updateTexture();
}

void GradientTexture::setWidth(int width)
{
    if (m_width == width)
        return;

    m_width = width;
    emit widthChanged(m_width);
    updateTexture();
}

//! [property]
void GradientTexture::setStartColor(QColor startColor)
{
    if (m_startColor == startColor)
        return;

    m_startColor = startColor;
    emit startColorChanged(m_startColor);
    updateTexture();
}
//! [property]

void GradientTexture::setEndColor(QColor endColor)
{
    if (m_endColor == endColor)
        return;

    m_endColor = endColor;
    emit endColorChanged(m_endColor);
    updateTexture();
}

//! [updateTexture]
void GradientTexture::updateTexture()
{
    setSize(QSize(m_width, m_height));
    setFormat(QQuick3DTextureData::RGBA8);
    setHasTransparency(false);
    setTextureData(generateTexture());
}
//! [updateTexture]

//! [generateTexture]
QByteArray GradientTexture::generateTexture()
{
    QByteArray imageData;
    // Create a horizontal gradient between startColor and endColor

    // Create a single scanline and reuse that data for each
    QByteArray gradientScanline;
    gradientScanline.resize(m_width * 4); // RGBA8

    for (int x = 0; x < m_width; ++x) {
        QColor color = linearInterpolate(m_startColor, m_endColor, x / float(m_width));
        int offset = x * 4;
        gradientScanline.data()[offset + 0] = char(color.red());
        gradientScanline.data()[offset + 1] = char(color.green());
        gradientScanline.data()[offset + 2] = char(color.blue());
        gradientScanline.data()[offset + 3] = char(255);
    }

    for (int y = 0; y < m_height; ++y)
        imageData += gradientScanline;

    return imageData;
}
//! [generateTexture]

QColor GradientTexture::linearInterpolate(const QColor &color1, const QColor &color2, float value)
{
    QColor output;

    output.setRedF(color1.redF() + (value * (color2.redF() - color1.redF())));
    output.setGreenF(color1.greenF() + (value * (color2.greenF() - color1.greenF())));
    output.setBlueF(color1.blueF() + (value * (color2.blueF() - color1.blueF())));

    return output;
}
