// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

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

#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtGui/qimage.h>
#include <QtGui/qvector3d.h>

class ShapeManager : public QObject
{
    Q_OBJECT
public:
    explicit ShapeManager(QObject *parent = nullptr);

    enum class SortingMode : quint8
    {
        Random,
        DistanceClosestFirst,
        DistanceClosestLast
    };

    void setImage(const QString &filename);
    void setDepth(float depth);
    void setAmount(int amount);
    void setScale(float scale);
    void setSortingMode(SortingMode mode);
    void setSortingPosition(const QVector3D &position);

    bool loadImage();
    bool generateData();
    bool saveShapeData(const QString &filename);
    void dumpOutput();

private:
    QString m_imageFilename;
    QImage m_image;
    float m_depth = 0.0f;
    int m_amount = -1;
    QString m_cborFilename;
    QList<QVector3D> m_data;
    QList<QVector3D> m_outputData;
    float m_scale = 1.0f;
    SortingMode m_sortingMode = SortingMode::Random;
    QVector3D m_sortingPosition;
};

#endif // SHAPEMANAGER_H
