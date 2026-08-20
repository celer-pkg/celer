// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef VOLUMETRICMODIFIER_H
#define VOLUMETRICMODIFIER_H

#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qcustom3dvolume.h>
#include <QtCore/qtimer.h>
#include <QtGui/qrgb.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qslider.h>
#include <QtWidgets/qradiobutton.h>

class VolumetricModifier : public QObject
{
    Q_OBJECT
public:
    explicit VolumetricModifier(Q3DScatter *scatter);
    ~VolumetricModifier();

    void setFpsLabel(QLabel *fpsLabel);
    void setMediumDetailRB(QRadioButton *button);
    void setHighDetailRB(QRadioButton *button);
    void setSliceLabels(QLabel *xLabel, QLabel *yLabel, QLabel *zLabel);
    void setAlphaMultiplierLabel(QLabel *label);

public Q_SLOTS:
    void sliceX(int enabled);
    void sliceY(int enabled);
    void sliceZ(int enabled);
    void adjustSliceX(int value);
    void adjustSliceY(int value);
    void adjustSliceZ(int value);
    void handleFpsChange(qreal fps);
    void handleTimeout();
    void toggleLowDetail(bool enabled);
    void toggleMediumDetail(bool enabled);
    void toggleHighDetail(bool enabled);
    void setFpsMeasurement(bool enabled);
    void setSliceSliders(QSlider *sliderX, QSlider *sliderY, QSlider *sliderZ);
    void changeColorTable(int enabled);
    void setPreserveOpacity(bool enabled);
    void setTransparentGround(bool enabled);
    void setUseHighDefShader(bool enabled);
    void adjustAlphaMultiplier(int value);
    void toggleAreaAll(bool enabled);
    void toggleAreaMine(bool enabled);
    void toggleAreaMountain(bool enabled);
    void setDrawSliceFrames(int enabled);

private:
    void initHeightMap(QString fileName, QList<uchar> &layerData);
    void initMineShaftArray();
    int createVolume(int textureSize, int startIndex, int count, QList<uchar> *textureData);
    int excavateMineShaft(int textureSize, int startIndex, int count, QList<uchar> *textureData);
    void excavateMineBlock(int textureSize, int dataIndex, int size, QList<uchar> *textureData);
    void handleSlicingChanges();

    Q3DScatter *m_graph = nullptr;
    QCustom3DVolume *m_volumeItem = nullptr;
    int m_sliceIndexX = 0;
    int m_sliceIndexY = 0;
    int m_sliceIndexZ = 0;
    bool m_slicingX = false;
    bool m_slicingY = false;
    bool m_slicingZ = false;
    QLabel *m_fpsLabel = nullptr;
    QRadioButton *m_mediumDetailRB = nullptr;
    QRadioButton *m_highDetailRB = nullptr;
    QList<uchar> *m_lowDetailData = nullptr;
    QList<uchar> *m_mediumDetailData = nullptr;
    QList<uchar> *m_highDetailData = nullptr;
    int m_mediumDetailIndex = 0;
    int m_highDetailIndex = 0;
    int m_mediumDetailShaftIndex = 0;
    int m_highDetailShaftIndex = 0;
    QSlider *m_sliceSliderX = nullptr;
    QSlider *m_sliceSliderY = nullptr;
    QSlider *m_sliceSliderZ = nullptr;
    QList<QRgb> m_colorTable1 = {};
    QList<QRgb> m_colorTable2 = {};
    bool m_usingPrimaryTable = true;
    QLabel *m_sliceLabelX = nullptr;
    QLabel *m_sliceLabelY = nullptr;
    QLabel *m_sliceLabelZ = nullptr;
    QLabel *m_alphaMultiplierLabel = nullptr;
    QList<uchar> m_magmaLayer = {};
    QList<uchar> m_waterLayer = {};
    QList<uchar> m_groundLayer = {};
    QList<QPair<QVector3D, QVector3D>> m_mineShaftArray = {};
    QTimer m_timer;
};

#endif
