// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "declarativescatter_p.h"
#include <QtCore/QMutexLocker>

QT_BEGIN_NAMESPACE

DeclarativeScatter::DeclarativeScatter(QQuickItem *parent)
    : AbstractDeclarative(parent),
      m_scatterController(0)
{
    setAcceptedMouseButtons(Qt::AllButtons);

    // Create the shared component on the main GUI thread.
    m_scatterController = new Scatter3DController(boundingRect().toRect(), new Declarative3DScene);
    setSharedController(m_scatterController);

    QObject::connect(m_scatterController, &Scatter3DController::selectedSeriesChanged,
                     this, &DeclarativeScatter::selectedSeriesChanged);
}

DeclarativeScatter::~DeclarativeScatter()
{
    QMutexLocker locker(m_nodeMutex.data());
    const QMutexLocker locker2(mutex());
    delete m_scatterController;
}

QValue3DAxis *DeclarativeScatter::axisX() const
{
    return static_cast<QValue3DAxis *>(m_scatterController->axisX());
}

void DeclarativeScatter::setAxisX(QValue3DAxis *axis)
{
    m_scatterController->setAxisX(axis);
}

QValue3DAxis *DeclarativeScatter::axisY() const
{
    return static_cast<QValue3DAxis *>(m_scatterController->axisY());
}

void DeclarativeScatter::setAxisY(QValue3DAxis *axis)
{
    m_scatterController->setAxisY(axis);
}

QValue3DAxis *DeclarativeScatter::axisZ() const
{
    return static_cast<QValue3DAxis *>(m_scatterController->axisZ());
}

void DeclarativeScatter::setAxisZ(QValue3DAxis *axis)
{
    m_scatterController->setAxisZ(axis);
}

QScatter3DSeries *DeclarativeScatter::selectedSeries() const
{
    return m_scatterController->selectedSeries();
}

QQmlListProperty<QScatter3DSeries> DeclarativeScatter::seriesList()
{
    return QQmlListProperty<QScatter3DSeries>(this, this,
                                              &DeclarativeScatter::appendSeriesFunc,
                                              &DeclarativeScatter::countSeriesFunc,
                                              &DeclarativeScatter::atSeriesFunc,
                                              &DeclarativeScatter::clearSeriesFunc);
}

void DeclarativeScatter::appendSeriesFunc(QQmlListProperty<QScatter3DSeries> *list,
                                          QScatter3DSeries *series)
{
    reinterpret_cast<DeclarativeScatter *>(list->data)->addSeries(series);
}

qsizetype DeclarativeScatter::countSeriesFunc(QQmlListProperty<QScatter3DSeries> *list)
{
    return reinterpret_cast<DeclarativeScatter *>(list->data)->m_scatterController->scatterSeriesList().size();
}

QScatter3DSeries *DeclarativeScatter::atSeriesFunc(QQmlListProperty<QScatter3DSeries> *list,
                                                   qsizetype index)
{
    return reinterpret_cast<DeclarativeScatter *>(list->data)->m_scatterController->scatterSeriesList().at(index);
}

void DeclarativeScatter::clearSeriesFunc(QQmlListProperty<QScatter3DSeries> *list)
{
    DeclarativeScatter *declScatter = reinterpret_cast<DeclarativeScatter *>(list->data);
    QList<QScatter3DSeries *> realList = declScatter->m_scatterController->scatterSeriesList();
    int count = realList.size();
    for (int i = 0; i < count; i++)
        declScatter->removeSeries(realList.at(i));
}

void DeclarativeScatter::addSeries(QScatter3DSeries *series)
{
    m_scatterController->addSeries(series);
}

void DeclarativeScatter::removeSeries(QScatter3DSeries *series)
{
    m_scatterController->removeSeries(series);
    series->setParent(this); // Reparent as removing will leave series parentless
}

void DeclarativeScatter::handleAxisXChanged(QAbstract3DAxis *axis)
{
    emit axisXChanged(static_cast<QValue3DAxis *>(axis));
}

void DeclarativeScatter::handleAxisYChanged(QAbstract3DAxis *axis)
{
    emit axisYChanged(static_cast<QValue3DAxis *>(axis));
}

void DeclarativeScatter::handleAxisZChanged(QAbstract3DAxis *axis)
{
    emit axisZChanged(static_cast<QValue3DAxis *>(axis));
}

QT_END_NAMESPACE
