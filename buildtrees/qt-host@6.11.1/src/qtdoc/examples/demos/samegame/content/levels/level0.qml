// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

TemplateBase{
    moveTarget: 3
    goalText: "1 of 10<br><br>Clear in three moves..."
    startingGrid: [ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
                    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
                    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
                    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
                    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
                    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
                    0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 ,
                    0 , 0 , 0 , 0 , 0 , 1 , 1 , 2 , 1 , 1 ,
                    0 , 0 , 0 , 1 , 1 , 3 , 3 , 3 , 3 , 3 ,
                    0 , 1 , 1 , 3 , 3 , 3 , 1 , 3 , 1 , 1 ,
                    1 , 2 , 3 , 3 , 1 , 1 , 3 , 3 , 3 , 3 ,
                    1 , 3 , 3 , 2 , 3 , 3 , 3 , 3 , 1 , 1 ,
                    1 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 ]
}
