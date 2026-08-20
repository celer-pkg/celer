// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DYNAMICTEXTUREDATA_H
#define DYNAMICTEXTUREDATA_H

#include <QtQuick3D/QQuick3DTextureData>

class DynamicTextureData : public QQuick3DTextureData
{
    Q_OBJECT
    QML_NAMED_ELEMENT(DynamicTextureData)
public:
    DynamicTextureData();

public Q_SLOTS:
    void generateRGBA8Texture();
    void generateRGBE8Texture();
    void generateRGBA32FTexture();

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
        quint32 toRGBE8() const;
    };
    static void generateHDRTexture(int width, int height, QByteArray &imageData, bool isRGBE);
};

#endif // DYNAMICTEXTUREDATA_H
