// Copyright (C) 2023 The Qt Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQml.StateMachine

SignalTransition {
    // Do not crash on SignalTransition without signal
    onTriggered: () => {}
}
