// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"

#include <QLoggingCategory>

#include <algorithm>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    QLoggingCategory::setFilterRules(u"qt.speech.tts=true \n qt.speech.tts.*=true"_s);

    // Populate engine selection list
    ui.engine->addItem(u"Default"_s, u"default"_s);
    const auto engines = QTextToSpeech::availableEngines();
    for (const QString &engine : engines)
        ui.engine->addItem(engine, engine);
    ui.engine->setCurrentIndex(0);
    engineSelected(0);

    connect(ui.pitch, &QSlider::valueChanged, this, &MainWindow::setPitch);
    connect(ui.rate, &QSlider::valueChanged, this, &MainWindow::setRate);
    connect(ui.volume, &QSlider::valueChanged, this, &MainWindow::setVolume);
    connect(ui.engine, &QComboBox::currentIndexChanged, this, &MainWindow::engineSelected);
    connect(ui.language, &QComboBox::currentIndexChanged, this, &MainWindow::languageSelected);
    connect(ui.voice, &QComboBox::currentIndexChanged, this, &MainWindow::voiceSelected);
}

void MainWindow::setRate(int rate)
{
    m_speech->setRate(rate / 10.0);
}

void MainWindow::setPitch(int pitch)
{
    m_speech->setPitch(pitch / 10.0);
}

void MainWindow::setVolume(int volume)
{
    m_speech->setVolume(volume / 100.0);
}

//! [stateChanged]
void MainWindow::stateChanged(QTextToSpeech::State state)
{
    switch (state) {
    case QTextToSpeech::Speaking:
        ui.statusbar->showMessage(tr("Speech started..."));
        break;
    case QTextToSpeech::Ready:
        ui.statusbar->showMessage(tr("Speech stopped..."), 2000);
        break;
    case QTextToSpeech::Paused:
        ui.statusbar->showMessage(tr("Speech paused..."));
        break;
    default:
        ui.statusbar->showMessage(tr("Speech error!"));
        break;
    }

    ui.pauseButton->setEnabled(state == QTextToSpeech::Speaking);
    ui.resumeButton->setEnabled(state == QTextToSpeech::Paused);
    ui.stopButton->setEnabled(state == QTextToSpeech::Speaking || state == QTextToSpeech::Paused);
}
//! [stateChanged]

void MainWindow::engineSelected(int index)
{
    const QString engineName = ui.engine->itemData(index).toString();

    delete m_speech;
    m_speech = engineName == u"default"
               ? new QTextToSpeech(this)
               : new QTextToSpeech(engineName, this);

    // some engines initialize asynchronously
    if (m_speech->state() == QTextToSpeech::Ready) {
        onEngineReady();
    } else {
        connect(m_speech, &QTextToSpeech::stateChanged, this, &MainWindow::onEngineReady,
                Qt::SingleShotConnection);
    }
}

static bool localeLessThan(const QLocale &l1, const QLocale &l2)
{
    return l1.name().compare(l2.name()) < 0;
}

void MainWindow::onEngineReady()
{
    if (m_speech->state() != QTextToSpeech::Ready) {
        stateChanged(m_speech->state());
        return;
    }

    const bool hasPauseResume = m_speech->engineCapabilities()
                              & QTextToSpeech::Capability::PauseResume;
    ui.pauseButton->setVisible(hasPauseResume);
    ui.resumeButton->setVisible(hasPauseResume);

    // Block signals of the languages combobox while populating
    QSignalBlocker blocker(ui.language);

    ui.language->clear();
    QList<QLocale> locales = m_speech->availableLocales();
    std::stable_sort(locales.begin(), locales.end(), localeLessThan);

    QLocale current = m_speech->locale();
    for (const QLocale &locale : std::as_const(locales)) {
        QString name(u"%1 (%2)"_s
                     .arg(QLocale::languageToString(locale.language()),
                          QLocale::territoryToString(locale.territory())));
        QVariant localeVariant(locale);
        ui.language->addItem(name, localeVariant);
        if (locale.name() == current.name())
            current = locale;
    }
    setRate(ui.rate->value());
    setPitch(ui.pitch->value());
    setVolume(ui.volume->value());
//! [say]
    connect(ui.speakButton, &QPushButton::clicked, m_speech, [this]{
        m_speech->say(ui.plainTextEdit->toPlainText());
    });
//! [say]
//! [stop]
    connect(ui.stopButton, &QPushButton::clicked, m_speech, [this]{
        m_speech->stop();
    });
//! [stop]
//! [pause]
    connect(ui.pauseButton, &QPushButton::clicked, m_speech, [this]{
        m_speech->pause();
    });
//! [pause]
//! [resume]
    connect(ui.resumeButton, &QPushButton::clicked, m_speech, &QTextToSpeech::resume);
//! [resume]

    connect(m_speech, &QTextToSpeech::stateChanged, this, &MainWindow::stateChanged);
    connect(m_speech, &QTextToSpeech::localeChanged, this, &MainWindow::localeChanged);

    blocker.unblock();

    localeChanged(current);
}

void MainWindow::languageSelected(int language)
{
    QLocale locale = ui.language->itemData(language).toLocale();
    m_speech->setLocale(locale);
}

void MainWindow::voiceSelected(int index)
{
    m_speech->setVoice(m_voices.at(index));
}

void MainWindow::localeChanged(const QLocale &locale)
{
    QVariant localeVariant(locale);
    ui.language->setCurrentIndex(ui.language->findData(localeVariant));

    QSignalBlocker blocker(ui.voice);

    ui.voice->clear();

    m_voices = m_speech->availableVoices();
    QVoice currentVoice = m_speech->voice();
    for (const QVoice &voice : std::as_const(m_voices)) {
        ui.voice->addItem(u"%1 - %2 - %3"_s
                          .arg(voice.name(), QVoice::genderName(voice.gender()),
                               QVoice::ageName(voice.age())));
        if (voice.name() == currentVoice.name())
            ui.voice->setCurrentIndex(ui.voice->count() - 1);
    }
}

MainWindow::~MainWindow()
{
    m_speech->disconnect(this);
}
