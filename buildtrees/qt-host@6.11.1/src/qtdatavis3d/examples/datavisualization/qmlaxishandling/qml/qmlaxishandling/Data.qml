// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    property alias model: dataModel

    ListModel {
        id: dataModel
        ListElement{ xPos: 2.456103; yPos: 1.0; zPos: 5.0 }
        ListElement{ xPos: 5.687549; yPos: 3.0; zPos: 2.5 }
        ListElement{ xPos: 2.357458; yPos: 4.1; zPos: 1.0 }
        ListElement{ xPos: 4.567458; yPos: 4.75; zPos: 3.9 }
        ListElement{ xPos: 6.885439; yPos: 4.9; zPos: 7.2 }
        ListElement{ xPos: 2.366769; yPos: 13.42; zPos: 3.5 }
        ListElement{ xPos: 7.546457; yPos: 233.1; zPos: 6.9 }
        ListElement{ xPos: 2.475867; yPos: 32.91; zPos: 4.1 }
        ListElement{ xPos: 8.456546; yPos: 153.68; zPos: 9.52 }
        ListElement{ xPos: 3.456348; yPos: 52.96; zPos: 1.6 }
        ListElement{ xPos: 1.536446; yPos: 32.4; zPos: 2.92 }
        ListElement{ xPos: 8.456666; yPos: 114.74; zPos: 8.18 }
        ListElement{ xPos: 5.468486; yPos: 83.1; zPos: 3.8 }
        ListElement{ xPos: 6.546586 ; yPos: 63.66; zPos: 3.58 }
        ListElement{ xPos: 8.567516 ; yPos: 1.82; zPos: 4.64 }
        ListElement{ xPos: 7.678984 ; yPos: 213.18; zPos: 7.22 }
        ListElement{ xPos: 7.457569 ; yPos: 63.06; zPos: 4.3 }
        ListElement{ xPos: 8.456755 ; yPos: 122.64; zPos: 6.44 }
        ListElement{ xPos: 6.234536 ; yPos: 63.96; zPos: 4.38 }
        ListElement{ xPos: 9.456718 ; yPos: 243.32; zPos: 4.04 }
        ListElement{ xPos: 10.789889 ; yPos: 43.4; zPos: 2.78 }
        ListElement{ xPos: 11.346554 ; yPos: 345.12; zPos: 3.1 }
        ListElement{ xPos: 12.023454 ; yPos: 500.0; zPos: 3.68 }
    }
}
