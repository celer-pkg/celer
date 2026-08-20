// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// qmllint disable missing-property

pragma Singleton
import QtQml

QtObject {
    readonly property QtObject controls: Qt.styleHints.colorScheme === Qt.Light ? light.controls : dark.controls

    readonly property QtObject dark: QtObject {
        readonly property QtObject controls: QtObject {
            readonly property QtObject button: QtObject {
                readonly property QtObject checked: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17023;2356:10516;2373:10903"
                        readonly property string filePath: "dark/images/button-background-checked.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-checked"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 2045
                        readonly property real y: 6128
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:17023;2356:10516"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-checked"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17023;2356:10516;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-checked"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 2063
                        readonly property real y: 6146
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17023;2356:10516;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-checked"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 2087
                        readonly property real y: 6133
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17029;2356:10522;2373:10903"
                        readonly property string filePath: "dark/images/button-background-checked-disabled.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-checked-disabled"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 2045
                        readonly property real y: 6329
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:17029;2356:10522"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-checked-disabled"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17029;2356:10522;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 2063
                        readonly property real y: 6347
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17029;2356:10522;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 2087
                        readonly property real y: 6334
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17027;2356:10520;2373:10903"
                        readonly property string filePath: "dark/images/button-background-checked-hovered.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-checked-hovered"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 2045
                        readonly property real y: 6262
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:17027;2356:10520"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-checked-hovered"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17027;2356:10520;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 2063
                        readonly property real y: 6280
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17027;2356:10520;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 2087
                        readonly property real y: 6267
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17031;2356:10524;2373:10903"
                        readonly property string filePath: "dark/images/button-background-checked-pressed.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-checked-pressed"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 2045
                        readonly property real y: 6396
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:17031;2356:10524"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-checked-pressed"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17031;2356:10524;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 2063
                        readonly property real y: 6414
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17031;2356:10524;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 2087
                        readonly property real y: 6401
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17025;2356:10518;2373:10903"
                        readonly property string filePath: "dark/images/button-background-disabled.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-disabled"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 2045
                        readonly property real y: 6195
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:17025;2356:10518"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-disabled"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17025;2356:10518;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 2063
                        readonly property real y: 6213
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17025;2356:10518;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 2087
                        readonly property real y: 6200
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 6
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17019;2356:10512;2373:10903"
                        readonly property string filePath: "dark/images/button-background-hovered.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 2
                        readonly property string name: "button-background-hovered"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 2
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 2045
                        readonly property real y: 5994
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:17019;2356:10512"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-hovered"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17019;2356:10512;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 2063
                        readonly property real y: 6012
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17019;2356:10512;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 2087
                        readonly property real y: 5999
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject normal: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 4
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17017;2356:10510;2373:10903"
                        readonly property string filePath: "dark/images/button-background.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 2
                        readonly property string name: "button-background"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 2
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 2066.5
                        readonly property real y: 5924
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:17017;2356:10510"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17017;2356:10510;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 2084.5
                        readonly property real y: 5942
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17017;2356:10510;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 2108.5
                        readonly property real y: 5931
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17021;2356:10514;2373:10903"
                        readonly property string filePath: "dark/images/button-background-pressed.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-pressed"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 2045
                        readonly property real y: 6061
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:17021;2356:10514"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-pressed"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17021;2356:10514;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 2063
                        readonly property real y: 6079
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17021;2356:10514;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 2087
                        readonly property real y: 6066
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

            }

            readonly property QtObject flatbutton: QtObject {
                readonly property QtObject checked: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9227;3987:9104;3987:9044"
                        readonly property string filePath: "dark/images/flatbutton-background-checked.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-checked"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3301.5
                        readonly property real y: 6196.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9227;3987:9104"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-checked"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9227;3987:9104;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-checked"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3319.5
                        readonly property real y: 6214.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9227;3987:9104;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-checked"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3355.5
                        readonly property real y: 6204.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9230;3987:9122;3987:9044"
                        readonly property string filePath: "dark/images/flatbutton-background-checked-disabled.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-checked-disabled"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3301.5
                        readonly property real y: 6330.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9230;3987:9122"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-checked-disabled"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9230;3987:9122;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3319.5
                        readonly property real y: 6348.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9230;3987:9122;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3355.5
                        readonly property real y: 6338.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9229;3987:9113;3987:9044"
                        readonly property string filePath: "dark/images/flatbutton-background-checked-hovered.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-checked-hovered"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3301.5
                        readonly property real y: 6263.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9229;3987:9113"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-checked-hovered"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9229;3987:9113;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3319.5
                        readonly property real y: 6281.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9229;3987:9113;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3355.5
                        readonly property real y: 6271.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9231;3987:9131;3987:9044"
                        readonly property string filePath: "dark/images/flatbutton-background-checked-pressed.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-checked-pressed"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3301.5
                        readonly property real y: 6397.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9231;3987:9131"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-checked-pressed"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9231;3987:9131;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3319.5
                        readonly property real y: 6415.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9231;3987:9131;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3355.5
                        readonly property real y: 6405.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9228;3987:9095;3987:9044"
                        readonly property string filePath: "dark/images/flatbutton-background-disabled.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-disabled"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3301.5
                        readonly property real y: 6129.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9228;3987:9095"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-disabled"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9228;3987:9095;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3319.5
                        readonly property real y: 6147.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9228;3987:9095;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3355.5
                        readonly property real y: 6137.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 6
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9225;3987:9077;3987:9044"
                        readonly property string filePath: "dark/images/flatbutton-background-hovered.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 2
                        readonly property string name: "flatbutton-background-hovered"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 2
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3301.5
                        readonly property real y: 5995.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9225;3987:9077"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-hovered"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9225;3987:9077;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3319.5
                        readonly property real y: 6013.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9225;3987:9077;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3355.5
                        readonly property real y: 6003.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject normal: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 4
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9224;3987:9068;3987:9044"
                        readonly property string filePath: "dark/images/flatbutton-background.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 2
                        readonly property string name: "flatbutton-background"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 2
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3301.5
                        readonly property real y: 5928.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9224;3987:9068"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9224;3987:9068;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3319.5
                        readonly property real y: 5946.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9224;3987:9068;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3355.5
                        readonly property real y: 5936.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9226;3987:9086;3987:9044"
                        readonly property string filePath: "dark/images/flatbutton-background-pressed.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-pressed"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3301.5
                        readonly property real y: 6062.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9226;3987:9086"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-pressed"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9226;3987:9086;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3319.5
                        readonly property real y: 6080.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9226;3987:9086;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3355.5
                        readonly property real y: 6070.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

            }

            readonly property QtObject roundbutton: QtObject {
                readonly property QtObject checked: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17167;2513:14093;2513:14075"
                        readonly property string filePath: "dark/images/roundbutton-background-checked.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-checked"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3773
                        readonly property real y: 6884.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:17167;2513:14093"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-checked"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17167;2513:14093;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-checked"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3793
                        readonly property real y: 6904.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17167;2513:14093;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-checked"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3827
                        readonly property real y: 6894.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject checked_disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17165;2513:14108;2513:14075"
                        readonly property string filePath: "dark/images/roundbutton-background-checked-disabled.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-checked-disabled"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3773
                        readonly property real y: 7085.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:17165;2513:14108"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-checked-disabled"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17165;2513:14108;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3793
                        readonly property real y: 7105.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17165;2513:14108;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3827
                        readonly property real y: 7095.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject checked_hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17161;2513:14103;2513:14075"
                        readonly property string filePath: "dark/images/roundbutton-background-checked-hovered.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-checked-hovered"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3773
                        readonly property real y: 7018.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:17161;2513:14103"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-checked-hovered"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17161;2513:14103;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3793
                        readonly property real y: 7038.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17161;2513:14103;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3827
                        readonly property real y: 7028.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject checked_pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17163;2513:14113;2513:14075"
                        readonly property string filePath: "dark/images/roundbutton-background-checked-pressed.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-checked-pressed"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3773
                        readonly property real y: 7152.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:17163;2513:14113"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-checked-pressed"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17163;2513:14113;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3793
                        readonly property real y: 7172.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17163;2513:14113;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3827
                        readonly property real y: 7162.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17169;2513:14098;2513:14075"
                        readonly property string filePath: "dark/images/roundbutton-background-disabled.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-disabled"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3773
                        readonly property real y: 6951.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:17169;2513:14098"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-disabled"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17169;2513:14098;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3793
                        readonly property real y: 6971.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17169;2513:14098;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3827
                        readonly property real y: 6961.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 6
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17157;2513:14083;2513:14075"
                        readonly property string filePath: "dark/images/roundbutton-background-hovered.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 2
                        readonly property string name: "roundbutton-background-hovered"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3773
                        readonly property real y: 6750.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:17157;2513:14083"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-hovered"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17157;2513:14083;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3793
                        readonly property real y: 6770.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17157;2513:14083;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3827
                        readonly property real y: 6760.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject normal: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 4
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17155;2513:14078;2513:14075"
                        readonly property string filePath: "dark/images/roundbutton-background.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 2
                        readonly property string name: "roundbutton-background"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3773
                        readonly property real y: 6683.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:17155;2513:14078"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17155;2513:14078;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3793
                        readonly property real y: 6703.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17155;2513:14078;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3827
                        readonly property real y: 6693.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:17159;2513:14088;2513:14075"
                        readonly property string filePath: "dark/images/roundbutton-background-pressed.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-pressed"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3773
                        readonly property real y: 6817.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:17159;2513:14088"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-pressed"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17159;2513:14088;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3793
                        readonly property real y: 6837.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:17159;2513:14088;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3827
                        readonly property real y: 6827.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

            }

        }
    }
    readonly property QtObject light: QtObject {
        readonly property QtObject controls: QtObject {
            readonly property QtObject button: QtObject {
                readonly property QtObject checked: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15399;2356:10516;2373:10903"
                        readonly property string filePath: "light/images/button-background-checked.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-checked"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 1902
                        readonly property real y: 6128
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:15399;2356:10516"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-checked"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15399;2356:10516;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-checked"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 1920
                        readonly property real y: 6146
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15399;2356:10516;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-checked"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 1944
                        readonly property real y: 6133
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15405;2356:10522;2373:10903"
                        readonly property string filePath: "light/images/button-background-checked-disabled.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-checked-disabled"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 1902
                        readonly property real y: 6329
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:15405;2356:10522"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-checked-disabled"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15405;2356:10522;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 1920
                        readonly property real y: 6347
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15405;2356:10522;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 1944
                        readonly property real y: 6334
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15403;2356:10520;2373:10903"
                        readonly property string filePath: "light/images/button-background-checked-hovered.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-checked-hovered"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 1902
                        readonly property real y: 6262
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:15403;2356:10520"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-checked-hovered"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15403;2356:10520;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 1920
                        readonly property real y: 6280
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15403;2356:10520;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 1944
                        readonly property real y: 6267
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15407;2356:10524;2373:10903"
                        readonly property string filePath: "light/images/button-background-checked-pressed.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-checked-pressed"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 1902
                        readonly property real y: 6396
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:15407;2356:10524"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-checked-pressed"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15407;2356:10524;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 1920
                        readonly property real y: 6414
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15407;2356:10524;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 1944
                        readonly property real y: 6401
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15401;2356:10518;2373:10903"
                        readonly property string filePath: "light/images/button-background-disabled.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-disabled"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 1902
                        readonly property real y: 6195
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:15401;2356:10518"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-disabled"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15401;2356:10518;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 1920
                        readonly property real y: 6213
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15401;2356:10518;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 1944
                        readonly property real y: 6200
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 6
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15395;2356:10512;2373:10903"
                        readonly property string filePath: "light/images/button-background-hovered.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 2
                        readonly property string name: "button-background-hovered"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 2
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 1902
                        readonly property real y: 5994
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:15395;2356:10512"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-hovered"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15395;2356:10512;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 1920
                        readonly property real y: 6012
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15395;2356:10512;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 1944
                        readonly property real y: 5999
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject normal: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 4
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15393;2356:10510;2373:10903"
                        readonly property string filePath: "light/images/button-background.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 2
                        readonly property string name: "button-background"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 2
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 1923.5
                        readonly property real y: 5924
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:15393;2356:10510"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15393;2356:10510;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 1941.5
                        readonly property real y: 5942
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15393;2356:10510;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 1965.5
                        readonly property real y: 5931
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

                readonly property QtObject pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15397;2356:10514;2373:10903"
                        readonly property string filePath: "light/images/button-background-pressed.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "button-background-pressed"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 1902
                        readonly property real y: 6061
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I2557:15397;2356:10514"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "button-contentItem-pressed"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15397;2356:10514;4693:13271"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-icon-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 1920
                        readonly property real y: 6079
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15397;2356:10514;2248:10452"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "button-label-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 1944
                        readonly property real y: 6066
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 0
                    readonly property real topPadding: 18
                }

            }

            readonly property QtObject flatbutton: QtObject {
                readonly property QtObject checked: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9165;3987:9104;3987:9044"
                        readonly property string filePath: "light/images/flatbutton-background-checked.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-checked"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3158.5
                        readonly property real y: 6196.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9165;3987:9104"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-checked"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9165;3987:9104;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-checked"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3176.5
                        readonly property real y: 6214.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9165;3987:9104;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-checked"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3212.5
                        readonly property real y: 6204.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9168;3987:9122;3987:9044"
                        readonly property string filePath: "light/images/flatbutton-background-checked-disabled.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-checked-disabled"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3158.5
                        readonly property real y: 6330.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9168;3987:9122"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-checked-disabled"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9168;3987:9122;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3176.5
                        readonly property real y: 6348.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9168;3987:9122;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3212.5
                        readonly property real y: 6338.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9167;3987:9113;3987:9044"
                        readonly property string filePath: "light/images/flatbutton-background-checked-hovered.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-checked-hovered"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3158.5
                        readonly property real y: 6263.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9167;3987:9113"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-checked-hovered"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9167;3987:9113;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3176.5
                        readonly property real y: 6281.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9167;3987:9113;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3212.5
                        readonly property real y: 6271.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject checked_pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9169;3987:9131;3987:9044"
                        readonly property string filePath: "light/images/flatbutton-background-checked-pressed.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-checked-pressed"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3158.5
                        readonly property real y: 6397.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9169;3987:9131"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-checked-pressed"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9169;3987:9131;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3176.5
                        readonly property real y: 6415.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9169;3987:9131;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3212.5
                        readonly property real y: 6405.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9166;3987:9095;3987:9044"
                        readonly property string filePath: "light/images/flatbutton-background-disabled.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-disabled"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3158.5
                        readonly property real y: 6129.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9166;3987:9095"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-disabled"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9166;3987:9095;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3176.5
                        readonly property real y: 6147.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9166;3987:9095;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3212.5
                        readonly property real y: 6137.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 6
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9163;3987:9077;3987:9044"
                        readonly property string filePath: "light/images/flatbutton-background-hovered.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 2
                        readonly property string name: "flatbutton-background-hovered"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 2
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3158.5
                        readonly property real y: 5995.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9163;3987:9077"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-hovered"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9163;3987:9077;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3176.5
                        readonly property real y: 6013.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9163;3987:9077;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3212.5
                        readonly property real y: 6003.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject normal: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 4
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9162;3987:9068;3987:9044"
                        readonly property string filePath: "light/images/flatbutton-background.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 2
                        readonly property string name: "flatbutton-background"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 2
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3158.5
                        readonly property real y: 5928.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9162;3987:9068"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9162;3987:9068;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3176.5
                        readonly property real y: 5946.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9162;3987:9068;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3212.5
                        readonly property real y: 5936.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

                readonly property QtObject pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 25
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I3991:9164;3987:9086;3987:9044"
                        readonly property string filePath: "light/images/flatbutton-background-pressed.png"
                        readonly property real height: 60
                        readonly property real leftOffset: 25
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-background-pressed"
                        readonly property real rightOffset: 25
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 25
                        readonly property real topShadow: 0
                        readonly property real width: 60
                        readonly property real x: 3158.5
                        readonly property real y: 6062.5
                    }

                    readonly property real bottomPadding: 18
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 18
                        readonly property string figmaId: "I3991:9164;3987:9086"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 18
                        readonly property string name: "flatbutton-contentItem-pressed"
                        readonly property real rightPadding: 18
                        readonly property real spacing: 10
                        readonly property real topPadding: 18
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9164;3987:9086;4709:15937"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-icon-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3176.5
                        readonly property real y: 6080.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I3991:9164;3987:9086;3987:9039"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "flatbutton-label-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3212.5
                        readonly property real y: 6070.5
                    }

                    readonly property real leftPadding: 18
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 18
                    readonly property real spacing: 12
                    readonly property real topPadding: 18
                }

            }

            readonly property QtObject roundbutton: QtObject {
                readonly property QtObject checked: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15543;2513:14093;2513:14075"
                        readonly property string filePath: "light/images/roundbutton-background-checked.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-checked"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3679
                        readonly property real y: 6884.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:15543;2513:14093"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-checked"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15543;2513:14093;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-checked"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3699
                        readonly property real y: 6904.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15543;2513:14093;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-checked"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3733
                        readonly property real y: 6894.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject checked_disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15541;2513:14108;2513:14075"
                        readonly property string filePath: "light/images/roundbutton-background-checked-disabled.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-checked-disabled"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3679
                        readonly property real y: 7085.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:15541;2513:14108"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-checked-disabled"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15541;2513:14108;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3699
                        readonly property real y: 7105.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15541;2513:14108;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-checked-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3733
                        readonly property real y: 7095.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject checked_hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15537;2513:14103;2513:14075"
                        readonly property string filePath: "light/images/roundbutton-background-checked-hovered.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-checked-hovered"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3679
                        readonly property real y: 7018.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:15537;2513:14103"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-checked-hovered"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15537;2513:14103;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3699
                        readonly property real y: 7038.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15537;2513:14103;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-checked-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3733
                        readonly property real y: 7028.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject checked_pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15539;2513:14113;2513:14075"
                        readonly property string filePath: "light/images/roundbutton-background-checked-pressed.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-checked-pressed"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3679
                        readonly property real y: 7152.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:15539;2513:14113"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-checked-pressed"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15539;2513:14113;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3699
                        readonly property real y: 7172.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15539;2513:14113;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-checked-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3733
                        readonly property real y: 7162.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject disabled: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15545;2513:14098;2513:14075"
                        readonly property string filePath: "light/images/roundbutton-background-disabled.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-disabled"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3679
                        readonly property real y: 6951.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:15545;2513:14098"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-disabled"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15545;2513:14098;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-disabled"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3699
                        readonly property real y: 6971.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15545;2513:14098;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-disabled"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3733
                        readonly property real y: 6961.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject hovered: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 6
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15533;2513:14083;2513:14075"
                        readonly property string filePath: "light/images/roundbutton-background-hovered.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 2
                        readonly property string name: "roundbutton-background-hovered"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3679
                        readonly property real y: 6750.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:15533;2513:14083"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-hovered"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15533;2513:14083;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-hovered"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3699
                        readonly property real y: 6770.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15533;2513:14083;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-hovered"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3733
                        readonly property real y: 6760.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject normal: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 4
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15531;2513:14078;2513:14075"
                        readonly property string filePath: "light/images/roundbutton-background.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 2
                        readonly property string name: "roundbutton-background"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3679
                        readonly property real y: 6683.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:15531;2513:14078"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15531;2513:14078;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3699
                        readonly property real y: 6703.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15531;2513:14078;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3733
                        readonly property real y: 6693.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

                readonly property QtObject pressed: QtObject {
                    readonly property QtObject background: QtObject {
                        readonly property real bottomOffset: 31
                        readonly property real bottomShadow: 0
                        readonly property string exportType: "image"
                        readonly property string figmaId: "I2557:15535;2513:14088;2513:14075"
                        readonly property string filePath: "light/images/roundbutton-background-pressed.png"
                        readonly property real height: 64
                        readonly property real leftOffset: 32
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-background-pressed"
                        readonly property real rightOffset: 31
                        readonly property real rightShadow: 0
                        readonly property real topOffset: 32
                        readonly property real topShadow: 0
                        readonly property real width: 64
                        readonly property real x: 3679
                        readonly property real y: 6817.5
                    }

                    readonly property real bottomPadding: 20
                    readonly property QtObject contentItem: QtObject {
                        readonly property string alignItems: "CENTER"
                        readonly property real bottomPadding: 20
                        readonly property string figmaId: "I2557:15535;2513:14088"
                        readonly property string layoutMode: "HORIZONTAL"
                        readonly property real leftPadding: 20
                        readonly property string name: "roundbutton-contentItem-pressed"
                        readonly property real rightPadding: 20
                        readonly property real spacing: 10
                        readonly property real topPadding: 20
                    }

                    readonly property QtObject icon: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15535;2513:14088;4709:13874"
                        readonly property real height: 24
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-icon-pressed"
                        readonly property real rightShadow: 0
                        readonly property real topShadow: 0
                        readonly property real width: 24
                        readonly property real x: 3699
                        readonly property real y: 6837.5
                    }

                    readonly property QtObject label: QtObject {
                        readonly property real bottomShadow: 0
                        readonly property string figmaId: "I2557:15535;2513:14088;2513:14073"
                        readonly property string fontFamily: "Titillium Web"
                        readonly property real fontSize: 16
                        readonly property real height: 20
                        readonly property real leftShadow: 0
                        readonly property string name: "roundbutton-label-pressed"
                        readonly property real rightShadow: 0
                        readonly property real textHAlignment: 4
                        readonly property real textVAlignment: 128
                        readonly property real topShadow: 0
                        readonly property real width: 36
                        readonly property real x: 3733
                        readonly property real y: 6827.5
                    }

                    readonly property real leftPadding: 20
                    readonly property bool mirrored: false
                    readonly property real rightPadding: 20
                    readonly property real spacing: 10
                    readonly property real topPadding: 20
                }

            }

        }
    }
}
