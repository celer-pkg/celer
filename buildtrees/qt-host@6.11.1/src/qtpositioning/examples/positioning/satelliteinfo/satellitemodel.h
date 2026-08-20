// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SATELLITEMODEL_H
#define SATELLITEMODEL_H

#include <QAbstractListModel>
#include <QGeoSatelliteInfo>
#include <QtQml/qqmlregistration.h>

#include <utility>

//! [0]
class SatelliteModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int size READ rowCount NOTIFY sizeChanged)
    QML_ELEMENT
public:
    explicit SatelliteModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void updateSatellitesInView(const QList<QGeoSatelliteInfo> &inView);
    void updateSatellitesInUse(const QList<QGeoSatelliteInfo> &inUse);

signals:
    void sizeChanged();
//! [0]

private:
    QList<QGeoSatelliteInfo> m_satellites;

    using SatelliteId = int;
    using SystemId = int;
    using UniqueId = std::pair<SystemId, SatelliteId>;
    static UniqueId getUid(const QGeoSatelliteInfo &info);
    QSet<UniqueId> m_inUseIds;
    QSet<UniqueId> m_allIds;
//! [1]
};
//! [1]

#endif // SATELLITEMODEL_H
