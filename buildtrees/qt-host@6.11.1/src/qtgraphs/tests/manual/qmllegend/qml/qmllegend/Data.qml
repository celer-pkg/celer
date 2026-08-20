// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    property alias model: dataModel

    ListModel {
        id: dataModel
        ListElement{ year: "2010"; month: "Jan"; s1: "-14"; s2: "-15"; s3: "-15" }
        ListElement{ year: "2010"; month: "Feb"; s1: "-15"; s2: "-16"; s3: "-9" }
        ListElement{ year: "2010"; month: "Mar"; s1: "-7";  s2: "-4";  s3: "-2" }
        ListElement{ year: "2010"; month: "Apr"; s1: "3";   s2: "2";   s3: "2" }
        ListElement{ year: "2010"; month: "May"; s1: "7";   s2: "9";   s3: "10" }
        ListElement{ year: "2010"; month: "Jun"; s1: "12";  s2: "13";  s3: "22" }
        ListElement{ year: "2010"; month: "Jul"; s1: "18";  s2: "19";  s3: "24" }
        ListElement{ year: "2010"; month: "Aug"; s1: "15";  s2: "13";  s3: "16" }
        ListElement{ year: "2010"; month: "Sep"; s1: "6";   s2: "3";   s3: "4" }
        ListElement{ year: "2010"; month: "Oct"; s1: "1";   s2: "2";   s3: "-2" }
        ListElement{ year: "2010"; month: "Nov"; s1: "-2";  s2: "-5";  s3: "-6" }
        ListElement{ year: "2010"; month: "Dec"; s1: "-3";  s2: "-3";  s3: "-9" }

        ListElement{ year: "2011"; month: "Jan"; s1: "-12"; s2: "-11"; s3: "-14" }
        ListElement{ year: "2011"; month: "Feb"; s1: "-13"; s2: "-12"; s3: "-10" }
        ListElement{ year: "2011"; month: "Mar"; s1: "-6";  s2: "-4";  s3: "-3" }
        ListElement{ year: "2011"; month: "Apr"; s1: "0";   s2: "1";   s3: "3" }
        ListElement{ year: "2011"; month: "May"; s1: "4";   s2: "12";  s3: "11" }
        ListElement{ year: "2011"; month: "Jun"; s1: "9";   s2: "17";  s3: "23" }
        ListElement{ year: "2011"; month: "Jul"; s1: "15";  s2: "22";  s3: "25" }
        ListElement{ year: "2011"; month: "Aug"; s1: "12";  s2: "15";  s3: "12" }
        ListElement{ year: "2011"; month: "Sep"; s1: "2";   s2: "4";   s3: "7" }
        ListElement{ year: "2011"; month: "Oct"; s1: "-2";  s2: "4";   s3: "-4" }
        ListElement{ year: "2011"; month: "Nov"; s1: "-4";  s2: "-8";  s3: "-5" }
        ListElement{ year: "2011"; month: "Dec"; s1: "-6";  s2: "-6";  s3: "-7" }

        ListElement{ year: "2012"; month: "Jan"; s1: "-10"; s2: "-19"; s3: "-11" }
        ListElement{ year: "2012"; month: "Feb"; s1: "-11"; s2: "-17"; s3: "-4" }
        ListElement{ year: "2012"; month: "Mar"; s1: "-6";  s2: "-3";  s3: "-1" }
        ListElement{ year: "2012"; month: "Apr"; s1: "5";   s2: "1";   s3: "2" }
        ListElement{ year: "2012"; month: "May"; s1: "9";   s2: "12";  s3: "13" }
        ListElement{ year: "2012"; month: "Jun"; s1: "11";  s2: "16";  s3: "26" }
        ListElement{ year: "2012"; month: "Jul"; s1: "18";  s2: "20";  s3: "23" }
        ListElement{ year: "2012"; month: "Aug"; s1: "19";  s2: "12";  s3: "12" }
        ListElement{ year: "2012"; month: "Sep"; s1: "9";   s2: "1";   s3: "3" }
        ListElement{ year: "2012"; month: "Oct"; s1: "-3";  s2: "2";   s3: "-1" }
        ListElement{ year: "2012"; month: "Nov"; s1: "-5";  s2: "-4";  s3: "-3" }
        ListElement{ year: "2012"; month: "Dec"; s1: "-7";  s2: "-2";  s3: "-4" }

        ListElement{ year: "2013"; month: "Jan"; s1: "-18"; s2: "-19"; s3: "-19" }
        ListElement{ year: "2013"; month: "Feb"; s1: "-17"; s2: "-19"; s3: "-12" }
        ListElement{ year: "2013"; month: "Mar"; s1: "-9";  s2: "-6";  s3: "-5" }
        ListElement{ year: "2013"; month: "Apr"; s1: "0";   s2: "0";   s3: "0" }
        ListElement{ year: "2013"; month: "May"; s1: "4";   s2: "7";   s3: "9" }
        ListElement{ year: "2013"; month: "Jun"; s1: "9";   s2: "11";  s3: "18" }
        ListElement{ year: "2013"; month: "Jul"; s1: "13";  s2: "15";  s3: "20" }
        ListElement{ year: "2013"; month: "Aug"; s1: "10";  s2: "11";  s3: "13" }
        ListElement{ year: "2013"; month: "Sep"; s1: "3";   s2: "1";   s3: "2" }
        ListElement{ year: "2013"; month: "Oct"; s1: "0";   s2: "1";   s3: "-4" }
        ListElement{ year: "2013"; month: "Nov"; s1: "-5";  s2: "-6";  s3: "-5" }
        ListElement{ year: "2013"; month: "Dec"; s1: "-6";  s2: "-7";  s3: "-10" }
    }
}
