// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "volumetric.h"
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/qcustom3dlabel.h>
#include <QtDataVisualization/q3dinputhandler.h>
#include <QtCore/qmath.h>

#include <QtGui/qopenglcontext.h>
#include <QtGui/qquaternion.h>

const int lowDetailSize(256);
const int mediumDetailSize(512);
const int highDetailSize(1024);
const int colorTableSize(256);
const int layerDataSize(512);
const int mineShaftDiameter(1);

const int airColorIndex(254);
const int mineShaftColorIndex(255);
const int layerColorThickness(60);
const int heightToColorDiv(140);
const int magmaColorsMin(0);
const int magmaColorsMax(layerColorThickness);
const int aboveWaterGroundColorsMin(magmaColorsMax + 1);
const int aboveWaterGroundColorsMax(aboveWaterGroundColorsMin + layerColorThickness);
const int underWaterGroundColorsMin(aboveWaterGroundColorsMax + 1);
const int underWaterGroundColorsMax(underWaterGroundColorsMin + layerColorThickness);
const int waterColorsMin(underWaterGroundColorsMax + 1);
const int waterColorsMax(waterColorsMin + layerColorThickness);
const int terrainTransparency(12);

static bool isOpenGLES()
{
#if QT_CONFIG(opengles2)
    return true;
#else
    return QOpenGLContext::currentContext()->isOpenGLES();
#endif
}

