// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


/*
  Based on "sky.cpp" from the Godot engine v3
  Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.
  Copyright (c) 2014-2022 Godot Engine contributors.
*/

#include "proceduralskytexturedata_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrendertexturedata_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ProceduralSkyTextureData
    \inqmlmodule QtQuick3D.Helpers
    \inherits TextureData
    \brief Generates an HDR skybox cubemap.

    This helper type provides an easy way to generate a lightprobe/skybox texture in HDR format. Note that
    generating a lightprobe is an expensive process that can take significant time on embedded hardware.

    The generated cubemap consists of three elements: the sky, the ground, and the sun. The sky and the
    ground cover the top and bottom hemispheres. The position of the sun can be specified by setting
    \l sunLatitude and \l sunLongitude.

    \qml
    View3D {
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData {
                }
            }
        }
    }
    \endqml

    \image sceneenvironment_lightprobe_proceduralsky.jpg {Architectural scene lit by procedural sky visible in background}

    \sa SceneEnvironment
*/

/*! \qmlproperty color ProceduralSkyTextureData::skyTopColor
  Specifies the sky color at the top of the skybox. The top half of the skybox has a gradient from \l skyHorizonColor to \c skyTopColor.
 */

/*! \qmlproperty color ProceduralSkyTextureData::skyHorizonColor
  Specifies the sky color at the horizon.  The top half of the skybox has a gradient from \c skyHorizonColor to \l skyTopColor.
 */

/*! \qmlproperty real ProceduralSkyTextureData::skyCurve
  Modifies the curve of the sky gradient.
 */

/*! \qmlproperty real ProceduralSkyTextureData::skyEnergy
  Specifies the intensity of the top half of the skybox. The sky gradient is multiplied with this factor.
*/

/*! \qmlproperty color ProceduralSkyTextureData::groundBottomColor
  Specifies the ground color at the bottom of the skybox. The bottom half of the skybox has
  a gradient from \l groundHorizonColor to \c groundBottomColor.
*/

/*! \qmlproperty color ProceduralSkyTextureData::groundHorizonColor
  Specifies the ground color at the horizon. The bottom half of the skybox has
  a gradient from \c groundHorizonColor to \l groundBottomColor.
*/

/*! \qmlproperty real ProceduralSkyTextureData::groundCurve
  Modifies the curve of the ground gradient.
*/

/*! \qmlproperty real ProceduralSkyTextureData::groundEnergy
  Specifies the intensity of the bottom half of the skybox. The ground gradient is multiplied with this factor.
*/

/*! \qmlproperty color ProceduralSkyTextureData::sunColor
  Specifies the color of the sun.
*/

/*! \qmlproperty real ProceduralSkyTextureData::sunLatitude
  Specifies the angle between the horizon and the sun position.
 */

/*! \qmlproperty real ProceduralSkyTextureData::sunLongitude
  Specifies the angle between the forward direction and the sun position.
*/

/*! \qmlproperty real ProceduralSkyTextureData::sunAngleMin
  Specifies the angle from the center of the sun to where it starts to fade.
*/

/*! \qmlproperty real ProceduralSkyTextureData::sunAngleMax
  Specifies the angle from the center of the sun to where it fades out completely.
*/

/*! \qmlproperty real ProceduralSkyTextureData::sunCurve
  Modifies the curve of the sun gradient.
*/

/*! \qmlproperty real ProceduralSkyTextureData::sunEnergy
  Specifies the intensity of the sun.
*/

/*! \qmlproperty SkyTextureQuality ProceduralSkyTextureData::textureQuality
  This property sets the quality of the sky texture. Supported values are:

  \value ProceduralSkyTextureData.SkyTextureQualityLow Generate a 512x512 texture
  \value ProceduralSkyTextureData.SkyTextureQualityMedium  Generate a 1024x1024 texture
  \value ProceduralSkyTextureData.SkyTextureQualityHigh  Generate a 2048x2048 texture
  \value ProceduralSkyTextureData.SkyTextureQualityVeryHigh  Generate a 4096x4096 texture
*/

