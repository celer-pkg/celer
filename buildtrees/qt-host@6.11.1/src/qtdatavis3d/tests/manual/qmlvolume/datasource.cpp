// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "datasource.h"
#include <QtCore/qmath.h>
#include <QtGui/QRgb>
#include <QtGui/QVector3D>

Q_DECLARE_METATYPE(QCustom3DVolume *)

DataSource::DataSource(QObject *parent) :
    QObject(parent)
{
    qRegisterMetaType<QCustom3DVolume *>();
}

DataSource::~DataSource()
{
}

void DataSource::fillVolume(QCustom3DVolume *volumeItem)
{
    // Generate example texture data for an half-ellipsoid with a section missing.
    // This can take a while if the dimensions are large, so we support incremental data generation.

    int index = 0;
    int textureSize = 1024;
    QVector3D midPoint(float(textureSize) / 2.0f,
                       float(textureSize) / 2.0f,
                       float(textureSize) / 2.0f);

    QList<uchar> *textureData = new QList<uchar>(textureSize * textureSize * textureSize / 2);
    for (int i = 0; i < textureSize; i++) {
        for (int j = 0; j < textureSize / 2; j++) {
            for (int k = 0; k < textureSize; k++) {
                int colorIndex = 0;
                // Take a section out of the ellipsoid
                if (i >= textureSize / 2 || j >= textureSize / 4 || k >= textureSize / 2) {
                    QVector3D distVec = QVector3D(float(k), float(j * 2), float(i)) - midPoint;
                    float adjLen = qMin(255.0f, (distVec.length() * 512.0f / float(textureSize)));
                    colorIndex = 255 - int(adjLen);
                }

                (*textureData)[index] = colorIndex;
                index++;
            }
        }
    }

    volumeItem->setScaling(QVector3D(2.0f, 1.0f, 2.0f));
    volumeItem->setTextureWidth(textureSize);
    volumeItem->setTextureHeight(textureSize / 2);
    volumeItem->setTextureDepth(textureSize);
    volumeItem->setTextureFormat(QImage::Format_Indexed8);
    volumeItem->setTextureData(textureData);

    QList<QRgb> colorTable(256);

    for (int i = 1; i < 256; i++) {
        if (i < 15)
            colorTable[i] = qRgba(0, 0, 0, 0);
        else if (i < 60)
            colorTable[i] = qRgba((i * 2) + 120, 0, 0, 15);
        else if (i < 120)
            colorTable[i] = qRgba(0, ((i - 60) * 2) + 120, 0, 50);
        else if (i < 180)
            colorTable[i] = qRgba(0, 0, ((i - 120) * 2) + 120, 255);
        else
            colorTable[i] = qRgba(i, i, i, 255);
    }

    volumeItem->setColorTable(colorTable);
}
