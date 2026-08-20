// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "scatterdatamodifier.h"

#include <QtGraphs/QScatterDataProxy>
#include <QtGraphs/QValue3DAxis>
#include <QtGraphs/Q3DScene>
#include <QtGraphs/QScatter3DSeries>
#include <QtCore/qmath.h>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>

ScatterDataModifier::ScatterDataModifier(Q3DScatterWidgetItem *scatter)
    : m_graph(scatter)
{
    m_graph->activeTheme()->setTheme(QGraphsTheme::Theme::QtGreen);
    m_graph->setShadowQuality(QtGraphs3D::ShadowQuality::Medium);
    m_graph->setCameraPreset(QtGraphs3D::CameraPreset::Front);

    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);

    m_graph->axisX()->setRange(-10.0f, 10.0f);
    m_graph->axisY()->setRange(-5.0f, 5.0f);
    m_graph->axisZ()->setRange(-5.0f, 5.0f);

    QScatter3DSeries *series = new QScatter3DSeries;
    series->setItemLabelFormat(QStringLiteral("@xLabel, @yLabel, @zLabel"));
    series->setMesh(QAbstract3DSeries::Mesh::Cube);
    series->setItemSize(0.15f);
    m_graph->addSeries(series);

    //! [2]
    m_animationCameraX = new QPropertyAnimation(m_graph, "cameraXRotation");
    m_animationCameraX->setDuration(20000);
    m_animationCameraX->setStartValue(QVariant::fromValue(0.0f));
    m_animationCameraX->setEndValue(QVariant::fromValue(360.0f));
    m_animationCameraX->setLoopCount(-1);
    //! [2]

    //! [3]
    QPropertyAnimation *upAnimation = new QPropertyAnimation(m_graph, "cameraYRotation");
    upAnimation->setDuration(9000);
    upAnimation->setStartValue(QVariant::fromValue(5.0f));
    upAnimation->setEndValue(QVariant::fromValue(45.0f));

    QPropertyAnimation *downAnimation = new QPropertyAnimation(m_graph, "cameraYRotation");
    downAnimation->setDuration(9000);
    downAnimation->setStartValue(QVariant::fromValue(45.0f));
    downAnimation->setEndValue(QVariant::fromValue(5.0f));

    m_animationCameraY = new QSequentialAnimationGroup();
    m_animationCameraY->setLoopCount(-1);
    m_animationCameraY->addAnimation(upAnimation);
    m_animationCameraY->addAnimation(downAnimation);
    //! [3]

    m_animationCameraX->start();
    m_animationCameraY->start();

    // Give ownership of the handler to the graph and make it the active handler
    //! [0]
    m_graph->unsetDefaultWheelHandler();
    QObject::connect(m_graph, &Q3DGraphsWidgetItem::wheel, this, &ScatterDataModifier::onWheel);
    QObject::connect(m_graph, &Q3DGraphsWidgetItem::mouseMove, this, &ScatterDataModifier::onMouseMove);
    QObject::connect(m_graph, &Q3DGraphsWidgetItem::tapped, this, &ScatterDataModifier::onTapped);
    QObject::connect(m_graph,
                     &Q3DGraphsWidgetItem::queriedGraphPositionChanged,
                     this,
                     &ScatterDataModifier::onPositionQueryChanged);
    //! [0]
}

ScatterDataModifier::~ScatterDataModifier()
{
    delete m_graph;
}

void ScatterDataModifier::start()
{
    addData();
}

void ScatterDataModifier::addData()
{
    QList<QVector3D> itemList;

    // Read data items from the file to QList
    QTextStream stream;
    QFile dataFile(":/data/data.txt");
    if (dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        stream.setDevice(&dataFile);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.startsWith("#")) // Ignore comments
                continue;
            QStringList strList = line.split(",", Qt::SkipEmptyParts);
            // Each line has three data items: xPos, yPos and zPos value
            if (strList.size() < 3) {
                qWarning() << "Invalid row read from data:" << line;
                continue;
            }
            itemList.append(QVector3D(
                                 strList.at(0).trimmed().toFloat(),
                                 strList.at(1).trimmed().toFloat(),
                                 strList.at(2).trimmed().toFloat()));
        }
    } else {
        qWarning() << "Unable to open data file:" << dataFile.fileName();
    }

    // Add data from the QList to datamodel
    QScatterDataArray dataArray;
    dataArray.resize(itemList.size());

    for (int i = 0; i < itemList.size(); i++)
        dataArray[i].setPosition(itemList.at(i));

    m_graph->seriesList().at(0)->dataProxy()->resetArray(dataArray);
}

void ScatterDataModifier::toggleCameraAnimation()
{
    if (m_animationCameraX->state() != QAbstractAnimation::Paused) {
        m_animationCameraX->pause();
        m_animationCameraY->pause();
    } else {
        m_animationCameraX->resume();
        m_animationCameraY->resume();
    }
}

void ScatterDataModifier::onWheel(QWheelEvent *event)
{
    // Adjust zoom level based on what zoom range we're in.
    int zoomLevel = m_graph->cameraZoomLevel();
    if (zoomLevel > 100)
        zoomLevel += event->angleDelta().y() / 12;
    else if (zoomLevel > 50)
        zoomLevel += event->angleDelta().y() / 60;
    else
        zoomLevel += event->angleDelta().y() / 120;
    if (zoomLevel > 500)
        zoomLevel = 500;
    else if (zoomLevel < 10)
        zoomLevel = 10;

    m_graph->setCameraZoomLevel(zoomLevel);
}

void ScatterDataModifier::onMouseMove(QPoint mousePos)
{
    m_mousePos = mousePos;
    m_graph->doPicking(mousePos);
}

void ScatterDataModifier::onTapped(QEventPoint eventPoint, Qt::MouseButton button)
{
    Q_UNUSED(button);
    QPoint point = eventPoint.position().toPoint();
    qDebug() << "Queried at: " << point;
    m_graph->scene()->setGraphPositionQuery(point);
}

void ScatterDataModifier::onPositionQueryChanged(const QVector3D &position)
{
    qDebug() << "Queried Position from signal:" << position;
    qDebug() << "Queried Position from graph :" << m_graph->queriedGraphPosition();
}

void ScatterDataModifier::shadowQualityUpdatedByVisual(QtGraphs3D::ShadowQuality sq)
{
    int quality = int(sq);
    emit shadowQualityChanged(quality); // connected to a checkbox in main.cpp
}

void ScatterDataModifier::changeShadowQuality(int quality)
{
    QtGraphs3D::ShadowQuality sq = QtGraphs3D::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
}
