// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "datagenerator.h"
#include <QDebug>
#include <QRandomGenerator>

Q_DECLARE_METATYPE(QScatter3DSeries *)

DataGenerator::DataGenerator(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QScatter3DSeries *>();
    qRegisterMetaType<QSurface3DSeries *>();
    qRegisterMetaType<QBar3DSeries *>();
    setFilePath(QUrl());
}

DataGenerator::~DataGenerator()
{
    m_file->close();
    m_csv->close();
    delete m_file;
    delete m_csv;
}

void DataGenerator::generateSurfaceData(QSurface3DSeries *series, uint count)
{
    QSurfaceDataArray dataArray;
    dataArray.reserve(count);
    for (uint i = 0; i < count; i++) {
        dataArray.append(QSurfaceDataRow(count));
        for (uint j = 0; j < count; j++) {
            float x = float(j) / float(count);
            float z = float(i) / float(count);
            dataArray[i][j].setPosition(
                QVector3D(x, QRandomGenerator::global()->generateDouble(), z));
        }
    }

    writeLine(QString("Surface Graph: setting %1 points").arg(count * count));

    m_timer.start();
    series->dataProxy()->resetArray(dataArray);
    long long nsecs = m_timer.nsecsElapsed();

    writeLine(QString("Took %1 nanoseconds").arg(nsecs));
    emit onCaptureInit(nsecs);

    populateSurfaceCaches(count);
}

void DataGenerator::generateScatterData(QScatter3DSeries *series, uint count)
{
    QScatterDataArray dataArray;
    dataArray.resize(count * count);

    for (uint i = 0; i < count * count; i++) {
        dataArray[i].setPosition(QVector3D(QRandomGenerator::global()->generateDouble() * 2 - 1,
                                           QRandomGenerator::global()->generateDouble() * 2 - 1,
                                           QRandomGenerator::global()->generateDouble() * 2 - 1));
    }

    writeLine(QString("Scatter Graph: setting %1 points").arg(count * count));

    m_timer.start();
    series->dataProxy()->resetArray(dataArray);
    long long nsecs = m_timer.nsecsElapsed();

    writeLine(QString("Took %1 nanoseconds").arg(nsecs));
    emit onCaptureInit(nsecs);

    populateScatterCaches(count);
}

void DataGenerator::generateBarData(QBar3DSeries *series, uint count)
{
    QBarDataArray dataArray;
    dataArray.reserve(count);

    for (uint i = 0; i < count; i++) {
        dataArray.append(QBarDataRow(count));
        for (uint j = 0; j < count; j++) {
            dataArray[i][j].setValue(QRandomGenerator::global()->generateDouble());
        }
    }

    writeLine(QString("Scatter Graph: setting %1 points").arg(count * count));

    m_timer.start();
    series->dataProxy()->resetArray(dataArray);
    long long nsecs = m_timer.nsecsElapsed();

    writeLine(QString("Took %1 nanoseconds").arg(nsecs));
    emit onCaptureInit(nsecs);

    populateBarChaches(count);
}

void DataGenerator::updateSurfaceData(QSurface3DSeries *series)
{
    if (!series || series->dataProxy()->columnCount() == 0 || series->dataProxy()->rowCount() == 0)
        return;

    static int index = 0;

    const int rows = series->dataProxy()->rowCount();

    const QSurfaceDataArray &cache = m_surfaceCaches.at(index);
    QSurfaceDataArray newArray;
    newArray.reserve(rows);
    for (int i = 0; i < rows; i++) {
        newArray.append(cache.at(i));
    }

    series->dataProxy()->resetArray(newArray);

    index++;
    if (index >= m_cacheCount)
        index = 0;
}
void DataGenerator::updateScatterData(QScatter3DSeries *series)
{
    if (!series || series->dataProxy()->itemCount() == 0)
        return;

    static int index = 0;
    const int count = series->dataProxy()->itemCount();

    QScatterDataArray newArray;
    newArray.reserve(count);

    const QScatterDataArray &cache = m_scatterCaches.at(index);
    for (int i = 0; i < count; i++) {
        newArray.append(cache.at(i));
    }

    series->dataProxy()->resetArray(newArray);
    index++;
    if (index >= m_cacheCount)
        index = 0;
}
void DataGenerator::updateBarData(QBar3DSeries *series)
{
    if (!series || series->dataProxy()->colCount() == 0 || series->dataProxy()->rowCount() == 0)
        return;

    static int index = 0;
    const int rows = series->dataProxy()->rowCount();

    const QBarDataArray &cache = m_barCaches.at(index);
    QBarDataArray newArray;
    newArray.reserve(rows);

    for (int i = 0; i < rows; i++) {
        newArray.append(cache.at(i));
    }

    series->dataProxy()->resetArray(newArray);
    index++;
    if (index >= m_cacheCount)
        index = 0;
}

