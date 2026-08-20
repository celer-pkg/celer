// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QResource>
#include <QFile>

void wrap()
{
    //! [4]
    QResource::registerResource("/path/to/myresource.rcc");
    //! [4]
}

class BaseClass {
    public:
    BaseClass() {}
};

class MyClass : BaseClass {
    public:
    MyClass();
};

//! [5]
MyClass::MyClass() : BaseClass()
{
    Q_INIT_RESOURCE(resources);

    QFile file(":/myfile.dat");
    //...
}
//! [5]
