// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QSharedDataPointer>

//! [0]
class EmployeeData;
//! [0]

class EmployeeData
{
public:
    virtual EmployeeData *clone() const;
};

//! [1]
template<>
EmployeeData *QSharedDataPointer<EmployeeData>::clone()
{
    return d->clone();
}
//! [1]