ProceduralSkyTextureData::ProceduralSkyTextureData()
{
    scheduleTextureUpdate();
}

ProceduralSkyTextureData::~ProceduralSkyTextureData()
{
}

QColor ProceduralSkyTextureData::skyTopColor() const
{
    return m_skyTopColor;
}

QColor ProceduralSkyTextureData::skyHorizonColor() const
{
    return m_skyHorizonColor;
}

float ProceduralSkyTextureData::skyCurve() const
{
    return m_skyCurve;
}

float ProceduralSkyTextureData::skyEnergy() const
{
    return m_skyEnergy;
}

QColor ProceduralSkyTextureData::groundBottomColor() const
{
    return m_groundBottomColor;
}

QColor ProceduralSkyTextureData::groundHorizonColor() const
{
    return m_groundHorizonColor;
}

float ProceduralSkyTextureData::groundCurve() const
{
    return m_groundCurve;
}

float ProceduralSkyTextureData::groundEnergy() const
{
    return m_groundEnergy;
}

QColor ProceduralSkyTextureData::sunColor() const
{
    return m_sunColor;
}

float ProceduralSkyTextureData::sunLatitude() const
{
    return m_sunLatitude;
}

float ProceduralSkyTextureData::sunLongitude() const
{
    return m_sunLongitude;
}

float ProceduralSkyTextureData::sunAngleMin() const
{
    return m_sunAngleMin;
}

float ProceduralSkyTextureData::sunAngleMax() const
{
    return m_sunAngleMax;
}

float ProceduralSkyTextureData::sunCurve() const
{
    return m_sunCurve;
}

float ProceduralSkyTextureData::sunEnergy() const
{
    return m_sunEnergy;
}

ProceduralSkyTextureData::SkyTextureQuality ProceduralSkyTextureData::textureQuality() const
{
    return m_textureQuality;
}