VolumetricModifier::VolumetricModifier(Q3DScatter *scatter)
    : m_graph(scatter),
      m_sliceIndexX(lowDetailSize / 2),
      m_sliceIndexY(lowDetailSize / 4),
      m_sliceIndexZ(lowDetailSize / 2)
{
    m_graph->activeTheme()->setType(Q3DTheme::ThemePrimaryColors);
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricLeft);
    //! [0]
    m_graph->setOrthoProjection(true);
    //! [0]
    m_graph->activeTheme()->setBackgroundEnabled(false);

    // Only allow zooming at the center and limit the zoom to 200% to avoid clipping issues
    static_cast<Q3DInputHandler *>(m_graph->activeInputHandler())->setZoomAtTargetEnabled(false);
    m_graph->scene()->activeCamera()->setMaxZoomLevel(200.0f);

    toggleAreaAll(true);

    if (!isOpenGLES()) {
        m_lowDetailData
                = new QList<uchar>(lowDetailSize * lowDetailSize * lowDetailSize / 2);
        m_mediumDetailData
                = new QList<uchar>(mediumDetailSize * mediumDetailSize * mediumDetailSize / 2);
        m_highDetailData
                = new QList<uchar>(highDetailSize * highDetailSize * highDetailSize / 2);

        initHeightMap(QStringLiteral(":/heightmaps/layer_ground.png"), m_groundLayer);
        initHeightMap(QStringLiteral(":/heightmaps/layer_water.png"), m_waterLayer);
        initHeightMap(QStringLiteral(":/heightmaps/layer_magma.png"), m_magmaLayer);

        initMineShaftArray();

        createVolume(lowDetailSize, 0, lowDetailSize, m_lowDetailData);
        excavateMineShaft(lowDetailSize, 0, m_mineShaftArray.size(), m_lowDetailData);

        //! [1]
        m_volumeItem = new QCustom3DVolume;
        // Adjust water level to zero with a minor tweak to y-coordinate position and scaling
        m_volumeItem->setScaling(
                    QVector3D(m_graph->axisX()->max() - m_graph->axisX()->min(),
                              (m_graph->axisY()->max() - m_graph->axisY()->min()) * 0.91f,
                              m_graph->axisZ()->max() - m_graph->axisZ()->min()));
        m_volumeItem->setPosition(
                    QVector3D((m_graph->axisX()->max() + m_graph->axisX()->min()) / 2.0f,
                              -0.045f * (m_graph->axisY()->max() - m_graph->axisY()->min()) +
                              (m_graph->axisY()->max() + m_graph->axisY()->min()) / 2.0f,
                              (m_graph->axisZ()->max() + m_graph->axisZ()->min()) / 2.0f));
        m_volumeItem->setScalingAbsolute(false);
        //! [1]
        //! [2]
        m_volumeItem->setTextureWidth(lowDetailSize);
        m_volumeItem->setTextureHeight(lowDetailSize / 2);
        m_volumeItem->setTextureDepth(lowDetailSize);
        m_volumeItem->setTextureFormat(QImage::Format_Indexed8);
        m_volumeItem->setTextureData(new QList<uchar>(*m_lowDetailData));
        //! [2]

        // Generate color tables
        m_colorTable1.resize(colorTableSize);
        m_colorTable2.resize(colorTableSize);

        for (int i = 0; i < colorTableSize - 2; i++) {
            if (i < magmaColorsMax) {
                m_colorTable1[i] = qRgba(130 - (i * 2), 0, 0, 255);
            } else if (i < aboveWaterGroundColorsMax) {
                m_colorTable1[i] = qRgba((i - magmaColorsMax) * 4,
                                         ((i - magmaColorsMax) * 2) + 120,
                                         (i - magmaColorsMax) * 5, terrainTransparency);
            } else if (i < underWaterGroundColorsMax) {
                m_colorTable1[i] = qRgba(((layerColorThickness - i - aboveWaterGroundColorsMax)) + 70,
                                         ((layerColorThickness - i - aboveWaterGroundColorsMax) * 2) + 20,
                                         ((layerColorThickness - i - aboveWaterGroundColorsMax)) + 50,
                                         terrainTransparency);
            } else if (i < waterColorsMax) {
                m_colorTable1[i] = qRgba(0, 0, ((i - underWaterGroundColorsMax) * 2) + 120,
                                         terrainTransparency);
            } else {
                m_colorTable1[i] = qRgba(0, 0, 0, 0); // Not used
            }
        }
        m_colorTable1[airColorIndex] = qRgba(0, 0, 0, 0);
        m_colorTable1[mineShaftColorIndex] = qRgba(50, 50, 50, 255);

        // The alternate color table just has gray gradients for all terrain except water
        for (int i = 0; i < colorTableSize - 2; i++) {
            if (i < magmaColorsMax) {
                m_colorTable2[i] = qRgba(((i - aboveWaterGroundColorsMax) * 2),
                                         ((i - aboveWaterGroundColorsMax) * 2),
                                         ((i - aboveWaterGroundColorsMax) * 2),
                                         255);
            } else if (i < underWaterGroundColorsMax) {
                m_colorTable2[i] = qRgba(((i - aboveWaterGroundColorsMax) * 2),
                                         ((i - aboveWaterGroundColorsMax) * 2),
                                         ((i - aboveWaterGroundColorsMax) * 2),
                                         terrainTransparency);
            } else if (i < waterColorsMax) {
                m_colorTable2[i] = qRgba(0,
                                         0,
                                         ((i - underWaterGroundColorsMax) * 2) + 120,
                                         terrainTransparency);
            } else {
                m_colorTable2[i] = qRgba(0, 0, 0, 0); // Not used
            }
        }
        m_colorTable2[airColorIndex] = qRgba(0, 0, 0, 0);
        m_colorTable2[mineShaftColorIndex] = qRgba(255, 255, 0, 255);

        //! [3]
        m_volumeItem->setColorTable(m_colorTable1);
        //! [3]

        //! [4]
        m_volumeItem->setSliceFrameGaps(QVector3D(0.01f, 0.02f, 0.01f));
        m_volumeItem->setSliceFrameThicknesses(QVector3D(0.0025f, 0.005f, 0.0025f));
        m_volumeItem->setSliceFrameWidths(QVector3D(0.0025f, 0.005f, 0.0025f));
        m_volumeItem->setDrawSliceFrames(false);
        //! [4]
        handleSlicingChanges();

        //! [5]
        m_graph->addCustomItem(m_volumeItem);
        //! [5]

        m_timer.start(0);
    } else {
        // OpenGL ES2 doesn't support 3D textures, so show a warning label instead
        QCustom3DLabel *warningLabel = new QCustom3DLabel(
                    "QCustom3DVolume is not supported with OpenGL ES2",
                    QFont(),
                    QVector3D(0.0f, 0.5f, 0.0f),
                    QVector3D(1.5f, 1.5f, 0.0f),
                    QQuaternion());
        warningLabel->setPositionAbsolute(true);
        warningLabel->setFacingCamera(true);
        m_graph->addCustomItem(warningLabel);
    }

    QObject::connect(m_graph, &QAbstract3DGraph::currentFpsChanged, this,
                     &VolumetricModifier::handleFpsChange);
    QObject::connect(&m_timer, &QTimer::timeout, this,
                     &VolumetricModifier::handleTimeout);

}

