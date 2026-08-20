// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest 1.1
import QtRemoteObjects 5.12

TestCase {
    id: testCase

    Node {
        id: defaultNode
    }

    Node {
        id: nodeWithPersistedStore

        persistedStore: SettingsStore {}
    }

    name: "testIntegration"
    function test_integration()
    {
        // test defaultNode
        compare(defaultNode.registryUrl, "")
        compare(defaultNode.persistedStore, null)

        // test nodeWithPersistedStore
        compare(nodeWithPersistedStore.registryUrl, "")
        verify(nodeWithPersistedStore.persistedStore)
    }
}
