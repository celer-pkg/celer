// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QSharedData>
#include <QString>

class EmployeeData : public QSharedData
{
public:
    EmployeeData();
    EmployeeData(const EmployeeData &other);
    ~EmployeeData();

    int id;
    QString *name;
};

class Employee
{
public:
    Employee();
    Employee(int id, const QString &name);

    void setId(int id) { d->id = id; }
    void setName(const QString &name);

    int id() const { return d->id; }
    QString name() const;

private:
    QExplicitlySharedDataPointer<EmployeeData> d;
};

#endif
