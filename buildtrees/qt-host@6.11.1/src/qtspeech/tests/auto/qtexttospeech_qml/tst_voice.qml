// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtTextToSpeech

TestCase {
    id: testCase
    name: "Voice"

    TextToSpeech {
        id: tts
        engine: "mock"
    }

    // verifies that the mock engine is synchronous
    function initTestCase() {
        compare(tts.state, TextToSpeech.Ready)
    }

    // basic API test of the voice type and Voice namespace
    function test_default_voice() {
        compare(tts.voice.name, "Bob")
        compare(tts.voice.age, Voice.Adult)
        compare(tts.voice.gender, Voice.Male)
        compare(tts.voice.locale, Qt.locale("en-GB"))
    }
}
