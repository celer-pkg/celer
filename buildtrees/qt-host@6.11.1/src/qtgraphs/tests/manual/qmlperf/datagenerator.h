// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include <QtCore/QFile>
#include <QtGraphs>

class DataGenerator : public QObject
{
    Q_OBJECT
public:
    DataGenerator(QObject *parent = 0);
    ~DataGenerator() override;

public Q_SLOTS:
    void generateSurfaceData(QSurface3DSeries *series, uint count);
    void generateScatterData(QScatter3DSeries *series, uint count);
    void generateBarData(QBar3DSeries *series, uint count);

    void updateScatterData(QScatter3DSeries *series);
    void updateSurfaceData(QSurface3DSeries *series);
    void updateBarData(QBar3DSeries *series);

    void setFilePath(const QUrl &path);
    void writeLine(const QString &line);
    void writeCSV(const QString &line);

Q_SIGNALS:
    void onMessage(const QString &message);
    void onCaptureInit(long long nanoseconds);

private:
    QFile *m_file = nullptr;
    QFile *m_csv = nullptr;
    QElapsedTimer m_timer;
    int m_cacheCount = 60;

    QList<QSurfaceDataArray> m_surfaceCaches;
    QList<QScatterDataArray> m_scatterCaches;
    QList<QBarDataArray> m_barCaches;

    void populateSurfaceCaches(int sideLength);
    void populateScatterCaches(int sideLength);
    void populateBarChaches(int sideLength);
};

#endif // DATAGENERATOR_H
