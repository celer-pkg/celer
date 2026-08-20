// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QPushButton>

//![open]
int main()
{
//![open]

    {
        //![example1]
            QWidget window;
            QPushButton quit("Quit", &window);
            //...
        //![example1]
    }

    {
        //![example2]
            QPushButton quit("Quit");
            QWidget window;

            quit.setParent(&window);
            //...
        //![example2]
    }

//![close]
}
//![close]
