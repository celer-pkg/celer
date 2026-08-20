// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtQml.StateMachine

TestCase {
    StateMachine {
        State {
          id: stateId
        }
        initialState: stateId
    }
    name: "anonymousState"
    // no real tests, just make sure it runs
}
