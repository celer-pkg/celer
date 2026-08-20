// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    property alias sharedData: dataModel

    ListModel {
        id: dataModel
        ListElement{ coords: "0,0"; data: "20.0/10.0/4.75"; }
        ListElement{ coords: "1,0"; data: "21.1/10.3/3.00"; }
        ListElement{ coords: "2,0"; data: "22.5/10.7/1.24"; }
        ListElement{ coords: "3,0"; data: "24.0/10.5/2.53"; }
        ListElement{ coords: "0,1"; data: "20.2/11.2/3.55"; }
        ListElement{ coords: "1,1"; data: "21.3/11.5/3.03"; }
        ListElement{ coords: "2,1"; data: "22.6/11.7/3.46"; }
        ListElement{ coords: "3,1"; data: "23.4/11.5/4.12"; }
        ListElement{ coords: "0,2"; data: "20.2/12.3/3.37"; }
        ListElement{ coords: "1,2"; data: "21.1/12.4/2.98"; }
        ListElement{ coords: "2,2"; data: "22.5/12.1/3.33"; }
        ListElement{ coords: "3,2"; data: "23.3/12.7/3.23"; }
        ListElement{ coords: "0,3"; data: "20.7/13.3/5.34"; }
        ListElement{ coords: "1,3"; data: "21.5/13.2/4.54"; }
        ListElement{ coords: "2,3"; data: "22.4/13.6/4.65"; }
        ListElement{ coords: "3,3"; data: "23.2/13.4/6.67"; }
        ListElement{ coords: "0,4"; data: "20.6/15.0/6.01"; }
        ListElement{ coords: "1,4"; data: "21.3/14.6/5.83"; }
        ListElement{ coords: "2,4"; data: "22.5/14.8/7.32"; }
        ListElement{ coords: "3,4"; data: "23.7/14.3/6.90"; }

        ListElement{ coords: "0,0"; data: "40.0/30.0/14.75"; }
        ListElement{ coords: "1,0"; data: "41.1/30.3/13.00"; }
        ListElement{ coords: "2,0"; data: "42.5/30.7/11.24"; }
        ListElement{ coords: "3,0"; data: "44.0/30.5/12.53"; }
        ListElement{ coords: "0,1"; data: "40.2/31.2/13.55"; }
        ListElement{ coords: "1,1"; data: "41.3/31.5/13.03"; }
        ListElement{ coords: "2,1"; data: "42.6/31.7/13.46"; }
        ListElement{ coords: "3,1"; data: "43.4/31.5/14.12"; }
        ListElement{ coords: "0,2"; data: "40.2/32.3/13.37"; }
        ListElement{ coords: "1,2"; data: "41.1/32.4/12.98"; }
        ListElement{ coords: "2,2"; data: "42.5/32.1/13.33"; }
        ListElement{ coords: "3,2"; data: "43.3/32.7/13.23"; }
        ListElement{ coords: "0,3"; data: "40.7/33.3/15.34"; }
        ListElement{ coords: "1,3"; data: "41.5/33.2/14.54"; }
        ListElement{ coords: "2,3"; data: "42.4/33.6/14.65"; }
        ListElement{ coords: "3,3"; data: "43.2/33.4/16.67"; }
        ListElement{ coords: "0,4"; data: "40.6/35.0/16.01"; }
        ListElement{ coords: "1,4"; data: "41.3/34.6/15.83"; }
        ListElement{ coords: "2,4"; data: "42.5/34.8/17.32"; }
        ListElement{ coords: "3,4"; data: "43.7/34.3/16.90"; }
    }
}

