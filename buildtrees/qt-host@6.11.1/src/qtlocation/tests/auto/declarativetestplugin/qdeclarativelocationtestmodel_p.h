// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QDECLARATIVELOCATIONTESTMODEL_H
#define QDECLARATIVELOCATIONTESTMODEL_H

#include <QtQuick/QQuickItem>
#include <QAbstractListModel>
#include <QTimer>
#include <QDebug>
#include <QList>
#include <QtPositioning/QGeoCoordinate>

QT_BEGIN_NAMESPACE

class DataObject: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

public:
    DataObject() {}
    ~DataObject() {}

    QGeoCoordinate coordinate_;
    QGeoCoordinate coordinate() const {return coordinate_;}
};

class QDeclarativeLocationTestModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(int datacount READ datacount WRITE setDatacount NOTIFY datacountChanged)
    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)
    Q_PROPERTY(bool crazyMode READ crazyMode WRITE setCrazyMode NOTIFY crazyModeChanged)
    Q_PROPERTY(int crazyLevel READ crazyLevel WRITE setCrazyLevel NOTIFY crazyLevelChanged)
    Q_PROPERTY(QString datatype READ datatype WRITE setDatatype NOTIFY datatypeChanged)
    Q_INTERFACES(QQmlParserStatus)

public:
    QDeclarativeLocationTestModel(QObject* parent = nullptr);
    ~QDeclarativeLocationTestModel();

    enum Roles {
        TestDataRole = Qt::UserRole + 500
    };

    // from QQmlParserStatus
    void componentComplete() override;
    void classBegin() override {}

    // From QAbstractListModel
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int datacount() const;
    void setDatacount(int datacount);

    int delay() const;
    void setDelay(int delay);

    int crazyLevel() const;
    void setCrazyLevel(int level);

    bool crazyMode() const;
    void setCrazyMode(bool mode);

    QString datatype() const;
    void setDatatype(QString datatype);

    //Q_INVOKABLE void clear();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void update();
    //Q_INVOKABLE void reset();

signals:
    void countChanged();
    void datacountChanged();
    void datatypeChanged();
    void delayChanged();
    void modelChanged();
    void crazyLevelChanged();
    void crazyModeChanged();

private slots:
    void repopulate();
    void timerFired();

private:
    void scheduleRepopulation();

private:
    int delay_;
    int datacount_;
    bool componentCompleted_;
    QString datatype_;
    QTimer timer_;
    QList<DataObject*> dataobjects_;
    int crazyLevel_;
    bool crazyMode_;
};

QT_END_NAMESPACE

#endif