VolumetricModifier::~VolumetricModifier()
{
    delete m_graph;
}

void VolumetricModifier::setFpsLabel(QLabel *fpsLabel)
{
    m_fpsLabel = fpsLabel;
}

void VolumetricModifier::setMediumDetailRB(QRadioButton *button)
{
    m_mediumDetailRB = button;
}

void VolumetricModifier::setHighDetailRB(QRadioButton *button)
{
    m_highDetailRB = button;
}

void VolumetricModifier::setSliceLabels(QLabel *xLabel, QLabel *yLabel, QLabel *zLabel)
{
    m_sliceLabelX = xLabel;
    m_sliceLabelY = yLabel;
    m_sliceLabelZ = zLabel;

    adjustSliceX(m_sliceSliderX->value());
    adjustSliceY(m_sliceSliderY->value());
    adjustSliceZ(m_sliceSliderZ->value());
}

void VolumetricModifier::setAlphaMultiplierLabel(QLabel *label)
{
    m_alphaMultiplierLabel = label;
}

void VolumetricModifier::sliceX(int enabled)
{
    m_slicingX = enabled;
    handleSlicingChanges();
}

void VolumetricModifier::sliceY(int enabled)
{
    m_slicingY = enabled;
    handleSlicingChanges();
}

void VolumetricModifier::sliceZ(int enabled)
{
    m_slicingZ = enabled;
    handleSlicingChanges();
}

void VolumetricModifier::adjustSliceX(int value)
{
    if (m_volumeItem) {
        m_sliceIndexX = value / (1024 / m_volumeItem->textureWidth());
        if (m_sliceIndexX == m_volumeItem->textureWidth())
            m_sliceIndexX--;
        if (m_volumeItem->sliceIndexX() != -1)
            //! [6]
            m_volumeItem->setSliceIndexX(m_sliceIndexX);
            //! [6]
        //! [8]
        m_sliceLabelX->setPixmap(
                    QPixmap::fromImage(m_volumeItem->renderSlice(Qt::XAxis, m_sliceIndexX)));
        //! [8]
    }
}

void VolumetricModifier::adjustSliceY(int value)
{
    if (m_volumeItem) {
        m_sliceIndexY = value / (1024 / m_volumeItem->textureHeight());
        if (m_sliceIndexY == m_volumeItem->textureHeight())
            m_sliceIndexY--;
        if (m_volumeItem->sliceIndexY() != -1)
            m_volumeItem->setSliceIndexY(m_sliceIndexY);
        m_sliceLabelY->setPixmap(
                    QPixmap::fromImage(m_volumeItem->renderSlice(Qt::YAxis, m_sliceIndexY)));
    }
}

void VolumetricModifier::adjustSliceZ(int value)
{
    if (m_volumeItem) {
        m_sliceIndexZ = value / (1024 / m_volumeItem->textureDepth());
        if (m_sliceIndexZ == m_volumeItem->textureDepth())
            m_sliceIndexZ--;
        if (m_volumeItem->sliceIndexZ() != -1)
            m_volumeItem->setSliceIndexZ(m_sliceIndexZ);
        m_sliceLabelZ->setPixmap(
                    QPixmap::fromImage(m_volumeItem->renderSlice(Qt::ZAxis, m_sliceIndexZ)));
    }
}

void VolumetricModifier::handleFpsChange(qreal fps)
{
    const QString fpsFormat = QStringLiteral("FPS: %1");
    int fps10 = int(fps * 10.0);
    m_fpsLabel->setText(fpsFormat.arg(qreal(fps10) / 10.0));
}

void VolumetricModifier::handleTimeout()
{
    if (!m_mediumDetailRB->isEnabled()) {
        if (m_mediumDetailIndex != mediumDetailSize) {
            m_mediumDetailIndex = createVolume(mediumDetailSize, m_mediumDetailIndex, 4,
                                               m_mediumDetailData);
        } else if (m_mediumDetailShaftIndex != m_mineShaftArray.size()) {
            m_mediumDetailShaftIndex = excavateMineShaft(mediumDetailSize, m_mediumDetailShaftIndex,
                                                         1, m_mediumDetailData );
        } else {
            m_mediumDetailRB->setEnabled(true);
            QString label = QStringLiteral("Medium (%1x%2x%1)");
            m_mediumDetailRB->setText(label.arg(mediumDetailSize).arg(mediumDetailSize / 2));
        }
    } else if (!m_highDetailRB->isEnabled()) {
        if (m_highDetailIndex != highDetailSize) {
            m_highDetailIndex = createVolume(highDetailSize, m_highDetailIndex, 1,
                                             m_highDetailData);
        } else if (m_highDetailShaftIndex != m_mineShaftArray.size()) {
            m_highDetailShaftIndex = excavateMineShaft(highDetailSize, m_highDetailShaftIndex, 1,
                                                       m_highDetailData);
        } else  {
            m_highDetailRB->setEnabled(true);
            QString label = QStringLiteral("High (%1x%2x%1)");
            m_highDetailRB->setText(label.arg(highDetailSize).arg(highDetailSize / 2));
            m_timer.stop();
        }
    }
}