void ProceduralSkyTextureData::setSkyTopColor(QColor skyTopColor)
{
    if (m_skyTopColor == skyTopColor)
        return;

    m_skyTopColor = skyTopColor;
    emit skyTopColorChanged(m_skyTopColor);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setSkyHorizonColor(QColor skyHorizonColor)
{
    if (m_skyHorizonColor == skyHorizonColor)
        return;

    m_skyHorizonColor = skyHorizonColor;
    emit skyHorizonColorChanged(m_skyHorizonColor);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setSkyCurve(float skyCurve)
{
    if (qFuzzyCompare(m_skyCurve, skyCurve))
        return;

    m_skyCurve = skyCurve;
    emit skyCurveChanged(m_skyCurve);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setSkyEnergy(float skyEnergy)
{
    if (qFuzzyCompare(m_skyEnergy, skyEnergy))
        return;

    m_skyEnergy = skyEnergy;
    emit skyEnergyChanged(m_skyEnergy);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setGroundBottomColor(QColor groundBottomColor)
{
    if (m_groundBottomColor == groundBottomColor)
        return;

    m_groundBottomColor = groundBottomColor;
    emit groundBottomColorChanged(m_groundBottomColor);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setGroundHorizonColor(QColor groundHorizonColor)
{
    if (m_groundHorizonColor == groundHorizonColor)
        return;

    m_groundHorizonColor = groundHorizonColor;
    emit groundHorizonColorChanged(m_groundHorizonColor);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setGroundCurve(float groundCurve)
{
    if (qFuzzyCompare(m_groundCurve, groundCurve))
        return;

    m_groundCurve = groundCurve;
    emit groundCurveChanged(m_groundCurve);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setGroundEnergy(float groundEnergy)
{
    if (qFuzzyCompare(m_groundEnergy, groundEnergy))
        return;

    m_groundEnergy = groundEnergy;
    emit groundEnergyChanged(m_groundEnergy);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setSunColor(QColor sunColor)
{
    if (m_sunColor == sunColor)
        return;

    m_sunColor = sunColor;
    emit sunColorChanged(m_sunColor);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setSunLatitude(float sunLatitude)
{
    if (qFuzzyCompare(m_sunLatitude, sunLatitude))
        return;

    m_sunLatitude = sunLatitude;
    emit sunLatitudeChanged(m_sunLatitude);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setSunLongitude(float sunLongitude)
{
    if (qFuzzyCompare(m_sunLongitude, sunLongitude))
        return;

    m_sunLongitude = sunLongitude;
    emit sunLongitudeChanged(m_sunLongitude);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setSunAngleMin(float sunAngleMin)
{
    if (qFuzzyCompare(m_sunAngleMin, sunAngleMin))
        return;

    m_sunAngleMin = sunAngleMin;
    emit sunAngleMinChanged(m_sunAngleMin);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setSunAngleMax(float sunAngleMax)
{
    if (qFuzzyCompare(m_sunAngleMax, sunAngleMax))
        return;

    m_sunAngleMax = sunAngleMax;
    emit sunAngleMaxChanged(m_sunAngleMax);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setSunCurve(float sunCurve)
{
    if (qFuzzyCompare(m_sunCurve, sunCurve))
        return;

    m_sunCurve = sunCurve;
    emit sunCurveChanged(m_sunCurve);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setSunEnergy(float sunEnergy)
{
    if (qFuzzyCompare(m_sunEnergy, sunEnergy))
        return;

    m_sunEnergy = sunEnergy;
    emit sunEnergyChanged(m_sunEnergy);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::setTextureQuality(ProceduralSkyTextureData::SkyTextureQuality textureQuality)
{
    if (m_textureQuality == textureQuality)
        return;

    m_textureQuality = textureQuality;
    emit textureQualityChanged(m_textureQuality);
    scheduleTextureUpdate();
}

void ProceduralSkyTextureData::generateRGBA16FTexture()
{
    int size = 0;
    switch (m_textureQuality) {
    case SkyTextureQuality::SkyTextureQualityLow:
        size = 512;
        break;
    case SkyTextureQuality::SkyTextureQualityMedium:
        size = 1024;
        break;
    case SkyTextureQuality::SkyTextureQualityHigh:
        size = 2048;
        break;
    case SkyTextureQuality::SkyTextureQualityVeryHigh:
        size = 4096;
        break;
    }

    const int width = size;
    const int height = width / 2;
    setSize(QSize(width, height));
    setFormat(Format::RGBA16F);
    setHasTransparency(false);
    const int dataSize = width * height * 4 * 2; // 2 bytes per channel
    QByteArray imageData;
    imageData.resize(dataSize);
    generateSkyTexture(width, height, imageData, false);
    setTextureData(imageData);
}

QByteArray ProceduralSkyTextureData::generateSkyTexture(int width, int height, QByteArray &imageData, bool isRGBE) const
{
    quint32 *data = reinterpret_cast<quint32 *>(imageData.data());

    LinearColor skyTopLinear(m_skyTopColor);
    LinearColor skyHorizonLinear(m_skyHorizonColor);
    LinearColor groundBottomLinear(m_groundBottomColor);
    LinearColor groundHorizonLinear(m_groundHorizonColor);
    LinearColor sunLinear(m_sunColor);
    sunLinear.r *= m_sunEnergy;
    sunLinear.g *= m_sunEnergy;
    sunLinear.b *= m_sunEnergy;

    QVector3D sun(0, 0, -1);

    sun = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), m_sunLatitude) * sun;
    sun = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), m_sunLongitude) * sun;
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
                color = groundHorizonLinear.interpolate(groundBottomLinear, ease(c, m_groundCurve));
                color.r *= m_groundEnergy;
                color.g *= m_groundEnergy;
                color.b *= m_groundEnergy;
            } else {
                // Sky color
                float c = vAngle / (M_PI * 0.5f);
                color = skyHorizonLinear.interpolate(skyTopLinear, ease(1.0 - c, m_skyCurve));
                color.r *= m_skyEnergy;
                color.g *= m_skyEnergy;
                color.b *= m_skyEnergy;

                float sunAngle = qRadiansToDegrees(qAcos(clamp(QVector3D::dotProduct(sun, normal), -1.0f, 1.0f)));
                if (sunAngle < m_sunAngleMin) {
                    color = color.blend(sunLinear);
                } else if (sunAngle < m_sunAngleMax) {
                    float c2 = (sunAngle - m_sunAngleMin) / (m_sunAngleMax - m_sunAngleMin);
                    c2 = ease(c2, m_sunCurve);
                    color = color.blend(sunLinear).interpolate(color, c2);
                }
            }

            // Write from bottom to top
            if (isRGBE) {
                data[(height - j - 1) * width + i] = color.toRGBE8();
            } else {
                // RGBA16F
                const int offset = ((height - j - 1) * width + i) * 2;
                qfloat16 *fData = reinterpret_cast<qfloat16 *>(data + offset);
                float pixel[4] = {color.r, color.g, color.b, color.a };
                qFloatToFloat16(fData, pixel, 4);
            }
        }
    }

    return imageData;
}

void ProceduralSkyTextureData::scheduleTextureUpdate()
{
    generateRGBA16FTexture();
}

ProceduralSkyTextureData::LinearColor::LinearColor(const QColor &color)
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

ProceduralSkyTextureData::LinearColor ProceduralSkyTextureData::LinearColor::interpolate(const ProceduralSkyTextureData::LinearColor &color, float value) const
{
    LinearColor copy = *this;

    copy.r += (value * (color.r - r));
    copy.g += (value * (color.g - g));
    copy.b += (value * (color.b - b));
    copy.a += (value * (color.a - a));

    return copy;
}

ProceduralSkyTextureData::LinearColor ProceduralSkyTextureData::LinearColor::blend(const ProceduralSkyTextureData::LinearColor &color) const
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

quint32 ProceduralSkyTextureData::LinearColor::toRGBA8() const
{
    return (quint32(lrintf(r)) & 0xFF) |
           ((quint32(lrintf(g)) & 0xFF) << 8) |
           ((quint32(lrintf(b)) & 0xFF) << 16) |
            ((quint32(lrintf(a)) & 0xFF) << 24);
}

quint32 ProceduralSkyTextureData::LinearColor::toRGBE8() const
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

quint32 ProceduralSkyTextureData::LinearColor::toRGBE9995() const
{
    const float pow2to9 = 512.0f;
    const float B = 15.0f;
    const float N = 9.0f;

    float sharedExp = 65408.000f;

    float cRed = qMax(0.0f, qMin(sharedExp, r));
    float cGreen = qMax(0.0f, qMin(sharedExp, g));
    float cBlue = qMax(0.0f, qMin(sharedExp, b));

    float cMax = qMax(cRed, qMax(cGreen, cBlue));

    float expp = qMax(-B - 1.0f, floor(std::log(cMax) / M_LN2)) + 1.0f + B;

    float sMax = (float)floor((cMax / qPow(2.0f, expp - B - N)) + 0.5f);

    float exps = expp + 1.0f;

    if (0.0 <= sMax && sMax < pow2to9) {
        exps = expp;
    }

    float sRed = qFloor((cRed / pow(2.0f, exps - B - N)) + 0.5f);
    float sGreen = qFloor((cGreen / pow(2.0f, exps - B - N)) + 0.5f);
    float sBlue = qFloor((cBlue / pow(2.0f, exps - B - N)) + 0.5f);

    return (quint32(lrintf(sRed)) & 0x1FF) |
           ((quint32(lrintf(sGreen)) & 0x1FF) << 9) |
           ((quint32(lrintf(sBlue)) & 0x1FF) << 18) |
           ((quint32(lrintf(exps)) & 0x1F) << 27);
}

QT_END_NAMESPACE
