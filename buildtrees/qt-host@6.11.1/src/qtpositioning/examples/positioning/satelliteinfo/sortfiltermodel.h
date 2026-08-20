// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SORTFILTERMODEL_H
#define SORTFILTERMODEL_H

#include <QGeoSatelliteInfo>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QQmlEngine>

//! [0]
class SortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit SortFilterModel(QObject *parent = nullptr);

public slots:
    void updateFilterString(const QString &str);
    void updateShowInView(bool show);
    void updateShowInUse(bool show);
    void updateSelectedSystems(int id, bool show);
    void updateSortRoles(int role, bool use);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
//! [0]

private:
    QString m_filterString;
    QSet<int> m_selectedSystems = {
        QGeoSatelliteInfo::GPS,
        QGeoSatelliteInfo::GLONASS,
        QGeoSatelliteInfo::GALILEO,
        QGeoSatelliteInfo::BEIDOU,
        QGeoSatelliteInfo::QZSS
    };
    bool m_showInView = true;
    bool m_showInUse = true;
    QSet<int> m_sortRoles;
//! [1]
};
//! [1]

#endif // SORTFILTERMODEL_H
