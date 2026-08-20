// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

TemplateBase{
    scoreTarget: 2000
    timeTarget: 60
    moveTarget: 20
    mustClear: false
    goalText: "10 of 10<br><br>Score 2000 in one minute with less than 20 moves!"
    startingGrid: [ 3 , 2 , 3 , 1 , 3 , 3 , 4 , 1 , 3 , 3 ,
                    2 , 3 , 2 , 1 , 1 , 2 , 2 , 2 , 4 , 1 ,
                    2 , 4 , 4 , 4 , 3 , 1 , 4 , 4 , 4 , 1 ,
                    3 , 1 , 3 , 4 , 4 , 2 , 2 , 2 , 2 , 3 ,
                    2 , 1 , 4 , 4 , 3 , 3 , 1 , 1 , 3 , 2 ,
                    3 , 2 , 1 , 4 , 3 , 4 , 1 , 3 , 4 , 2 ,
                    3 , 3 , 1 , 4 , 4 , 4 , 2 , 1 , 2 , 3 ,
                    2 , 3 , 4 , 3 , 4 , 1 , 1 , 3 , 2 , 4 ,
                    4 , 4 , 1 , 2 , 4 , 3 , 2 , 2 , 2 , 4 ,
                    1 , 4 , 2 , 2 , 1 , 1 , 2 , 1 , 1 , 4 ,
                    1 , 4 , 3 , 3 , 3 , 1 , 3 , 4 , 4 , 2 ,
                    3 , 4 , 1 , 1 , 2 , 2 , 2 , 3 , 2 , 1 ,
                    3 , 3 , 4 , 3 , 1 , 1 , 1 , 4 , 4 , 3 ]
}
