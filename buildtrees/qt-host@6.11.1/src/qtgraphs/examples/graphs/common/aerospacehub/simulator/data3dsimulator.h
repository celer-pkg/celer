// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DATA3DSIMULATOR_H
#define DATA3DSIMULATOR_H

#include <QtQml/qqmlregistration.h>

#include "datasimulator.h"
#include "simulateddatamodel.h"

class Data3DSimulator : public DataSimulator
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int numberOfRows READ numberOfRows WRITE setNumberOfRows NOTIFY numberOfRowsChanged FINAL)
    Q_PROPERTY(int numberOfColumns READ numberOfColumns WRITE setNumberOfColumns NOTIFY numberOfColumnsChanged FINAL)
    Q_PROPERTY(SimulatedDataModel *dataModel READ dataModel WRITE setDataModel NOTIFY dataModelChanged FINAL)

public:
    explicit Data3DSimulator(QObject *parent = nullptr);

    int numberOfRows() const;
    void setNumberOfRows(int numberOfRows);
    int numberOfColumns() const;
    void setNumberOfColumns(int numberOfColumns);

    SimulatedDataModel *dataModel() const;
    void setDataModel(SimulatedDataModel *dataModel);

    void generateData() override;

Q_SIGNALS:
    void numberOfRowsChanged(int numberOfRows);
    void numberOfColumnsChanged(int numberOfColumns);
    void dataModelChanged(SimulatedDataModel *dataModel);

private:
    int m_numberOfRows = 1;
    int m_numberOfColumns = 1;
    SimulatedDataModel *m_dataModel = nullptr;
};

#endif // DATA3DSIMULATOR_H
