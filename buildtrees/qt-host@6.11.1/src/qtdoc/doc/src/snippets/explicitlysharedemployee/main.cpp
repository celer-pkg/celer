// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "employee.h"

int main()
{
    {
        Employee e1(10, "Albrecht Durer");
        Employee e2 = e1;
        e1.setName("Hans Holbein");
    }
}
