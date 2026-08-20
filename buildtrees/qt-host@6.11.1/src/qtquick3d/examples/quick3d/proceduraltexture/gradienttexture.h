// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GRADIENTTEXTURE_H
#define GRADIENTTEXTURE_H

#include <QtQuick3D/QQuick3DTextureData>

#include <QtGui/QColor>

//! [class definition]
class GradientTexture : public QQuick3DTextureData
{
    Q_OBJECT
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(QColor startColor READ startColor WRITE setStartColor NOTIFY startColorChanged)
    Q_PROPERTY(QColor endColor READ endColor WRITE setEndColor NOTIFY endColorChanged)
    QML_NAMED_ELEMENT(GradientTexture)
//! [class definition]

public:
    GradientTexture();

    int height() const;
    int width() const;
    QColor startColor() const;
    QColor endColor() const;

public Q_SLOTS:
    void setHeight(int height);
    void setWidth(int width);
    void setStartColor(QColor startColor);
    void setEndColor(QColor endColor);

Q_SIGNALS:
    void heightChanged(int height);
    void widthChanged(int width);
    void startColorChanged(QColor startColor);
    void endColorChanged(QColor endColor);

private:
    void updateTexture();
    QByteArray generateTexture();

    static QColor linearInterpolate(const QColor &color1, const QColor &color2, float value);

    int m_height = 256;
    int m_width = 256;
    QColor m_startColor = QColor(QStringLiteral("#d4fc79"));
    QColor m_endColor = QColor(QStringLiteral("#96e6a1"));
};

#endif // GRADIENTTEXTURE_H
