// Copyright (C) 2012 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Text {
    font.pixelSize: Settings.fontPixelSize;
    color: "white";
    textFormat: Text.StyledText;
    Behavior on opacity { NumberAnimation {} }
}
