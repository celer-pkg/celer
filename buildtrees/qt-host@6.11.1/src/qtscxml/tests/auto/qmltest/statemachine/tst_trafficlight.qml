// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtQml.StateMachine

TestCase {
    StateMachine {
        id: machine
        initialState: red
        FinalState {
            id: finalState
        }

        State {
            id: red
            initialState: justRed
            State {
                id: justRed
                SignalTransition {
                    id: e1
                    targetState: waitingForGreen
                }
                SignalTransition {
                    id: finalSignal
                    targetState: finalState
                }
            }
            State {
                id: waitingForGreen
                TimeoutTransition {
                    id: e2
                    targetState: yellowred
                    timeout: 30
                }
            }
        }
        State {
            id: yellowred
            TimeoutTransition {
                id: e3
                targetState: green
                timeout: 10
            }
        }
        State {
            id: green
            TimeoutTransition {
                id: e4
                targetState: yellow
                timeout: 50
            }
        }
        State {
            id: yellow
            TimeoutTransition {
                id: e5
                targetState: red
                timeout: 10
            }
        }
    }

    SignalSpy {
        id: machineSpyRunning
        target: machine
        signalName: "runningChanged"
    }

    SignalSpy {
        id: redSpyActive
        target: red
        signalName: "activeChanged"
    }

    SignalSpy {
        id: yellowredSpyActive
        target: yellowred
        signalName: "activeChanged"
    }

    SignalSpy {
        id: greenSpyActive
        target: green
        signalName: "activeChanged"
    }

    SignalSpy {
        id: yellowSpyActive
        target: yellow
        signalName: "activeChanged"
    }


    name: "testTrafficLight"
    function test_trafficLight()
    {
        var i = 1;
        machine.start();
        tryCompare(machine, "running", true);
        tryCompare(machineSpyRunning, "count", 1);
        tryCompare(redSpyActive, "count", 1);
        for (; i <= 5; ++i) {
            e1.invoke();
            tryCompare(yellowredSpyActive, "count", i * 2);
            tryCompare(greenSpyActive, "count", i * 2);
            tryCompare(redSpyActive, "count", i * 2 + 1);
            tryCompare(yellowSpyActive, "count", i * 2);
        }
        finalSignal.guard = false;
        finalSignal.invoke();
        wait(100);
        tryCompare(machine, "running", true);
        finalSignal.guard = true;
        finalSignal.invoke();
        tryCompare(machine, "running", false);
        tryCompare(redSpyActive, "count", i * 2);
    }
}
