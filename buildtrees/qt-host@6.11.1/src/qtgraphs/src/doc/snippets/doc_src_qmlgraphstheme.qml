// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtGraphs
import QtQuick
import QtQuick.Window

Window {
    height: 512
    width: 512

    //! [scatter]
    Scatter3D {
        theme: GraphsTheme { theme: GraphsTheme.Theme.YellowSeries }
        //! [scatter]
    }

    //! [bars]
    Bars3D {
        theme: GraphsTheme {
            theme: GraphsTheme.Theme.QtGreenNeon
            labelBorderVisible: true
            labelFont.pointSize: 35
            labelBackgroundVisible: false
        }
        //! [bars]
    }

    //! [surface]
    Surface3D {
        theme: GraphsTheme {
            theme: GraphsTheme.Theme.UserDefined
            backgroundColor: "red"
            backgroundVisible: true
            seriesColors: ["blue"]
            colorStyle: GraphsTheme.ColorStyle.Uniform
            labelFont.family: "Lucida Handwriting"
            labelFont.pointSize: 35
            gridVisible: false
            grid.mainColor: "red"
            grid.subColor: "blue"
            labelBackgroundColor: "black"
            labelBackgroundVisible: true
            labelBorderVisible: false
            labelTextColor: "white"
            multiHighlightColor: "green"
            singleHighlightColor: "darkRed"
        }
        //! [surface]
    }
}
