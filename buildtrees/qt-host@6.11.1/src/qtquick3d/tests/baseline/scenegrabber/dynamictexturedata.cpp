// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "dynamictexturedata.h"
#include <QSize>
#include <QColor>
#include <QVector3D>
#include <QQuaternion>

DynamicTextureData::DynamicTextureData()
{

}

static QColor linearInterpolate(const QColor &color1, const QColor &color2, float value)
{
    QColor output;

    output.setRedF(color1.redF() + (value * (color2.redF() - color1.redF())));
    output.setGreenF(color1.greenF() + (value * (color2.greenF() - color1.greenF())));
    output.setBlueF(color1.blueF() + (value * (color2.blueF() - color1.blueF())));

    return output;
}

void DynamicTextureData::generateRGBA8Texture()
{
    const int width = 256;
    const int height = 256;
    setSize(QSize(width, height));
    setFormat(Format::RGBA8);
    setHasTransparency(false);
    QByteArray imageData;

    QByteArray gradientScanline;
    gradientScanline.resize(width * 4); // RGBA8

    for (int x = 0; x < width; ++x) {
        QColor color = linearInterpolate(QColor(255, 0, 0, 255), QColor(0, 255, 255, 255), x / float(width));
        int offset = x * 4;
        gradientScanline.data()[offset + 0] = char(color.red());
        gradientScanline.data()[offset + 1] = char(color.green());
        gradientScanline.data()[offset + 2] = char(color.blue());
        gradientScanline.data()[offset + 3] = char(255);
    }

    for (int y = 0; y < height; ++y)
        imageData += gradientScanline;

    setTextureData(imageData);
}

void DynamicTextureData::generateRGBE8Texture()
{
    const int width = 1024;
    const int height = 512;
    setSize(QSize(width, height));
    setFormat(Format::RGBE8);
    setHasTransparency(false);
    const int dataSize = width * height * 4; // 1 byte per channel
    QByteArray imageData;
    imageData.resize(dataSize);
    generateHDRTexture(width, height, imageData, true);
    setTextureData(imageData);
}

void DynamicTextureData::generateRGBA32FTexture()
{
    const int width = 1024;
    const int height = 512;
    setSize(QSize(width, height));
    setFormat(Format::RGBA32F);
    setHasTransparency(false);
    const int dataSize = width * height * 4 * 4; // 4 bytes per channel
    QByteArray imageData;
    imageData.resize(dataSize);
    generateHDRTexture(width, height, imageData, false);
    setTextureData(imageData);
}


DynamicTextureData::LinearColor::LinearColor(const QColor &color)
{
    const float red = color.redF();
    const float green = color.greenF();
    const float blue = color.blueF();
    const float alpha = color.alphaF();

    r = red < 0.04045 ? red * (1.0 / 12.92) : qPow((red + 0.055) * (1.0 / (1 + 0.055)), 2.4),
    g = green < 0.04045 ? green * (1.0 / 12.92) : qPow((green + 0.055) * (1.0 / (1 + 0.055)), 2.4),
    b = blue < 0.04045 ? blue * (1.0 / 12.92) : qPow((blue + 0.055) * (1.0 / (1 + 0.055)), 2.4),
    a = alpha;
}

DynamicTextureData::LinearColor DynamicTextureData::LinearColor::interpolate(const DynamicTextureData::LinearColor &color, float value) const
{
    LinearColor copy = *this;

    copy.r += (value * (color.r - r));
    copy.g += (value * (color.g - g));
    copy.b += (value * (color.b - b));
    copy.a += (value * (color.a - a));

    return copy;
}

DynamicTextureData::LinearColor DynamicTextureData::LinearColor::blend(const DynamicTextureData::LinearColor &color) const
{
    LinearColor copy;
    float sa = 1.0 - color.a;
    copy.a = a * sa + color.a;
    if (copy.a == 0) {
        return LinearColor();
    } else {
        copy.r = (r * a * sa + color.r * color.a) / copy.a;
        copy.g = (g * a * sa + color.g * color.a) / copy.a;
        copy.b = (b * a * sa + color.b * color.a) / copy.a;
    }
    return copy;
}

