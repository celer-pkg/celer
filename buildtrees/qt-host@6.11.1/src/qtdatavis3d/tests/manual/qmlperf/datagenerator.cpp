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
    if (m_surfaceResetArray) {
        for (auto row : *m_surfaceResetArray)
            row->clear();
        m_surfaceResetArray->clear();
    }

    m_surfaceResetArray = new QSurfaceDataArray;
    m_surfaceResetArray->reserve(count);
    for (uint i = 0; i < count; i++) {
        m_surfaceResetArray->append(new QSurfaceDataRow());
        QSurfaceDataRow *row = m_surfaceResetArray->at(i);
        row->reserve(count);
        for (uint j = 0; j < count; j++) {
            float x = float(j) / float(count);
            float z = float(i) / float(count);
            row->append(
                QSurfaceDataItem(QVector3D(x, QRandomGenerator::global()->generateDouble(), z)));
        }
    }

    writeLine(QString("Surface Graph: setting %1 points").arg(count * count));

    m_timer.start();
    series->dataProxy()->resetArray(m_surfaceResetArray);
    long long nsecs = m_timer.nsecsElapsed();

    writeLine(QString("Took %1 nanoseconds").arg(nsecs));

    populateSurfaceCaches(count);
}

void DataGenerator::generateScatterData(QScatter3DSeries *series, uint count)
{
    if (m_scatterResetArray) {
        m_scatterResetArray->clear();
    }
    m_scatterResetArray = new QScatterDataArray;
    m_scatterResetArray->reserve(count * count);
    for (uint i = 0; i < count * count; i++) {
        m_scatterResetArray->append(
            QScatterDataItem(QVector3D(QRandomGenerator::global()->generateDouble() * 2 - 1,
                                       QRandomGenerator::global()->generateDouble() * 2 - 1,
                                       QRandomGenerator::global()->generateDouble() * 2 - 1)));
    }

    writeLine(QString("Scatter Graph: setting %1 points").arg(count * count));

    m_timer.start();
    series->dataProxy()->resetArray(m_scatterResetArray);
    long long nsecs = m_timer.nsecsElapsed();

    writeLine(QString("Took %1 nanoseconds").arg(nsecs));

    populateScatterCaches(count);
}

void DataGenerator::generateBarData(QBar3DSeries *series, uint count)
{
    if (m_barResetArray) {
        for (auto row : *m_barResetArray)
            row->clear();
        m_barResetArray->clear();
    }

    m_barResetArray = new QBarDataArray;
    m_barResetArray->reserve(count);
    for (uint i = 0; i < count; i++) {
        m_barResetArray->append(new QBarDataRow());
        QBarDataRow *row = m_barResetArray->at(i);
        row->reserve(count);
        for (uint j = 0; j < count; j++) {
            row->append(QBarDataItem(QRandomGenerator::global()->generateDouble()));
        }
    }

    writeLine(QString("Bar Graph: setting %1 points").arg(count * count));

    m_timer.start();
    series->dataProxy()->resetArray(m_barResetArray);
    long long nsecs = m_timer.nsecsElapsed();

    writeLine(QString("Took %1 nanoseconds").arg(nsecs));

    populateBarChaches(count);
}

void DataGenerator::updateSurfaceData(QSurface3DSeries *series)
{
    if (!series || series->dataProxy()->columnCount() == 0 || series->dataProxy()->rowCount() == 0)
        return;

    static int index = 0;
    const QSurfaceDataArray &cache = m_surfaceCaches.at(index);
    const int rows = cache.count();
    for (int i = 0; i < rows; i++) {
        const QSurfaceDataRow &sourceRow = *(cache.at(i));
        QSurfaceDataRow &row = *(*m_surfaceResetArray)[i];
        std::copy(sourceRow.cbegin(), sourceRow.cend(), row.begin());
    }

    series->dataProxy()->resetArray(m_surfaceResetArray);

    index++;
    if (index >= m_cacheCount)
        index = 0;
}
void DataGenerator::updateScatterData(QScatter3DSeries *series)
{
    if (!series || series->dataProxy()->array()->count() == 0)
        return;

    static int index = 0;
    const QScatterDataArray &cache = m_scatterCaches.at(index);

    const int count = cache.count();
    for (int i = 0; i < count; i++) {
        (*m_scatterResetArray)[i].setPosition(cache.at(i).position());
    }

    series->dataProxy()->resetArray(m_scatterResetArray);
    index++;
    if (index >= m_cacheCount)
        index = 0;
}
void DataGenerator::updateBarData(QBar3DSeries *series)
{
    static int index = 0;
    const int rows = series->dataProxy()->rowCount();

    const QBarDataArray &cache = m_barCaches.at(index);
    for (int i = 0; i < rows; i++) {
        const QBarDataRow &sourceRow = *(cache.at(i));
        QBarDataRow &row = *(*m_barResetArray)[i];
        std::copy(sourceRow.cbegin(), sourceRow.cend(), row.begin());
    }

    series->dataProxy()->resetArray(m_barResetArray);
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
    m_surfaceCaches.clear();

    // Re-create the cache array
    m_surfaceCaches.resize(m_cacheCount);
    for (int i = 0; i < m_cacheCount; i++) {
        QSurfaceDataArray &array = m_surfaceCaches[i];
        array.reserve(sideLength);
        for (int j = 0; j < sideLength; j++) {
            array.append(new QSurfaceDataRow(sideLength));
        }
    }

    //Populate caches
    for (int i = 0; i < m_cacheCount; i++) {
        QSurfaceDataArray &cache = m_surfaceCaches[i];
        float timeStep = float(i) / float(m_cacheCount);
        for (int j = 0; j < sideLength; j++) {
            QSurfaceDataRow &row = *(cache[j]);
            for (int k = 0; k < sideLength; k++) {
                float x = float(k) / float(sideLength);
                float z = float(j) / float(sideLength);
                float y = qSin(2 * M_PI * (x + z + (timeStep))) * 0.5 + 0.5;
                row[k] = QSurfaceDataItem(QVector3D(x, y, z));
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
    m_scatterCaches.clear();

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
    m_barCaches.clear();

    // Re-create the cache array
    m_barCaches.resize(m_cacheCount);
    for (int i = 0; i < m_cacheCount; i++) {
        QBarDataArray &array = m_barCaches[i];
        array.reserve(sideLength);
        for (int j = 0; j < sideLength; j++) {
            array.append(new QBarDataRow(sideLength));
        }
    }
    for (int i = 0; i < m_cacheCount; i++) {
        QBarDataArray &cache = m_barCaches[i];
        float timeStep = float(i) / float(m_cacheCount);
        for (int j = 0; j < sideLength; j++) {
            QBarDataRow &row = *(cache[j]);
            for (int k = 0; k < sideLength; k++) {
                float x = float(j) / float(sideLength);
                float z = float(k) / float(sideLength);
                float y = qSin(2 * M_PI * (x + z + (timeStep))) * 0.5 + 0.5;
                row[k] = QBarDataItem(y);
            }
        }
    }
}
