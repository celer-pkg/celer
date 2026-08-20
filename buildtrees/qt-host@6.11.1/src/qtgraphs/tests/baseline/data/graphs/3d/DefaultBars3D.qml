import QtGraphs
import QtQuick
import QtQuick3D

Rectangle {
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    ListModel {
        id: dataModel
        ListElement{ timestamp: "2020-01"; expenses: "-14"; income: "22"  }
        ListElement{ timestamp: "2020-02"; expenses: "-5";  income: "7"  }
        ListElement{ timestamp: "2020-03"; expenses: "-1";  income: "9"  }
        ListElement{ timestamp: "2020-04"; expenses: "-1";  income: "12"  }
        ListElement{ timestamp: "2020-05"; expenses: "-5";  income: "9"  }
        ListElement{ timestamp: "2020-06"; expenses: "-5";  income: "8"  }
        ListElement{ timestamp: "2020-07"; expenses: "-3";  income: "7"  }
        ListElement{ timestamp: "2020-08"; expenses: "-1";  income: "5"  }
        ListElement{ timestamp: "2020-09"; expenses: "-2";  income: "4"  }
        ListElement{ timestamp: "2020-10"; expenses: "-10"; income: "13"  }
        ListElement{ timestamp: "2020-11"; expenses: "-12"; income: "17"  }
        ListElement{ timestamp: "2020-12"; expenses: "-6";  income: "9"  }

        ListElement{ timestamp: "2021-01"; expenses: "-2";  income: "6"  }
        ListElement{ timestamp: "2021-02"; expenses: "-4";  income: "8"  }
        ListElement{ timestamp: "2021-03"; expenses: "-7";  income: "12"  }
        ListElement{ timestamp: "2021-04"; expenses: "-9";  income: "15"  }
        ListElement{ timestamp: "2021-05"; expenses: "-7";  income: "19"  }
        ListElement{ timestamp: "2021-06"; expenses: "-9";  income: "18"  }
        ListElement{ timestamp: "2021-07"; expenses: "-13"; income: "17"  }
        ListElement{ timestamp: "2021-08"; expenses: "-5";  income: "9"  }
        ListElement{ timestamp: "2021-09"; expenses: "-3";  income: "8"  }
        ListElement{ timestamp: "2021-10"; expenses: "-13"; income: "15"  }
        ListElement{ timestamp: "2021-11"; expenses: "-8";  income: "17"  }
        ListElement{ timestamp: "2021-12"; expenses: "-7";  income: "10"  }

        ListElement{ timestamp: "2022-01"; expenses: "-12";  income: "16"  }
        ListElement{ timestamp: "2022-02"; expenses: "-24";  income: "28"  }
        ListElement{ timestamp: "2022-03"; expenses: "-27";  income: "22"  }
        ListElement{ timestamp: "2022-04"; expenses: "-29";  income: "25"  }
        ListElement{ timestamp: "2022-05"; expenses: "-27";  income: "29"  }
        ListElement{ timestamp: "2022-06"; expenses: "-19";  income: "18"  }
        ListElement{ timestamp: "2022-07"; expenses: "-13";  income: "17"  }
        ListElement{ timestamp: "2022-08"; expenses: "-15";  income: "19"  }
        ListElement{ timestamp: "2022-09"; expenses: "-3";   income: "8"  }
        ListElement{ timestamp: "2022-10"; expenses: "-3";   income: "6"  }
        ListElement{ timestamp: "2022-11"; expenses: "-4";   income: "8"  }
        ListElement{ timestamp: "2022-12"; expenses: "-5";   income: "9"  }
    }

    Bars3D {
        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.MixSeries
        }

        anchors.fill: parent
        cameraPreset: Graphs3D.CameraPreset.Front
        environment: SceneEnvironment {}

        Bar3DSeries {
            ItemModelBarDataProxy {
                itemModel: dataModel
                rowRole: "timestamp"
                columnRole: "timestamp"
                valueRole: "income"
                rowRolePattern: /^(\d\d\d\d).*$/
                columnRolePattern: /^.*-(\d\d)$/
                rowRoleReplace: "\\1"
                columnRoleReplace: "\\1"
                multiMatchBehavior: ItemModelBarDataProxy.MultiMatchBehavior.Cumulative
            }
        }
    }
}