quint32 DynamicTextureData::LinearColor::toRGBE8() const
{
    float v = 0.0f;
    int exp = 0;

    v = r;
    if (g > v)
        v = g;
    if (b > v)
        v = b;

    v = frexp(v, &exp) * 256.0f / v;
    quint32 result = 0;
    quint8 *components = reinterpret_cast<quint8*>(&result);
    components[0] = quint8(r * v);
    components[1] = quint8(g * v);
    components[2] = quint8(b * v);
    components[3] = quint8(exp + 128);
    return result;
}

void DynamicTextureData::generateHDRTexture(int width, int height, QByteArray &imageData, bool isRGBE)
{
    quint32 *data = reinterpret_cast<quint32 *>(imageData.data());

    LinearColor skyTopLinear(QColor(68, 129, 235));
    LinearColor skyHorizonLinear(QColor(4, 190, 254));
    LinearColor groundBottomLinear(QColor(48, 67, 82));
    LinearColor groundHorizonLinear(QColor(215, 210, 204));
    LinearColor sunLinear(QColor(255, 255, 249));

    QVector3D sun(0, 0, -1);

    sun = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 28.0) * sun;
    sun = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), 45.0) * sun;
    sun.normalize();

    auto clamp = [](float value, float min, float max) {
        if (value < min)
            return min;
        else if (value > max)
            return max;
        return value;
    };

    auto ease = [](float x, float c) {
        if (x < 0.0f)
            x = 0.0f;
        else if (x > 1.0f)
            x = 1.0f;
        if (c > 0.0f) {
            if (c < 1.0f) {
                return 1.0f - qPow(1.0f - x, 1.0f / c);
            } else {
                return qPow(x, c);
            }
        } else if (c < 0.0f) {
            if (x < 0.5f) {
                return qPow(x * 2.0f, -c) * 0.5f;
            } else {
                return (1.0f - qPow(1.0f - (x - 0.5f) * 2.0f, -c)) * 0.5f + 0.5f;
            }
        } else
            return 0.0f;
    };

    for (int i = 0; i < width; i++) {

        float u = float(i) / (width - 1);
        float phi = u * 2.0 * M_PI;

        for (int j = 0; j < height; j++) {
            float v = float(j) / (height - 1);
            float theta = v * M_PI;

            QVector3D normal(qSin(phi) * qSin(theta) * -1.0,
                             qCos(theta),
                             qCos(phi) * qSin(theta) * -1.0);
            normal.normalize();
            float vAngle = qAcos(clamp(normal.y(), -1.0, 1.0));
            LinearColor color;

            if (normal.y() < 0) {
                // Ground color
                float c = (vAngle - (M_PI * 0.5f)) / (M_PI * 0.5f);
                color = groundHorizonLinear.interpolate(groundBottomLinear, ease(c, 0.02f));
            } else {
                // Sky color
                float c = vAngle / (M_PI * 0.5f);
                color = skyHorizonLinear.interpolate(skyTopLinear, ease(1.0 - c, 0.09f));

                float sunAngle = qRadiansToDegrees(qAcos(clamp(QVector3D::dotProduct(sun, normal), -1.0f, 1.0f)));
                if (sunAngle < 1.0) {
                    color = color.blend(sunLinear);
                } else if (sunAngle < 100) {
                    float c2 = (sunAngle - 1) / (100 - 1);
                    c2 = ease(c2, 0.05f);
                    color = color.blend(sunLinear).interpolate(color, c2);
                }
            }
            // Write from bottem to top
            if (isRGBE) {
                data[(height - j - 1) * width + i] = color.toRGBE8();
            } else {
                const int offset = ((height - j - 1) * width + i) * 4;
                float *fData = reinterpret_cast<float *>(data + offset);
                fData[0] = color.r;
                fData[1] = color.g;
                fData[2] = color.b;
                fData[3] = color.a;
            }
        }
    }
}