void DataGenerator::setFilePath(const QUrl &path)
{
    if (m_file) {
        m_file->close();
        delete m_file;
    }
    if (m_csv) {
        m_csv->close();
        delete m_csv;
    }

    QString pathString = path.toLocalFile();
    if (!pathString.isEmpty()) {
        pathString += "/";
        qDebug() << "Set path to : " << pathString;
        emit onMessage("Set path to " + pathString);
    }

    m_file = new QFile(pathString + "results.txt");
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << m_file->errorString();
        delete m_file;
        m_file = 0;
    }
    m_csv = new QFile(pathString + "measurements.csv");
    if (!m_csv->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << m_file->errorString();
        delete m_csv;
        m_csv = 0;
    } else {
        QTextStream out(m_csv);
        QString headers = QString("Graph type,Number of points,Optimization,MSAA "
                                  "Samples,Shadow Quality,Init Time,Average FPS");
        out << headers << Qt::endl;
    }
}

void DataGenerator::writeLine(const QString &line)
{
    if (m_file) {
        QTextStream out(m_file);

        qDebug() << line << Qt::endl;
        out << line << Qt::endl;
        emit onMessage(line);
    }
}

void DataGenerator::writeCSV(const QString &line)
{
    if (m_csv) {
        qDebug() << line << Qt::endl;
        QTextStream out(m_csv);
        out << line << Qt::endl;
    }
}

void DataGenerator::populateSurfaceCaches(int sideLength)
{
    for (int i = 0; i < m_surfaceCaches.size(); i++) {
        QSurfaceDataArray &array = m_surfaceCaches[i];
        array.clear();
    }

    // Re-create the cache array
    m_surfaceCaches.resize(m_cacheCount);
    for (int i = 0; i < m_cacheCount; i++) {
        QSurfaceDataArray &array = m_surfaceCaches[i];
        array.reserve(sideLength);
        for (int j = 0; j < sideLength; j++) {
            array.append(QSurfaceDataRow(sideLength));
        }
    }

    //Populate caches
    for (int i = 0; i < m_cacheCount; i++) {
        QSurfaceDataArray &cache = m_surfaceCaches[i];
        float timeStep = float(i) / float(m_cacheCount);
        for (int j = 0; j < sideLength; j++) {
            QSurfaceDataRow &row = cache[j];
            for (int k = 0; k < sideLength; k++) {
                float x = float(k) / float(sideLength);
                float z = float(j) / float(sideLength);
                float y = qSin(2 * M_PI * (x + z + (timeStep))) * 0.5 + 0.5;
                row[k] = QSurfaceDataItem(x, y, z);
            }
        }
    }
}

void DataGenerator::populateScatterCaches(int sideLength)
{
    for (int i = 0; i < m_scatterCaches.size(); i++) {
        QScatterDataArray &array = m_scatterCaches[i];
        array.clear();
    }

    // Re-create the cache array
    const int count = sideLength * sideLength;
    m_scatterCaches.resize(m_cacheCount);
    for (int i = 0; i < m_cacheCount; i++) {
        QScatterDataArray &array = m_scatterCaches[i];
        array.reserve(count);
        for (int j = 0; j < count; j++) {
            array.append(QScatterDataItem());
        }
    }

    //Populate caches
    for (int i = 0; i < m_cacheCount; i++) {
        // time loops from 0 to 4
        float t = (float(i) * 4) / float(m_cacheCount);
        QScatterDataArray &cache = m_scatterCaches[i];
        for (int j = 0; j < sideLength; j++) {
            for (int k = 0; k < sideLength; k++) {
                float u = (float(j) / float(sideLength)) * 2 - 1;
                float v = (float(k) / float(sideLength)) * 2 - 1;

                //create a torus
                float r1 = 0.7f + 0.1f * qSin(M_PI * (6.0f * u + 0.5f * t));
                float r2 = 0.15f + 0.05f * qSin(M_PI * (8.0f * u + 4.0f * v + 2.0f * t));
                float s = r1 + r2 * qCos(M_PI * v);

                float x = s * qSin(M_PI * u);
                float y = r2 * qSin(M_PI * v);
                float z = s * qCos(M_PI * u);
                cache[sideLength * j + k].setPosition(QVector3D(x, y, z));
            }
        }
    }
}

void DataGenerator::populateBarChaches(int sideLength)
{
    for (int i = 0; i < m_barCaches.size(); i++) {
        QBarDataArray &array = m_barCaches[i];
        array.clear();
    }

    // Re-create the cache array
    m_barCaches.resize(m_cacheCount);
    for (int i = 0; i < m_cacheCount; i++) {
        QBarDataArray &array = m_barCaches[i];
        array.reserve(sideLength);
        for (int j = 0; j < sideLength; j++) {
            array.append(QBarDataRow(sideLength));
        }
    }
    for (int i = 0; i < m_cacheCount; i++) {
        QBarDataArray &cache = m_barCaches[i];
        float timeStep = float(i) / float(m_cacheCount);
        for (int j = 0; j < sideLength; j++) {
            QBarDataRow &row = cache[j];
            for (int k = 0; k < sideLength; k++) {
                float x = float(j) / float(sideLength);
                float z = float(k) / float(sideLength);
                float y = qSin(2 * M_PI * (x + z + (timeStep))) * 0.5 + 0.5;
                row[k] = QBarDataItem(y);
            }
        }
    }
}
