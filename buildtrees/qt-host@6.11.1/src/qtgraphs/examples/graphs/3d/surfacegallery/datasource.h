// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QtGraphs/qsurface3dseries.h>
#include <QtQml/qqmlregistration.h>

//! [0]
//! [2]
class DataSource : public QObject
{
    Q_OBJECT
    //! [0]
    QML_ELEMENT
    //! [2]
public:
    explicit DataSource(QObject *parent = 0);
    ~DataSource() override;

    //! [1]
    Q_INVOKABLE void generateData(int cacheCount,
                                  int rowCount,
                                  int columnCount,
                                  float xMin,
                                  float xMax,
                                  float yMin,
                                  float yMax,
                                  float zMin,
                                  float zMax);

    Q_INVOKABLE void update(QSurface3DSeries *series);
    //! [1]
private:
    void clearData();

    QList<QSurfaceDataArray> m_data;
    int m_index;
    QSurfaceDataArray m_resetArray;
};

#endif
