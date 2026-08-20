// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QTest>
#include <QTextToSpeech>
#include <QOperatingSystemVersion>

class tst_QVoice : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase_data();
    void init();

    void basic();
    void sameEngine();
    void datastream();
};

void tst_QVoice::initTestCase_data()
{
    qInfo("Available text-to-speech engines:");
    QTest::addColumn<QString>("engine");
    const auto engines = QTextToSpeech::availableEngines();
    if (!engines.size())
        QSKIP("No speech engines available, skipping test case");
    for (const auto &engine : engines) {
        QTest::addRow("%s", engine.toUtf8().constData()) << engine;
        qInfo().noquote() << "- " << engine;
    }
}

void tst_QVoice::init()
{
    QFETCH_GLOBAL(QString, engine);
    if (engine == "speechd") {
        QTextToSpeech tts(engine);
        if (tts.state() == QTextToSpeech::Error) {
            QSKIP("speechd engine reported an error, "
                  "make sure the speech-dispatcher service is running!");
        }
    } else if (engine == "darwin"
        && QOperatingSystemVersion::current() <= QOperatingSystemVersion::MacOSMojave) {
        QTextToSpeech tts(engine);
        if (!tts.availableLocales().size())
            QSKIP("iOS engine is not functional on macOS <= 10.14");
    }
}

/*
    Test basic value type semantics.
*/
void tst_QVoice::basic()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);
    const QList<QVoice> voices = tts.availableVoices();
    QVERIFY(voices.size());

    QVoice emptyVoice;
    for (const auto &voice : voices) {
        QCOMPARE(voice, voice);
        QCOMPARE(voice.locale(), tts.locale());
        QVERIFY(voice != emptyVoice);

        QVoice voiceCopy = voice;
        QCOMPARE(voiceCopy, voice);
    }

    QVoice movedFrom = voices.first();
    QVoice movedTo = std::move(movedFrom);
    QCOMPARE(movedTo, voices.first());
    QCOMPARE(movedFrom, emptyVoice);
}

/*
    A QVoice from one engine should match the same voice from the same engine.
*/
void tst_QVoice::sameEngine()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts1(engine);
    QTextToSpeech tts2(engine);
    QTRY_COMPARE(tts1.state(), QTextToSpeech::Ready);
    QTRY_COMPARE(tts2.state(), QTextToSpeech::Ready);

    const QList<QVoice> voices = tts1.availableVoices();
    QVERIFY(voices.size());
    QCOMPARE(tts1.availableVoices(), tts2.availableVoices());

    for (const auto &voice : voices)
        QVERIFY(tts2.availableVoices().indexOf(voice) != -1);
}

void tst_QVoice::datastream()
{
    QFETCH_GLOBAL(QString, engine);
    QVoice savedVoice;

    QByteArray storage;
    {
        QTextToSpeech tts(engine);
        QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);
        const QList<QVoice> voices = tts.availableVoices();
        QVERIFY(voices.size());
        savedVoice = voices.first();

        QDataStream writeStream(&storage, QIODevice::WriteOnly);
        writeStream << savedVoice;
        QVERIFY(storage.size() > 0);
    }

    QVoice loadedVoice;
    QDataStream readStream(storage);
    readStream >> loadedVoice;

    QCOMPARE(loadedVoice, savedVoice);
}

QTEST_MAIN(tst_QVoice)
#include "tst_qvoice.moc"
