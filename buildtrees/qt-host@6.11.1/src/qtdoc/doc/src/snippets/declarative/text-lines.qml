// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![0]
import QtQuick

Text {
    textFormat: Text.RichText
    wrapMode: Text.WordWrap
    width: 440
    font.pointSize: 12
    text: `<p><u style="color: green;">green with underline</u>
              <span style="text-decoration: underline; text-decoration-color: green;">
                green underline</span></p>
           <p><s style="background-color: lightgrey;">plain strikethrough</s>
              <span style="text-decoration: line-through; text-decoration-color: orange;">
                orange strikethrough</span></p>
           <p><span style="text-decoration: overline;">plain overline</span>
              <span style="text-decoration: overline; text-decoration-color: red;">
                red overline</span></p>`
}
//![0]