void VolumetricModifier::toggleLowDetail(bool enabled)
{
    if (enabled && m_volumeItem) {
        m_volumeItem->setTextureData(new QList<uchar>(*m_lowDetailData));
        m_volumeItem->setTextureDimensions(lowDetailSize, lowDetailSize / 2, lowDetailSize);
        adjustSliceX(m_sliceSliderX->value());
        adjustSliceY(m_sliceSliderY->value());
        adjustSliceZ(m_sliceSliderZ->value());
    }
}

void VolumetricModifier::toggleMediumDetail(bool enabled)
{
    if (enabled && m_volumeItem) {
        m_volumeItem->setTextureData(new QList<uchar>(*m_mediumDetailData));
        m_volumeItem->setTextureDimensions(mediumDetailSize, mediumDetailSize / 2, mediumDetailSize);
        adjustSliceX(m_sliceSliderX->value());
        adjustSliceY(m_sliceSliderY->value());
        adjustSliceZ(m_sliceSliderZ->value());
    }
}

void VolumetricModifier::toggleHighDetail(bool enabled)
{
    if (enabled && m_volumeItem) {
        m_volumeItem->setTextureData(new QList<uchar>(*m_highDetailData));
        m_volumeItem->setTextureDimensions(highDetailSize, highDetailSize / 2, highDetailSize);
        adjustSliceX(m_sliceSliderX->value());
        adjustSliceY(m_sliceSliderY->value());
        adjustSliceZ(m_sliceSliderZ->value());
    }
}

void VolumetricModifier::setFpsMeasurement(bool enabled)
{
    m_graph->setMeasureFps(enabled);
    if (enabled)
        m_fpsLabel->setText(QStringLiteral("Measuring..."));
    else
        m_fpsLabel->setText(QString());
}

void VolumetricModifier::setSliceSliders(QSlider *sliderX, QSlider *sliderY, QSlider *sliderZ)
{
    m_sliceSliderX = sliderX;
    m_sliceSliderY = sliderY;
    m_sliceSliderZ = sliderZ;

    // Set sliders to interesting values
    m_sliceSliderX->setValue(715);
    m_sliceSliderY->setValue(612);
    m_sliceSliderZ->setValue(715);
}

void VolumetricModifier::changeColorTable(int enabled)
{
    if (m_volumeItem) {
        if (enabled)
            m_volumeItem->setColorTable(m_colorTable2);
        else
            m_volumeItem->setColorTable(m_colorTable1);

        m_usingPrimaryTable = !enabled;

        // Rerender image labels
        adjustSliceX(m_sliceSliderX->value());
        adjustSliceY(m_sliceSliderY->value());
        adjustSliceZ(m_sliceSliderZ->value());
    }
}

void VolumetricModifier::setPreserveOpacity(bool enabled)
{

    if (m_volumeItem) {
        //! [10]
        m_volumeItem->setPreserveOpacity(enabled);
        //! [10]

        // Rerender image labels
        adjustSliceX(m_sliceSliderX->value());
        adjustSliceY(m_sliceSliderY->value());
        adjustSliceZ(m_sliceSliderZ->value());
    }
}

