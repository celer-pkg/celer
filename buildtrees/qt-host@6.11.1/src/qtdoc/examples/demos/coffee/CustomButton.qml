// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

CustomButtonForm {
    rectangle.gradient: if (buttonColor == "grey"
                                && Colors.currentTheme == Colors.dark) {
                            Colors.darkButtonGradient
                        } else if (buttonColor == "grey"
                                   && Colors.currentTheme == Colors.light) {
                            Colors.lightButtonGradient
                        } else {
                            Colors.greenButtonGradient
                        }
}
