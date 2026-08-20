// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

InsertForm {
    required property ApplicationFlow appFlow
    state: Config.mode
    continueButton.onClicked: appFlow.continueButton()
    cancelButton.onClicked: appFlow.cancelButton()
}