void VolumetricModifier::setTransparentGround(bool enabled)
{
    if (m_volumeItem) {
        //! [11]
        int newAlpha = enabled ? terrainTransparency : 255;
        for (int i = aboveWaterGroundColorsMin; i < underWaterGroundColorsMax; i++) {
            QRgb oldColor1 = m_colorTable1.at(i);
            QRgb oldColor2 = m_colorTable2.at(i);
            m_colorTable1[i] = qRgba(qRed(oldColor1), qGreen(oldColor1), qBlue(oldColor1), newAlpha);
            m_colorTable2[i] = qRgba(qRed(oldColor2), qGreen(oldColor2), qBlue(oldColor2), newAlpha);
        }
        if (m_usingPrimaryTable)
            m_volumeItem->setColorTable(m_colorTable1);
        else
            m_volumeItem->setColorTable(m_colorTable2);
        //! [11]
        adjustSliceX(m_sliceSliderX->value());
        adjustSliceY(m_sliceSliderY->value());
        adjustSliceZ(m_sliceSliderZ->value());
    }
}

void VolumetricModifier::setUseHighDefShader(bool enabled)
{
    if (m_volumeItem) {
        //! [12]
        m_volumeItem->setUseHighDefShader(enabled);
        //! [12]
    }
}

void VolumetricModifier::adjustAlphaMultiplier(int value)
{
    if (m_volumeItem) {
        float mult;
        if (value > 100)
            mult = float(value - 99) / 2.0f;
        else
            mult = float(value) / float(500 - value * 4);
        //! [9]
        m_volumeItem->setAlphaMultiplier(mult);
        //! [9]
        QString labelFormat = QStringLiteral("Alpha multiplier: %1");
        m_alphaMultiplierLabel->setText(labelFormat.arg(
                                            QString::number(m_volumeItem->alphaMultiplier(), 'f', 3)));

        // Rerender image labels
        adjustSliceX(m_sliceSliderX->value());
        adjustSliceY(m_sliceSliderY->value());
        adjustSliceZ(m_sliceSliderZ->value());
    }
}

void VolumetricModifier::toggleAreaAll(bool enabled)
{
    if (enabled) {
        m_graph->axisX()->setRange(0.0f, 1000.0f);
        m_graph->axisY()->setRange(-600.0f, 600.0f);
        m_graph->axisZ()->setRange(0.0f, 1000.0f);
        m_graph->axisX()->setSegmentCount(5);
        m_graph->axisY()->setSegmentCount(6);
        m_graph->axisZ()->setSegmentCount(5);
    }
}

void VolumetricModifier::toggleAreaMine(bool enabled)
{
    if (enabled) {
        m_graph->axisX()->setRange(350.0f, 850.0f);
        m_graph->axisY()->setRange(-500.0f, 100.0f);
        m_graph->axisZ()->setRange(350.0f, 900.0f);
        m_graph->axisX()->setSegmentCount(10);
        m_graph->axisY()->setSegmentCount(6);
        m_graph->axisZ()->setSegmentCount(11);
    }
}

void VolumetricModifier::toggleAreaMountain(bool enabled)
{
    if (enabled) {
        m_graph->axisX()->setRange(300.0f, 600.0f);
        m_graph->axisY()->setRange(-100.0f, 400.0f);
        m_graph->axisZ()->setRange(300.0f, 600.0f);
        m_graph->axisX()->setSegmentCount(9);
        m_graph->axisY()->setSegmentCount(5);
        m_graph->axisZ()->setSegmentCount(9);
    }
}

void VolumetricModifier::setDrawSliceFrames(int enabled)
{
    if (m_volumeItem)
        m_volumeItem->setDrawSliceFrames(enabled);
}

void VolumetricModifier::initHeightMap(QString fileName, QList<uchar> &layerData)
{
    QImage heightImage(fileName);

    layerData.resize(layerDataSize * layerDataSize);
    const uchar *bits = heightImage.bits();
    int index = 0;
    QList<QRgb> colorTable = heightImage.colorTable();
    for (int i = 0; i < layerDataSize; i++) {
        for (int j = 0; j < layerDataSize; j++) {
            layerData[index] = qRed(colorTable.at(bits[index]));
            index++;
        }
    }
}

