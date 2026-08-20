// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQml

import QtQml.StateMachine

import CppObjectEnum 1.0

StateMachine {
    id: stateMachine
    initialState: state0

    State {
        id: state0
        SignalTransition {
            targetState: state1
            signal: _cppObject.mySignal
            // signalState is mySignal's parameter
            guard: signalState === CppObject.State1
        }
    }

    State {
        id: state1
        SignalTransition {
            targetState: state2
            signal: _cppObject.mySignal
            // signalState is mySignal's parameter
            guard: signalState === CppObject.State2
        }
        onEntered: _cppObject.objectState = CppObject.State1
    }

    FinalState {
        id: state2
        onEntered: _cppObject.objectState = CppObject.State2
    }
    Component.onCompleted: stateMachine.running = true
}
