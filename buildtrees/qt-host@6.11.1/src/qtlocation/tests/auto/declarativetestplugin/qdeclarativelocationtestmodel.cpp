// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qdeclarativelocationtestmodel_p.h"
#include <qgeocoordinate.h>
#include <QtCore/QTime>
#include <QtCore>

QT_BEGIN_NAMESPACE

QDeclarativeLocationTestModel::QDeclarativeLocationTestModel(QObject *parent):
     QAbstractListModel(parent),
    delay_(0),
    datacount_(0),
    componentCompleted_(false),
    crazyLevel_(0),
    crazyMode_(false)
{
    timer_.setSingleShot(true);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(timerFired()));
}

QDeclarativeLocationTestModel::~QDeclarativeLocationTestModel()
{
    if (timer_.isActive())
        timer_.stop();
    if (!dataobjects_.isEmpty()) {
        qDeleteAll(dataobjects_);
        dataobjects_.clear();
    }
}

void QDeclarativeLocationTestModel::timerFired()
{
    //qDebug() << "timer fired" ;
    repopulate();
    if (crazyMode_) {
        int delay = (QRandomGenerator::global()->bounded(uint(INT_MAX) + 1) % crazyLevel_); // writing software is exact science
        delay = qMax(1000, delay); // 3 ms at minimum
        qDebug() << "starting timer with : " << delay;
        timer_.start(delay);
    }
}

void QDeclarativeLocationTestModel::componentComplete()
{
    componentCompleted_ = true;
    scheduleRepopulation();
}


int QDeclarativeLocationTestModel::datacount() const
{
    return datacount_;
}

void QDeclarativeLocationTestModel::setDatacount(int datacount)
{
    if (datacount_ == datacount)
        return;
    datacount_ = datacount;
    emit datacountChanged();
    scheduleRepopulation();
}

int QDeclarativeLocationTestModel::delay() const
{
    return delay_;
}

void QDeclarativeLocationTestModel::setDelay(int delay)
{
    if (delay_ == delay)
        return;
    delay_ = delay;
    emit delayChanged();
}

QString QDeclarativeLocationTestModel::datatype() const
{
    return datatype_;
}

void QDeclarativeLocationTestModel::setDatatype(QString datatype)
{
    if (datatype_ == datatype)
        return;
    datatype_ = datatype;
    emit datatypeChanged();
    scheduleRepopulation();
}

int QDeclarativeLocationTestModel::crazyLevel() const
{
    return crazyLevel_;
}

void QDeclarativeLocationTestModel::setCrazyLevel(int level)
{
    if (level == crazyLevel_)
        return;
    crazyLevel_ = level;
    reset();
    scheduleRepopulation();
    emit crazyLevelChanged();
}

bool QDeclarativeLocationTestModel::crazyMode() const
{
    return crazyMode_;
}

void QDeclarativeLocationTestModel::setCrazyMode(bool mode)
{
    if (mode == crazyMode_)
        return;
    crazyMode_ = mode;
    //if (!crazyMode_)
        //reset();
    //else
    if (crazyMode_)
        scheduleRepopulation();
    emit crazyModeChanged();
}

// only coordinate datatype for now to get started with,
// refactor if more usecases for the model emerge.
void QDeclarativeLocationTestModel::repopulate()
{
    double latitude = -30;
    double longitude = 153;
    beginResetModel();
    if (!dataobjects_.isEmpty()) {
        qDeleteAll(dataobjects_);
        dataobjects_.clear();
    }
    int datacount = datacount_;
    if (crazyMode_)
        datacount = QRandomGenerator::global()->bounded(datacount_);

    for (int i = 0; i < datacount; ++i) {
        DataObject* dataobject = new DataObject;
        dataobject->coordinate_ = QGeoCoordinate(latitude, longitude);
        dataobjects_.append(dataobject);
        longitude -= 0.2;
        latitude += 0.2;
    }
    endResetModel();
}

void QDeclarativeLocationTestModel::update()
{
    scheduleRepopulation();
}

void QDeclarativeLocationTestModel::reset()
{
    if (timer_.isActive())
        timer_.stop();
    beginResetModel();
    if (!dataobjects_.isEmpty()) {
        qDeleteAll(dataobjects_);
        dataobjects_.clear();
    }
    endResetModel();
}

void QDeclarativeLocationTestModel::scheduleRepopulation()
{
    if (!componentCompleted_)
        return;

    if (datacount_ <= 0)
        return;

    if (timer_.isActive())
        timer_.stop();

    if (crazyMode_) {
        // start generating arbitrary amount of data at arbitrary intervals
        int delay = QRandomGenerator::global()->bounded(crazyLevel_); // writing software is exact science
        delay = qMax(3, delay); // 3 ms at minimum
        qDebug() << "starting timer with : " << delay;
        timer_.start(delay);
    } else {
        // just update
        if (delay_ > 0)
            timer_.start(delay_);
        else
            repopulate();
    }
}

int QDeclarativeLocationTestModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return dataobjects_.count();
}

// Returns the stored under the given role for the item referred to by the index.
QVariant QDeclarativeLocationTestModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case TestDataRole:
        if (dataobjects_.at(index.row())) {
            return QVariant::fromValue(qobject_cast<QObject*>(dataobjects_.at(index.row())));
        }
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> QDeclarativeLocationTestModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(TestDataRole, "modeldata");
    return roles;
}

QT_END_NAMESPACE