int VolumetricModifier::createVolume(int textureSize, int startIndex, int count,
                                     QList<uchar> *textureData)
{
    // Generate volume from layer data.
    int index = startIndex * textureSize * textureSize / 2.0f;
    int endIndex = startIndex + count;
    if (endIndex > textureSize)
        endIndex = textureSize;
    QList<uchar> magmaHeights(textureSize);
    QList<uchar> waterHeights(textureSize);
    QList<uchar> groundHeights(textureSize);
    float multiplier = float(layerDataSize) / float(textureSize);
    for (int i = startIndex; i < endIndex; i++) {
        // Generate layer height arrays
        for (int l = 0; l < textureSize; l++) {
            int layerIndex = (int(i * multiplier) * layerDataSize + int(l * multiplier));
            magmaHeights[l] = int(m_magmaLayer.at(layerIndex));
            waterHeights[l] = int(m_waterLayer.at(layerIndex));
            groundHeights[l] = int(m_groundLayer.at(layerIndex));
        }
        for (int j = 0; j < textureSize / 2; j++) {
            for (int k = 0; k < textureSize; k++) {
                int colorIndex;
                int height((layerDataSize - (j * 2 * multiplier)) / 2);
                if (height < magmaHeights.at(k)) {
                    // Magma layer
                    colorIndex = int((float(height) / heightToColorDiv)
                                     * float(layerColorThickness)) + magmaColorsMin;
                } else if (height < groundHeights.at(k) && height < waterHeights.at(k)) {
                    // Ground layer below water
                    colorIndex = int((float(waterHeights.at(k) - height) / heightToColorDiv)
                                     * float(layerColorThickness)) + underWaterGroundColorsMin;
                } else if (height < waterHeights.at(k)) {
                    // Water layer where water goes over ground
                    colorIndex = int((float(height - magmaHeights.at(k)) / heightToColorDiv)
                                     * float(layerColorThickness)) + waterColorsMin;
                } else if (height <= groundHeights.at(k)) {
                    // Ground above water
                    colorIndex = int((float(height - waterHeights.at(k)) / heightToColorDiv)
                                     * float(layerColorThickness)) + aboveWaterGroundColorsMin;
                } else {
                    // Rest is air
                    colorIndex = airColorIndex;
                }

                (*textureData)[index] = colorIndex;
                index++;
            }
        }
    }
    return endIndex;
}

int VolumetricModifier::excavateMineShaft(int textureSize, int startIndex, int count,
                                          QList<uchar> *textureData)
{
    int endIndex = startIndex + count;
    if (endIndex > m_mineShaftArray.size())
        endIndex = m_mineShaftArray.size();
    int shaftSize = mineShaftDiameter * textureSize / lowDetailSize;
    for (int i = startIndex; i < endIndex; i++) {
        QVector3D shaftStart(m_mineShaftArray.at(i).first);
        QVector3D shaftEnd(m_mineShaftArray.at(i).second);
        int shaftLen = (shaftEnd - shaftStart).length() * lowDetailSize;
        int dataX = shaftStart.x() * textureSize - (shaftSize / 2);
        int dataY = (shaftStart.y() * textureSize - (shaftSize / 2)) / 2;
        int dataZ = shaftStart.z() * textureSize - (shaftSize / 2);
        int dataIndex = dataX + (dataY * textureSize) + dataZ * (textureSize * textureSize / 2);
        if (shaftStart.x() != shaftEnd.x()) {
            for (int j = 0; j <= shaftLen; j++) {
                excavateMineBlock(textureSize, dataIndex, shaftSize, textureData);
                dataIndex += shaftSize;
            }
        } else if (shaftStart.y() != shaftEnd.y()) {
            shaftLen /= 2; // Vertical shafts are half as long
            for (int j = 0; j <= shaftLen; j++) {
                excavateMineBlock(textureSize, dataIndex, shaftSize, textureData);
                dataIndex += textureSize * shaftSize;
            }
        } else {
            for (int j = 0; j <= shaftLen; j++) {
                excavateMineBlock(textureSize, dataIndex, shaftSize, textureData);
                dataIndex += (textureSize * textureSize / 2) * shaftSize;
            }
        }
    }
    return endIndex;
}

void VolumetricModifier::excavateMineBlock(int textureSize, int dataIndex, int size,
                                           QList<uchar> *textureData)
{
    for (int k = 0; k < size; k++) {
        int curIndex = 0;
        for (int l = 0; l < size; l++) {
            curIndex = dataIndex + (k * textureSize * textureSize / 2)
                    + (l * textureSize);
            for (int m = 0; m < size; m++) {
                if (textureData->at(curIndex) != airColorIndex)
                    (*textureData)[curIndex] = mineShaftColorIndex;
                curIndex++;
            }
        }
    }
}

