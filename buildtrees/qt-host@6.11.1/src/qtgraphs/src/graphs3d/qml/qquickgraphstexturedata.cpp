// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "commonutils_p.h"
#include "qquickgraphstexturedata_p.h"

#include <QtGraphs/private/qgraphsglobal_p.h>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
                   "QT_BEGIN_NAMESPACE" \
                   "class QQuickGraphsTextureData;" \
                   "QT_END_NAMESPACE"
               )

Q_TRACE_POINT(qtgraphs, QGraphs3DCreateGradient_entry, QSize textureSize);
Q_TRACE_POINT(qtgraphs, QGraphs3DCreateGradient_exit);

QQuickGraphsTextureData::QQuickGraphsTextureData() {}

QQuickGraphsTextureData::~QQuickGraphsTextureData() {}

void QQuickGraphsTextureData::createGradient(QLinearGradient gradient)
{
    const qreal textureWidth = CommonUtils::maxTextureSize();
    Q_TRACE_SCOPE(QGraphs3DCreateGradient, QSize(textureWidth, gradientTextureHeight));
    setSize(QSize(textureWidth, gradientTextureHeight));
    setFormat(QQuick3DTextureData::RGBA8);
    setHasTransparency(false);

    // Make sure the gradient fills the whole image
    gradient.setFinalStop(textureWidth, gradientTextureHeight);
    gradient.setStart(0., 0.);

    QByteArray imageData;

    QByteArray gradientScanline;
    gradientScanline.resize(textureWidth * 4); // RGBA8
    auto stops = gradient.stops();

    int x = 0;
    for (int i = 1; i < stops.size(); i++) {
        QColor startColor = stops.at(i - 1).second;
        QColor endColor = stops.at(i).second;
        float w = 0.0f;
        w = (stops.at(i).first - stops.at(i - 1).first) * textureWidth;

        if (startColor.alphaF() < 1.0 || endColor.alphaF() < 1.0)
            setHasTransparency(true);

        for (int t = 0; t <= static_cast<int>(w); t++) {
            QColor color = linearInterpolate(startColor, endColor, t / w);
            int offset = x * 4;
            gradientScanline.data()[offset + 0] = char(color.red());
            gradientScanline.data()[offset + 1] = char(color.green());
            gradientScanline.data()[offset + 2] = char(color.blue());
            gradientScanline.data()[offset + 3] = char(color.alpha());
            x++;
        }
    }
    for (int y = 0; y < gradientTextureHeight; y++)
        imageData += gradientScanline;
    setTextureData(imageData);
}

QColor QQuickGraphsTextureData::linearInterpolate(QColor startColor, QColor endColor, float value)
{
    QColor output;

    output.setRedF(startColor.redF() + (value * (endColor.redF() - startColor.redF())));
    output.setGreenF(startColor.greenF() + (value * (endColor.greenF() - startColor.greenF())));
    output.setBlueF(startColor.blueF() + (value * (endColor.blueF() - startColor.blueF())));
    output.setAlphaF(startColor.alphaF() + (value * (endColor.alphaF() - startColor.alphaF())));

    return output;
}

QT_END_NAMESPACE

#include "moc_qquickgraphstexturedata_p.cpp"
