// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQml.Models

Item {
    property alias model: dataModel

    property var modelAsJsArray: {
        var arr = []
        for (var i = 0; i < dataModel.count; i++) {
            var row = dataModel.get(i)
            arr.push({
                         timestamp: row.timestamp,
                         expenses: row.expenses,
                         income: row.income
                     })
        }
        return arr
    }

    //! [0]
    ListModel {
        id: dataModel
        ListElement{ timestamp: "2006-01"; expenses: "-4";  income: "5" }
        ListElement{ timestamp: "2006-02"; expenses: "-5";  income: "6" }
        ListElement{ timestamp: "2006-03"; expenses: "-7";  income: "4" }
        //! [0]
        ListElement{ timestamp: "2006-04"; expenses: "-3";  income: "2" }
        ListElement{ timestamp: "2006-05"; expenses: "-4";  income: "1" }
        ListElement{ timestamp: "2006-06"; expenses: "-2";  income: "2" }
        ListElement{ timestamp: "2006-07"; expenses: "-1";  income: "3" }
        ListElement{ timestamp: "2006-08"; expenses: "-5";  income: "1" }
        ListElement{ timestamp: "2006-09"; expenses: "-2";  income: "3" }
        ListElement{ timestamp: "2006-10"; expenses: "-5";  income: "2" }
        ListElement{ timestamp: "2006-11"; expenses: "-8";  income: "5" }
        ListElement{ timestamp: "2006-12"; expenses: "-3";  income: "3" }

        ListElement{ timestamp: "2007-01"; expenses: "-3";  income: "1" }
        ListElement{ timestamp: "2007-02"; expenses: "-4";  income: "2" }
        ListElement{ timestamp: "2007-03"; expenses: "-12"; income: "4" }
        ListElement{ timestamp: "2007-04"; expenses: "-13"; income: "6" }
        ListElement{ timestamp: "2007-05"; expenses: "-14"; income: "11" }
        ListElement{ timestamp: "2007-06"; expenses: "-7";  income: "7" }
        ListElement{ timestamp: "2007-07"; expenses: "-6";  income: "4" }
        ListElement{ timestamp: "2007-08"; expenses: "-4";  income: "15" }
        ListElement{ timestamp: "2007-09"; expenses: "-2";  income: "18" }
        ListElement{ timestamp: "2007-10"; expenses: "-29"; income: "25" }
        ListElement{ timestamp: "2007-11"; expenses: "-23"; income: "29" }
        ListElement{ timestamp: "2007-12"; expenses: "-5";  income: "9" }

        ListElement{ timestamp: "2008-01"; expenses: "-3";  income: "8" }
        ListElement{ timestamp: "2008-02"; expenses: "-8";  income: "14" }
        ListElement{ timestamp: "2008-03"; expenses: "-10"; income: "20" }
        ListElement{ timestamp: "2008-04"; expenses: "-12"; income: "24" }
        ListElement{ timestamp: "2008-05"; expenses: "-10"; income: "19" }
        ListElement{ timestamp: "2008-06"; expenses: "-5";  income: "8" }
        ListElement{ timestamp: "2008-07"; expenses: "-1";  income: "4" }
        ListElement{ timestamp: "2008-08"; expenses: "-7";  income: "12" }
        ListElement{ timestamp: "2008-09"; expenses: "-4";  income: "16" }
        ListElement{ timestamp: "2008-10"; expenses: "-22"; income: "33" }
        ListElement{ timestamp: "2008-11"; expenses: "-16"; income: "25" }
        ListElement{ timestamp: "2008-12"; expenses: "-2";  income: "7" }

        ListElement{ timestamp: "2009-01"; expenses: "-4";  income: "5"  }
        ListElement{ timestamp: "2009-02"; expenses: "-4";  income: "7"  }
        ListElement{ timestamp: "2009-03"; expenses: "-11"; income: "14"  }
        ListElement{ timestamp: "2009-04"; expenses: "-16"; income: "22"  }
        ListElement{ timestamp: "2009-05"; expenses: "-3";  income: "5"  }
        ListElement{ timestamp: "2009-06"; expenses: "-4";  income: "8"  }
        ListElement{ timestamp: "2009-07"; expenses: "-7";  income: "9"  }
        ListElement{ timestamp: "2009-08"; expenses: "-9";  income: "13"  }
        ListElement{ timestamp: "2009-09"; expenses: "-1";  income: "6"  }
        ListElement{ timestamp: "2009-10"; expenses: "-14"; income: "25"  }
        ListElement{ timestamp: "2009-11"; expenses: "-19"; income: "29"  }
        ListElement{ timestamp: "2009-12"; expenses: "-5";  income: "7"  }

        ListElement{ timestamp: "2010-01"; expenses: "-14"; income: "22"  }
        ListElement{ timestamp: "2010-02"; expenses: "-5";  income: "7"  }
        ListElement{ timestamp: "2010-03"; expenses: "-1";  income: "9"  }
        ListElement{ timestamp: "2010-04"; expenses: "-1";  income: "12"  }
        ListElement{ timestamp: "2010-05"; expenses: "-5";  income: "9"  }
        ListElement{ timestamp: "2010-06"; expenses: "-5";  income: "8"  }
        ListElement{ timestamp: "2010-07"; expenses: "-3";  income: "7"  }
        ListElement{ timestamp: "2010-08"; expenses: "-1";  income: "5"  }
        ListElement{ timestamp: "2010-09"; expenses: "-2";  income: "4"  }
        ListElement{ timestamp: "2010-10"; expenses: "-10"; income: "13"  }
        ListElement{ timestamp: "2010-11"; expenses: "-12"; income: "17"  }
        ListElement{ timestamp: "2010-12"; expenses: "-6";  income: "9"  }

        ListElement{ timestamp: "2011-01"; expenses: "-2";  income: "6"  }
        ListElement{ timestamp: "2011-02"; expenses: "-4";  income: "8"  }
        ListElement{ timestamp: "2011-03"; expenses: "-7";  income: "12"  }
        ListElement{ timestamp: "2011-04"; expenses: "-9";  income: "15"  }
        ListElement{ timestamp: "2011-05"; expenses: "-7";  income: "19"  }
        ListElement{ timestamp: "2011-06"; expenses: "-9";  income: "18"  }
        ListElement{ timestamp: "2011-07"; expenses: "-13"; income: "17"  }
        ListElement{ timestamp: "2011-08"; expenses: "-5";  income: "9"  }
        ListElement{ timestamp: "2011-09"; expenses: "-3";  income: "8"  }
        ListElement{ timestamp: "2011-10"; expenses: "-13"; income: "15"  }
        ListElement{ timestamp: "2011-11"; expenses: "-8";  income: "17"  }
        ListElement{ timestamp: "2011-12"; expenses: "-7";  income: "10"  }

        ListElement{ timestamp: "2012-01"; expenses: "-12";  income: "16"  }
        ListElement{ timestamp: "2012-02"; expenses: "-24";  income: "28"  }
        ListElement{ timestamp: "2012-03"; expenses: "-27";  income: "22"  }
        ListElement{ timestamp: "2012-04"; expenses: "-29";  income: "25"  }
        ListElement{ timestamp: "2012-05"; expenses: "-27";  income: "29"  }
        ListElement{ timestamp: "2012-06"; expenses: "-19";  income: "18"  }
        ListElement{ timestamp: "2012-07"; expenses: "-13";  income: "17"  }
        ListElement{ timestamp: "2012-08"; expenses: "-15";  income: "19"  }
        ListElement{ timestamp: "2012-09"; expenses: "-3";   income: "8"  }
        ListElement{ timestamp: "2012-10"; expenses: "-3";   income: "6"  }
        ListElement{ timestamp: "2012-11"; expenses: "-4";   income: "8"  }
        ListElement{ timestamp: "2012-12"; expenses: "-5";   income: "9"  }
    }
}