void VolumetricModifier::handleSlicingChanges()
{
    if (m_volumeItem) {
        if (m_slicingX || m_slicingY || m_slicingZ) {
            // Only show slices of selected dimensions
            //! [7]
            m_volumeItem->setDrawSlices(true);
            //! [7]
            m_volumeItem->setSliceIndexX(m_slicingX ? m_sliceIndexX : -1);
            m_volumeItem->setSliceIndexY(m_slicingY ? m_sliceIndexY : -1);
            m_volumeItem->setSliceIndexZ(m_slicingZ ? m_sliceIndexZ : -1);
        } else {
            // Show slice frames for all dimenstions when not actually slicing
            m_volumeItem->setDrawSlices(false);
            m_volumeItem->setSliceIndexX(m_sliceIndexX);
            m_volumeItem->setSliceIndexY(m_sliceIndexY);
            m_volumeItem->setSliceIndexZ(m_sliceIndexZ);
        }
    }
}

void VolumetricModifier::initMineShaftArray()
{
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.7f, 0.1f, 0.7f),
                                                    QVector3D(0.7f, 0.8f, 0.7f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.7f, 0.7f, 0.5f),
                                                    QVector3D(0.7f, 0.7f, 0.7f));

    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.4f, 0.7f, 0.7f),
                                                    QVector3D(0.7f, 0.7f, 0.7f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.4f, 0.7f, 0.7f),
                                                    QVector3D(0.4f, 0.7f, 0.8f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.45f, 0.7f, 0.7f),
                                                    QVector3D(0.45f, 0.7f, 0.8f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.5f, 0.7f, 0.7f),
                                                    QVector3D(0.5f, 0.7f, 0.8f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.55f, 0.7f, 0.7f),
                                                    QVector3D(0.55f, 0.7f, 0.8f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.7f, 0.7f),
                                                    QVector3D(0.6f, 0.7f, 0.8f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.65f, 0.7f, 0.7f),
                                                    QVector3D(0.65f, 0.7f, 0.8f));

    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.5f, 0.6f, 0.7f),
                                                    QVector3D(0.7f, 0.6f, 0.7f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.5f, 0.6f, 0.7f),
                                                    QVector3D(0.5f, 0.6f, 0.8f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.55f, 0.6f, 0.7f),
                                                    QVector3D(0.55f, 0.6f, 0.8f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.6f, 0.7f),
                                                    QVector3D(0.6f, 0.6f, 0.8f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.65f, 0.6f, 0.7f),
                                                    QVector3D(0.65f, 0.6f, 0.8f));

    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.7f, 0.6f, 0.4f),
                                                    QVector3D(0.7f, 0.6f, 0.7f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.6f, 0.45f),
                                                    QVector3D(0.8f, 0.6f, 0.45f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.6f, 0.5f),
                                                    QVector3D(0.8f, 0.6f, 0.5f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.6f, 0.55f),
                                                    QVector3D(0.8f, 0.6f, 0.55f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.6f, 0.6f),
                                                    QVector3D(0.8f, 0.6f, 0.6f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.6f, 0.65f),
                                                    QVector3D(0.8f, 0.6f, 0.65f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.6f, 0.7f),
                                                    QVector3D(0.8f, 0.6f, 0.7f));

    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.7f, 0.7f, 0.4f),
                                                    QVector3D(0.7f, 0.7f, 0.7f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.7f, 0.45f),
                                                    QVector3D(0.8f, 0.7f, 0.45f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.7f, 0.5f),
                                                    QVector3D(0.8f, 0.7f, 0.5f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.7f, 0.55f),
                                                    QVector3D(0.8f, 0.7f, 0.55f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.7f, 0.6f),
                                                    QVector3D(0.8f, 0.7f, 0.6f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.7f, 0.65f),
                                                    QVector3D(0.8f, 0.7f, 0.65f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.7f, 0.7f),
                                                    QVector3D(0.8f, 0.7f, 0.7f));

    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.7f, 0.8f, 0.5f),
                                                    QVector3D(0.7f, 0.8f, 0.7f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.8f, 0.55f),
                                                    QVector3D(0.8f, 0.8f, 0.55f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.8f, 0.6f),
                                                    QVector3D(0.8f, 0.8f, 0.6f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.8f, 0.65f),
                                                    QVector3D(0.8f, 0.8f, 0.65f));
    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.6f, 0.8f, 0.7f),
                                                    QVector3D(0.8f, 0.8f, 0.7f));

    m_mineShaftArray << QPair<QVector3D, QVector3D>(QVector3D(0.7f, 0.1f, 0.4f),
                                                    QVector3D(0.7f, 0.7f, 0.4f));
}
