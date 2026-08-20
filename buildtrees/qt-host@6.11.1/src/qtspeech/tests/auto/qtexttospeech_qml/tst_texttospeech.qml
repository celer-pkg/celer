// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

import QtTest
import QtTextToSpeech

TestCase {
    id: testCase
    name: "TextToSpeech"

    TextToSpeech {
        id: tts
        engine: "mock"
    }

    // verifies that the mock engine is synchronous by default
    function initTestCase() {
        compare(tts.state, TextToSpeech.Ready)
    }

    Component {
        id: defaultEngine
        TextToSpeech {
            rate: 0.5
            volume: 0.8
            pitch: 0.1
        }
    }

    function test_defaultEngine() {
        let def = createTemporaryObject(defaultEngine, testCase)
        if (!def.engine)
            skip("No default engine available on this platform")
        else if (def.engine == "speechd")
            skip("Older libspeechd versions don't implement attribute getters")
        else
            console.log("The default tts engine is " + def.engine)

        compare(def.rate, 0.5)
        compare(def.volume, 0.8)
        compare(def.pitch, 0.1)
    }

    function test_availableLocales() {
        compare(tts.availableLocales().length, 5)
    }

    function test_availableVoices() {
        compare(tts.availableVoices().length, 2)
    }

    function test_findVoices() {
        let bob = tts.findVoices({name: "Bob"})
        compare(bob.length, 1)
        let women = tts.findVoices({gender: Voice.Female})
        compare(women.length, 5)
        let children = tts.findVoices({age: Voice.Child})
        compare(children.length, 1)
        // includes all english speakers, no matter where they're from
        let english = tts.findVoices({language: Qt.locale("en")})
        compare(english.length, 4)
        let bokmalers = tts.findVoices({locale: Qt.locale("NO")})
        compare(bokmalers.length, 2)
        let nynorskers = tts.findVoices({locale: Qt.locale("nn-NO")})
        compare(nynorskers.length, 2)

        let englishWomen = tts.findVoices({
            language: Qt.locale("en"),
            gender: Voice.Female,
            age: Voice.Adult
        });
        compare(englishWomen.length, 1)
    }

    Component {
        id: lateEngine
        TextToSpeech {
            rate: 0.5
            volume: 0.8
            pitch: 0.1
            engine: "mock"
        }
    }

    function test_lateEngine() {
        let tts = createTemporaryObject(lateEngine, testCase)
        tryCompare(tts, "state", TextToSpeech.Ready)

        compare(tts.rate, 0.5)
        compare(tts.volume, 0.8)
        compare(tts.pitch, 0.1)
        compare(tts.engine, "mock")

        tts.engine = ""
        // If there is no default engine, then we use mock
        if (!tts.engine)
            tts.engine = "mock";
        else if (tts.engine == "speechd")
            skip("Older libspeechd versions don't implement attribute getters")

        compare(tts.rate, 0.5)
        compare(tts.volume, 0.8)
        compare(tts.pitch, 0.1)
    }

    Component {
        id: name_selector
        TextToSpeech {
            engine: "mock"
            engineParameters: {
                "delayedInitialization": true
            }

            VoiceSelector.name: "Ingvild"
        }
    }

    Component {
        id: genderLanguage_selector
        TextToSpeech {
            engine: "mock"
            engineParameters: {
                "delayedInitialization": true
            }

            VoiceSelector.gender: Voice.Female
            VoiceSelector.language: Qt.locale("en")
        }
    }

    function test_voiceSelector() {
        var selector = createTemporaryObject(name_selector, testCase)
        tryCompare(selector, "state", TextToSpeech.Ready)

        compare(selector.voice.name, "Ingvild")

        // there is no way to get to QLocale::English from QML
        let EnglishID = 75

        selector = createTemporaryObject(genderLanguage_selector, testCase)
        tryCompare(selector, "state", TextToSpeech.Ready)

        verify(["Anne", "Mary"].includes(selector.voice.name))
        let oldName = selector.voice.name
        compare(selector.voice.gender, Voice.Female)
        compare(selector.voice.language, EnglishID)

        // overwrite after initialization
        selector.VoiceSelector.gender = Voice.Male
        // no change until select is called
        compare(selector.voice.name, oldName)
        selector.VoiceSelector.select()

        compare(selector.voice.name, "Bob")
        compare(selector.voice.gender, Voice.Male)
        compare(selector.voice.language, EnglishID)
    }

    function test_delayedSelection() {
        var selector = createTemporaryObject(name_selector, testCase)
        tryCompare(selector, "state", TextToSpeech.Ready)

        selector.VoiceSelector.gender = Voice.Female
        selector.VoiceSelector.name = "Kjersti"
        selector.VoiceSelector.select()

        compare(selector.voice.name, "Kjersti")
    }

    function test_regularExpressionName() {
        var selector = createTemporaryObject(name_selector, testCase)
        tryCompare(selector, "state", TextToSpeech.Ready)

        selector.VoiceSelector.name = /K.*/
        selector.VoiceSelector.select()

        verify(["Kjersti", "Kari"].includes(selector.voice.name))
    }
}
