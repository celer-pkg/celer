// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtTextToSpeech

ApplicationWindow {
    id: root
    visible: true
    title: qsTr("Text to Speech")
    minimumWidth: inputForm.implicitWidth
    minimumHeight: inputForm.implicitHeight + footer.implicitHeight

//! [initialize]
    TextToSpeech {
        id: tts
        volume: volumeSlider.value
        pitch: pitchSlider.value
        rate: rateSlider.value
//! [initialize]

//! [stateChanged]
        onStateChanged: updateStateLabel(state)

        function updateStateLabel(state)
        {
            switch (state) {
                case TextToSpeech.Ready:
                    statusLabel.text = qsTr("Ready")
                    break
                case TextToSpeech.Speaking:
                    statusLabel.text = qsTr("Speaking")
                    break
                case TextToSpeech.Paused:
                    statusLabel.text = qsTr("Paused...")
                    break
                case TextToSpeech.Error:
                    statusLabel.text = qsTr("Error!")
                    break
            }
        }
//! [stateChanged]

//! [sayingWord]
        onSayingWord: (word, id, start, length)=> {
            input.select(start, start + length)
        }
//! [sayingWord]
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        id: inputForm

        TextArea {
            id: input
            wrapMode: TextEdit.WordWrap
            text: qsTr("Hello, world!")
            Layout.fillWidth: true
            Layout.minimumHeight: implicitHeight
            font.pointSize: 24
        }
//! [say0]
        RowLayout {
            Button {
                text: qsTr("Speak")
                enabled: [TextToSpeech.Paused, TextToSpeech.Ready].includes(tts.state)
                onClicked: {
                    let voices = tts.availableVoices()
                    tts.voice = voices[voicesComboBox.currentIndex]
                    tts.say(input.text)
                }
            }
//! [say0]
//! [pause]
            Button {
                text: qsTr("Pause")
                enabled: tts.state == TextToSpeech.Speaking
                onClicked: tts.pause()
                visible: tts.engineCapabilities & TextToSpeech.Capabilities.PauseResume
            }
//! [pause]
//! [resume]
            Button {
                text: qsTr("Resume")
                enabled: tts.state == TextToSpeech.Paused
                onClicked: tts.resume()
                visible: tts.engineCapabilities & TextToSpeech.Capabilities.PauseResume
            }
//! [resume]
//! [stop]
            Button {
                text: qsTr("Stop")
                enabled: [TextToSpeech.Speaking, TextToSpeech.Paused].includes(tts.state)
                onClicked: tts.stop()
            }
//! [stop]
        }

        GridLayout {
            columns: 2

            Label {
                text: qsTr("Engine:")
            }
            ComboBox {
                id: enginesComboBox
                Layout.fillWidth: true
                model: tts.availableEngines()
                onActivated: {
                    tts.engine = textAt(currentIndex)
                    updateLocales()
                    updateVoices()
                }
            }
            Label {
                text: qsTr("Locale:")
            }
            ComboBox {
                id: localesComboBox
                Layout.fillWidth: true
                onActivated: {
                    let locales = tts.availableLocales()
                    tts.locale = locales[currentIndex]
                    updateVoices()
                }
            }
            Label {
                text: qsTr("Voice:")
            }
            ComboBox {
                id: voicesComboBox
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Volume:")
            }
            Slider {
                id: volumeSlider
                from: 0
                to: 1.0
                stepSize: 0.2
                value: 0.8
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Pitch:")
            }
            Slider {
                id: pitchSlider
                from: -1.0
                to: 1.0
                stepSize: 0.5
                value: 0
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Rate:")
            }
            Slider {
                id: rateSlider
                from: -1.0
                to: 1.0
                stepSize: 0.5
                value: 0
                Layout.fillWidth: true
            }
        }
    }
    footer: Label {
        id: statusLabel
    }

    Component.onCompleted: {
        enginesComboBox.currentIndex = tts.availableEngines().indexOf(tts.engine)
        // some engines initialize asynchronously
        if (tts.state == TextToSpeech.Ready) {
            engineReady()
        } else {
            tts.stateChanged.connect(root.engineReady)
        }

        tts.updateStateLabel(tts.state)
    }

    function engineReady() {
        tts.stateChanged.disconnect(root.engineReady)
        if (tts.state != TextToSpeech.Ready) {
            tts.updateStateLabel(tts.state)
            return;
        }
        updateLocales()
        updateVoices()
    }

    function updateLocales() {
        let allLocales = tts.availableLocales().map((locale) => locale.nativeLanguageName)
        let currentLocaleIndex = allLocales.indexOf(tts.locale.nativeLanguageName)
        localesComboBox.model = allLocales
        localesComboBox.currentIndex = currentLocaleIndex
    }

    function updateVoices() {
        voicesComboBox.model = tts.availableVoices().map((voice) => voice.name)
        let indexOfVoice = tts.availableVoices().indexOf(tts.voice)
        voicesComboBox.currentIndex = indexOfVoice
    }
}
