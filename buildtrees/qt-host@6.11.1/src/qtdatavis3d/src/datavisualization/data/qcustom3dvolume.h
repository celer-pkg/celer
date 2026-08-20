// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QCUSTOM3DVOLUME_H
#define QTDATAVIS3D_QCUSTOM3DVOLUME_H

#include <QtDataVisualization/qdatavisualizationglobal.h>
#include <QtDataVisualization/QCustom3DItem>
#include <QtGui/QColor>
#include <QtGui/QImage>

QT_BEGIN_NAMESPACE

class QQuaternion;

class QCustom3DVolumePrivate;

class Q_DATAVISUALIZATION_EXPORT QCustom3DVolume : public QCustom3DItem
{
    Q_OBJECT
    Q_PROPERTY(int textureWidth READ textureWidth WRITE setTextureWidth NOTIFY textureWidthChanged)
    Q_PROPERTY(int textureHeight READ textureHeight WRITE setTextureHeight NOTIFY textureHeightChanged)
    Q_PROPERTY(int textureDepth READ textureDepth WRITE setTextureDepth NOTIFY textureDepthChanged)
    Q_PROPERTY(int sliceIndexX READ sliceIndexX WRITE setSliceIndexX NOTIFY sliceIndexXChanged)
    Q_PROPERTY(int sliceIndexY READ sliceIndexY WRITE setSliceIndexY NOTIFY sliceIndexYChanged)
    Q_PROPERTY(int sliceIndexZ READ sliceIndexZ WRITE setSliceIndexZ NOTIFY sliceIndexZChanged)
    Q_PROPERTY(QList<QRgb> colorTable READ colorTable WRITE setColorTable NOTIFY colorTableChanged)
    Q_PROPERTY(QList<uchar> *textureData READ textureData WRITE setTextureData NOTIFY
                       textureDataChanged)
    Q_PROPERTY(float alphaMultiplier READ alphaMultiplier WRITE setAlphaMultiplier NOTIFY alphaMultiplierChanged)
    Q_PROPERTY(bool preserveOpacity READ preserveOpacity WRITE setPreserveOpacity NOTIFY preserveOpacityChanged)
    Q_PROPERTY(bool useHighDefShader READ useHighDefShader WRITE setUseHighDefShader NOTIFY useHighDefShaderChanged)
    Q_PROPERTY(bool drawSlices READ drawSlices WRITE setDrawSlices NOTIFY drawSlicesChanged)
    Q_PROPERTY(bool drawSliceFrames READ drawSliceFrames WRITE setDrawSliceFrames NOTIFY drawSliceFramesChanged)
    Q_PROPERTY(QColor sliceFrameColor READ sliceFrameColor WRITE setSliceFrameColor NOTIFY sliceFrameColorChanged)
    Q_PROPERTY(QVector3D sliceFrameWidths READ sliceFrameWidths WRITE setSliceFrameWidths NOTIFY sliceFrameWidthsChanged)
    Q_PROPERTY(QVector3D sliceFrameGaps READ sliceFrameGaps WRITE setSliceFrameGaps NOTIFY sliceFrameGapsChanged)
    Q_PROPERTY(QVector3D sliceFrameThicknesses READ sliceFrameThicknesses WRITE setSliceFrameThicknesses NOTIFY sliceFrameThicknessesChanged)

public:

    explicit QCustom3DVolume(QObject *parent = nullptr);
    explicit QCustom3DVolume(const QVector3D &position, const QVector3D &scaling,
                             const QQuaternion &rotation, int textureWidth, int textureHeight,
                             int textureDepth, QList<uchar> *textureData,
                             QImage::Format textureFormat, const QList<QRgb> &colorTable,
                             QObject *parent = nullptr);
    virtual ~QCustom3DVolume();

    void setTextureWidth(int value);
    int textureWidth() const;
    void setTextureHeight(int value);
    int textureHeight() const;
    void setTextureDepth(int value);
    int textureDepth() const;
    void setTextureDimensions(int width, int height, int depth);
    int textureDataWidth() const;

    void setSliceIndexX(int value);
    int sliceIndexX() const;
    void setSliceIndexY(int value);
    int sliceIndexY() const;
    void setSliceIndexZ(int value);
    int sliceIndexZ() const;
    void setSliceIndices(int x, int y, int z);

    void setColorTable(const QList<QRgb> &colors);
    QList<QRgb> colorTable() const;

    void setTextureData(QList<uchar> *data);
    QList<uchar> *createTextureData(const QList<QImage *> &images);
    QList<uchar> *textureData() const;
    void setSubTextureData(Qt::Axis axis, int index, const uchar *data);
    void setSubTextureData(Qt::Axis axis, int index, const QImage &image);

    void setTextureFormat(QImage::Format format);
    QImage::Format textureFormat() const;

    void setAlphaMultiplier(float mult);
    float alphaMultiplier() const;
    void setPreserveOpacity(bool enable);
    bool preserveOpacity() const;

    void setUseHighDefShader(bool enable);
    bool useHighDefShader() const;

    void setDrawSlices(bool enable);
    bool drawSlices() const;
    void setDrawSliceFrames(bool enable);
    bool drawSliceFrames() const;

    void setSliceFrameColor(const QColor &color);
    QColor sliceFrameColor() const;
    void setSliceFrameWidths(const QVector3D &values);
    QVector3D sliceFrameWidths() const;
    void setSliceFrameGaps(const QVector3D &values);
    QVector3D sliceFrameGaps() const;
    void setSliceFrameThicknesses(const QVector3D &values);
    QVector3D sliceFrameThicknesses() const;

    QImage renderSlice(Qt::Axis axis, int index);

Q_SIGNALS:
    void textureWidthChanged(int value);
    void textureHeightChanged(int value);
    void textureDepthChanged(int value);
    void sliceIndexXChanged(int value);
    void sliceIndexYChanged(int value);
    void sliceIndexZChanged(int value);
    void colorTableChanged();
    void textureDataChanged(QList<uchar> *data);
    void textureFormatChanged(QImage::Format format);
    void alphaMultiplierChanged(float mult);
    void preserveOpacityChanged(bool enabled);
    void useHighDefShaderChanged(bool enabled);
    void drawSlicesChanged(bool enabled);
    void drawSliceFramesChanged(bool enabled);
    void sliceFrameColorChanged(const QColor &color);
    void sliceFrameWidthsChanged(const QVector3D &values);
    void sliceFrameGapsChanged(const QVector3D &values);
    void sliceFrameThicknessesChanged(const QVector3D &values);

protected:
    QCustom3DVolumePrivate *dptr();
    const QCustom3DVolumePrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QCustom3DVolume)

    friend class Abstract3DRenderer;
};

QT_END_NAMESPACE

#endif
