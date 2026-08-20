// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


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


#ifndef PROCEDURALSKYTEXTURE_H
#define PROCEDURALSKYTEXTURE_H

#include <QtQuick3DHelpers/qtquick3dhelpersexports.h>
#include <QtQuick3D/QQuick3DTextureData>
#include <QtQml/QQmlEngine>

#include <QtGui/QColor>
#include <QtCore/QByteArray>

QT_BEGIN_NAMESPACE

class Q_QUICK3DHELPERS_EXPORT ProceduralSkyTextureData : public QQuick3DTextureData
{
    Q_OBJECT
    Q_PROPERTY(QColor skyTopColor READ skyTopColor WRITE setSkyTopColor NOTIFY skyTopColorChanged)
    Q_PROPERTY(QColor skyHorizonColor READ skyHorizonColor WRITE setSkyHorizonColor NOTIFY skyHorizonColorChanged)
    Q_PROPERTY(float skyCurve READ skyCurve WRITE setSkyCurve NOTIFY skyCurveChanged)
    Q_PROPERTY(float skyEnergy READ skyEnergy WRITE setSkyEnergy NOTIFY skyEnergyChanged)

    Q_PROPERTY(QColor groundBottomColor READ groundBottomColor WRITE setGroundBottomColor NOTIFY groundBottomColorChanged)
    Q_PROPERTY(QColor groundHorizonColor READ groundHorizonColor WRITE setGroundHorizonColor NOTIFY groundHorizonColorChanged)
    Q_PROPERTY(float groundCurve READ groundCurve WRITE setGroundCurve NOTIFY groundCurveChanged)
    Q_PROPERTY(float groundEnergy READ groundEnergy WRITE setGroundEnergy NOTIFY groundEnergyChanged)

    Q_PROPERTY(QColor sunColor READ sunColor WRITE setSunColor NOTIFY sunColorChanged)
    Q_PROPERTY(float sunLatitude READ sunLatitude WRITE setSunLatitude NOTIFY sunLatitudeChanged)
    Q_PROPERTY(float sunLongitude READ sunLongitude WRITE setSunLongitude NOTIFY sunLongitudeChanged)
    Q_PROPERTY(float sunAngleMin READ sunAngleMin WRITE setSunAngleMin NOTIFY sunAngleMinChanged)
    Q_PROPERTY(float sunAngleMax READ sunAngleMax WRITE setSunAngleMax NOTIFY sunAngleMaxChanged)
    Q_PROPERTY(float sunCurve READ sunCurve WRITE setSunCurve NOTIFY sunCurveChanged)
    Q_PROPERTY(float sunEnergy READ sunEnergy WRITE setSunEnergy NOTIFY sunEnergyChanged)
    Q_PROPERTY(SkyTextureQuality textureQuality READ textureQuality WRITE setTextureQuality NOTIFY textureQualityChanged)
    QML_ELEMENT

public:
    enum class SkyTextureQuality {
        SkyTextureQualityLow, // 512
        SkyTextureQualityMedium, // 1024
        SkyTextureQualityHigh, // 2048
        SkyTextureQualityVeryHigh, // 4096
    };
    Q_ENUM(SkyTextureQuality)

    ProceduralSkyTextureData();
    ~ProceduralSkyTextureData();

    QColor skyTopColor() const;
    QColor skyHorizonColor() const;
    float skyCurve() const;
    float skyEnergy() const;

    QColor groundBottomColor() const;
    QColor groundHorizonColor() const;
    float groundCurve() const;
    float groundEnergy() const;

    QColor sunColor() const;
    float sunLatitude() const;
    float sunLongitude() const;
    float sunAngleMin() const;
    float sunAngleMax() const;
    float sunCurve() const;
    float sunEnergy() const;

    SkyTextureQuality textureQuality() const;

public Q_SLOTS:
    void setSkyTopColor(QColor skyTopColor);
    void setSkyHorizonColor(QColor skyHorizonColor);
    void setSkyCurve(float skyCurve);
    void setSkyEnergy(float skyEnergy);

    void setGroundBottomColor(QColor groundBottomColor);
    void setGroundHorizonColor(QColor groundHorizonColor);
    void setGroundCurve(float groundCurve);
    void setGroundEnergy(float groundEnergy);

    void setSunColor(QColor sunColor);
    void setSunLatitude(float sunLatitude);
    void setSunLongitude(float sunLongitude);
    void setSunAngleMin(float sunAngleMin);
    void setSunAngleMax(float sunAngleMax);
    void setSunCurve(float sunCurve);
    void setSunEnergy(float sunEnergy);

    void setTextureQuality(SkyTextureQuality textureQuality);

    void generateRGBA16FTexture();

Q_SIGNALS:
    void skyTopColorChanged(QColor skyTopColor);
    void skyHorizonColorChanged(QColor skyHorizonColor);
    void skyCurveChanged(float skyCurve);
    void skyEnergyChanged(float skyEnergy);

    void groundBottomColorChanged(QColor groundBottomColor);
    void groundHorizonColorChanged(QColor groundHorizonColor);
    void groundCurveChanged(float groundCurve);
    void groundEnergyChanged(float groundEnergy);

    void sunColorChanged(QColor sunColor);

    void sunLatitudeChanged(float sunLatitude);
    void sunLongitudeChanged(float sunLongitude);
    void sunAngleMinChanged(float sunAngleMin);
    void sunAngleMaxChanged(float sunAngleMax);
    void sunCurveChanged(float sunCurve);
    void sunEnergyChanged(float sunEnergy);

    void textureQualityChanged(SkyTextureQuality textureQuality);

private:

    struct LinearColor {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 0.0f;
        LinearColor(const QColor &color);
        LinearColor() {}

        LinearColor interpolate(const LinearColor &color, float value) const;
        LinearColor blend(const LinearColor &color) const;
        quint32 toRGBE9995() const;
        quint32 toRGBA8() const;
        quint32 toRGBE8() const;
    };

    QByteArray generateSkyTexture(int width, int height, QByteArray &imageData, bool isRGBE) const;
    void scheduleTextureUpdate();
    QColor m_skyTopColor = QColor(165, 214, 241);
    QColor m_skyHorizonColor = QColor(214, 234, 250);
    float m_skyCurve = 0.09f;
    float m_skyEnergy = 1.0f;

    QColor m_groundBottomColor = QColor(40, 47, 54);
    QColor m_groundHorizonColor = QColor(108, 101, 95);
    float m_groundCurve = 0.02f;
    float m_groundEnergy = 1.0f;

    QColor m_sunColor = QColor(255, 255, 255);
    float m_sunLatitude = 35.0f;
    float m_sunLongitude = 0.0f;
    float m_sunAngleMin = 1.0f;
    float m_sunAngleMax = 100.0f;
    float m_sunCurve = 0.05f;
    float m_sunEnergy = 1.0f;

    SkyTextureQuality m_textureQuality = SkyTextureQuality::SkyTextureQualityMedium;
};

QT_END_NAMESPACE

#endif // PROCEDURALSKYTEXTURE_H
