// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech.h"
#include "qtexttospeech_p.h"

#include <QtCore/qcborarray.h>
#include <QtCore/qdebug.h>
#include <QtCore/private/qfactoryloader_p.h>

#include <QtMultimedia/qaudiobuffer.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
        ("org.qt-project.qt.speech.tts.plugin/6.0",
         QLatin1String("/texttospeech")))

QMutex QTextToSpeechPrivate::m_mutex;

QTextToSpeechPrivate::QTextToSpeechPrivate(QTextToSpeech *speech)
    : q_ptr(speech)
{
    qRegisterMetaType<QTextToSpeech::State>();
    qRegisterMetaType<QTextToSpeech::ErrorReason>();
}

QTextToSpeechPrivate::~QTextToSpeechPrivate()
{
}

void QTextToSpeechPrivate::setEngineProvider(const QString &engine, const QVariantMap &params)
{
    Q_Q(QTextToSpeech);

    q->stop(QTextToSpeech::BoundaryHint::Immediate);
    m_engine.reset();

    m_providerName = engine;
    if (m_providerName.isEmpty()) {
        const auto plugins = QTextToSpeechPrivate::plugins();
        int priority = -1;
        for (const auto &&[provider, metadata] : plugins.asKeyValueRange()) {
            const int pluginPriority = metadata.value(QStringLiteral("Priority")).toInteger();
            if (pluginPriority > priority) {
                priority = pluginPriority;
                m_providerName = provider;
            }
        }
        if (m_providerName.isEmpty()) {
            qCritical() << "No text-to-speech plug-ins were found.";
            return;
        }
    }
    if (!loadMeta()) {
        qCritical() << "Text-to-speech plug-in" << m_providerName << "is not supported.";
        return;
    }
    loadPlugin();
    if (m_plugin) {
        QString errorString;
        m_engine.reset(m_plugin->createTextToSpeechEngine(params, nullptr, &errorString));
        if (!m_engine) {
            qCritical() << "Error creating text-to-speech engine" << m_providerName
                        << (errorString.isEmpty() ? QStringLiteral("") : (QStringLiteral(": ") + errorString));
        }
    } else {
        qCritical() << "Error loading text-to-speech plug-in" << m_providerName;
    }

    if (m_engine) {
        // We have to maintain the public state separately from the engine's actual
        // state, as we use it to manage queued texts
        updateState(m_engine->state());
        QObjectPrivate::connect(m_engine.get(), &QTextToSpeechEngine::stateChanged,
                                this, &QTextToSpeechPrivate::updateState);
        // The other engine signals are directly forwarded to public API signals
        QObject::connect(m_engine.get(), &QTextToSpeechEngine::errorOccurred,
                         q, &QTextToSpeech::errorOccurred);
        QObject::connect(m_engine.get(), &QTextToSpeechEngine::sayingWord,
                         q, [this, q](const QString &word, qsizetype start, qsizetype length){
            emit q->sayingWord(word, m_currentUtterance, start, length);
        });
    } else {
        m_providerName.clear();
    }
}

bool QTextToSpeechPrivate::loadMeta()
{
    m_plugin = nullptr;
    m_metaData = QCborMap();

    QList<QCborMap> candidates = QTextToSpeechPrivate::plugins().values(m_providerName);

    int versionFound = -1;

    // figure out which version of the plugin we want
    for (int i = 0; i < candidates.size(); ++i) {
        QCborMap meta = candidates[i];
        if (int ver = meta.value(QLatin1String("Version")).toInteger(); ver > versionFound) {
            versionFound = ver;
            m_metaData = std::move(meta);
        }
    }

    if (m_metaData.isEmpty()) {
        m_metaData.insert(QLatin1String("index"), -1); // not found
        return false;
    }

    return true;
}

void QTextToSpeechPrivate::loadPlugin()
{
    int idx = m_metaData.value(QLatin1String("index")).toInteger();
    if (idx < 0) {
        m_plugin = nullptr;
        return;
    }
    m_plugin = qobject_cast<QTextToSpeechPlugin *>(loader()->instance(idx));
}

QMultiHash<QString, QCborMap> QTextToSpeechPrivate::plugins(bool reload)
{
    static QMultiHash<QString, QCborMap> plugins;
    static bool alreadyDiscovered = false;
    QMutexLocker lock(&m_mutex);

    if (reload == true)
        alreadyDiscovered = false;

    if (!alreadyDiscovered) {
        loadPluginMetadata(plugins);
        alreadyDiscovered = true;
    }
    return plugins;
}

void QTextToSpeechPrivate::loadPluginMetadata(QMultiHash<QString, QCborMap> &list)
{
    QFactoryLoader *l = loader();
    QList<QPluginParsedMetaData> meta = l->metaData();
    for (int i = 0; i < meta.size(); ++i) {
        QCborMap obj = meta.at(i).value(QtPluginMetaDataKeys::MetaData).toMap();
        obj.insert(QLatin1String("index"), i);
        list.insert(obj.value(QLatin1String("Provider")).toString(), obj);
    }
}

void QTextToSpeechPrivate::updateState(QTextToSpeech::State newState)
{
    Q_Q(QTextToSpeech);
    if (m_state == newState)
        return;

    if (newState == QTextToSpeech::Ready) {
        // If we have more text to process, start the next request immediately,
        // and ignore the transition to Ready (don't emit the signals).
        if (!m_pendingUtterances.isEmpty()) {
            const QString nextText = m_pendingUtterances.first();
            // QTextToSpeech::pause prepends an empty entry to request a pause
            if (nextText.isEmpty()) {
                m_state = QTextToSpeech::Paused;
                m_pendingUtterances.dequeue();
            } else {
                const auto nextFunction = [this]{
                    switch (m_state) {
                    case QTextToSpeech::Synthesizing:
                        return &QTextToSpeechEngine::synthesize;
                    case QTextToSpeech::Speaking:
                    case QTextToSpeech::Paused:
                        return &QTextToSpeechEngine::say;
                    default:
                        break;
                    }
                    return decltype(&QTextToSpeechEngine::synthesize)(nullptr);
                }();
                if (nextFunction) {
                    const auto oldState = m_state;
                    emit q->aboutToSynthesize(m_currentUtterance);
                    // connected slot could have called pause or stop, in which
                    // case the state changed or the pendingTexts got reset.
                    if (m_state == oldState && !m_pendingUtterances.isEmpty()) {
                        m_pendingUtterances.dequeue();
                        ++m_currentUtterance;
                        (m_engine.get()->*nextFunction)(nextText);
                        return;
                    } else if (m_state == QTextToSpeech::Paused) {
                        // In case of pause(), empty strings got inserted.
                        // We are already idle, so remove them again.
                        while (!m_pendingUtterances.isEmpty() && m_pendingUtterances.first().isEmpty())
                            m_pendingUtterances.dequeue();
                        return;
                    }
                    // in case of stop(), disconnect and update the state
                    disconnectSynthesizeFunctor();
                }
            }
        } else {
            // If we are done synthesizing and the functor-overload was used,
            // clear the temporary connection.
            disconnectSynthesizeFunctor();
        }
    }
    m_state = newState;
    emit q->stateChanged(newState);
}

void QTextToSpeechPrivate::disconnectSynthesizeFunctor()
{
    if (m_slotObject) {
        m_slotObject->destroyIfLastRef();
        m_slotObject = nullptr;
        m_engine->disconnect(m_synthesizeConnection);
    }
}

/*!
    \class QTextToSpeech
    \brief The QTextToSpeech class provides a convenient access to text-to-speech engines.
    \inmodule QtTextToSpeech

    Use \l say() to start reading text to the default audio device, and
    \l stop(), \l pause(), and \l resume() to control the reading of the text.

    \snippet hello_speak/mainwindow.cpp say
    \snippet hello_speak/mainwindow.cpp stop
    \snippet hello_speak/mainwindow.cpp pause
    \snippet hello_speak/mainwindow.cpp resume

    To synthesize text into PCM data for further processing, use synthesize().

    Use \l findVoices() to get a list of matching voices, or use \l
    availableVoices() to get the list of voices that support the current
    locale. Change the \l locale property, using one of the \l availableLocales()
    that is a good match for the language that the input text is in, and for
    the accent of the desired voice output. This will change the list of
    available voices on most platforms. Then use one of the available voices in
    a call to \l setVoice().

    Not every engine supports all features. Use the engineCapabilities() function to
    test which features are available, and adjust the usage of the class accordingly.

    \note Which locales and voices the engine supports depends usually on the Operating
    System configuration. E.g. on macOS, end users can install voices through the
    \e Accessibility panel in \e{System Preferences}.
*/

/*!
    \qmltype TextToSpeech
    \inqmlmodule QtTextToSpeech
    \brief The TextToSpeech type provides access to text-to-speech engines.

    Use \l say() to start reading text to the default audio device, and
    \l stop(), \l pause(), and \l resume() to control the reading of the text.

    \snippet quickspeech/Main.qml initialize
    \codeline
    \dots
    \codeline
    \snippet quickspeech/Main.qml say0
    \snippet quickspeech/Main.qml pause
    \snippet quickspeech/Main.qml resume
    \dots

    To synthesize text into PCM data for further processing, use synthesize().

    To set a voice, use the VoiceSelector attached property like this:

    \code
    TextToSpeech {
        VoiceSelector.locale: Qt.locale("en_UK")
        VoiceSelector.gender: Voice.Male
    }
    \endcode

    The first voice that matches all specified criteria will be used. If no voice
    matches all criteria, then the voice will not change.

    Alternatively, use \l findVoices() to get a list of matching voices, or use
    \l availableVoices() to get the list of voices that support the current
    locale. Change the \l locale property, using one of the \l availableLocales()
    that is a good match for the language that the input text is in, and for
    the accent of the desired voice output. This will change the list of
    available voices on most platforms. Then use one of the available voices in
    the \l voice property.

    Not every engine supports all features. Use the engineCapabilities()
    function to test which features are available, and adjust the usage of the
    type accordingly.

    \note Which locales and voices the engine supports depends usually on the Operating
    System configuration. E.g. on macOS, end users can install voices through the
    \e Accessibility panel in \e{System Preferences}.
*/

/*!
    \enum QTextToSpeech::State

    \brief This enum describes the current state of the text-to-speech engine.

    \value Ready        The synthesizer is ready to start a new text. This is
                        also the state after a text was finished.
    \value Speaking     Text is being spoken.
    \value Synthesizing Text is being synthesized into PCM data. The synthesized()
                        signal will be emitted with chunks of data.
    \value Paused       The synthesis was paused and can be resumed with \l resume().
    \value Error        An error has occurred. Details are given by \l errorReason().

    \sa QTextToSpeech::ErrorReason errorReason() errorString()
*/

/*!
    \enum QTextToSpeech::ErrorReason

    \brief This enum describes the current error, if any, of the QTextToSpeech engine.

    \value NoError          No error has occurred.
    \value Initialization   The backend could not be initialized, e.g. due to
                            a missing driver or operating system requirement.
    \value Configuration    The given backend configuration is inconsistent, e.g.
                            due to wrong voice name or parameters.
    \value Input            The given text could not be synthesized, e.g. due to
                            invalid size or characters.
    \value Playback         Audio playback failed e.g. due to missing audio device,
                            wrong format or audio streaming interruption.

    Use \l errorReason() to obtain the current error and \l errorString() to get the
    related error message.

    \sa errorOccurred()
*/

/*!
    \enum QTextToSpeech::BoundaryHint

    \brief describes when speech should be stopped and paused.

    \value Default          Uses the engine specific default behavior.
    \value Immediate        The engine should stop playback immediately.
    \value Word             Stop speech when the current word is finished.
    \value Sentence         Stop speech when the current sentence is finished.
    \value [since 6.6] Utterance
                            Stop speech when the current utterance is finished.
                            An utterance is the block of text used in a call to
                            say() or enqueue().

    \note These are hints to the engine. The current engine might not support
    all options.
*/

/*!
    Loads a text-to-speech engine from a plug-in that uses the default
    engine plug-in and constructs a QTextToSpeech object as the child
    of \a parent.

    The default engine is platform-specific.

    If the engine initializes correctly, then the \l state of the engine will
    change to QTextToSpeech::Ready; note that this might happen asynchronously.
    If the plugin fails to load, then \l state will be set to QTextToSpeech::Error.

    \sa availableEngines()
*/
QTextToSpeech::QTextToSpeech(QObject *parent)
    : QTextToSpeech(QString(), QVariantMap(), parent)
{
}

/*!
    Loads a text-to-speech engine from a plug-in that matches parameter \a engine and
    constructs a QTextToSpeech object as the child of \a parent.

    If \a engine is empty, the default engine plug-in is used. The default
    engine is platform-specific.

    If the engine initializes correctly, the \l state of the engine will be set
    to QTextToSpeech::Ready. If the plugin fails to load, or if the engine fails to
    initialize, the engine's \l state will be set to QTextToSpeech::Error.

    \sa availableEngines()
*/
QTextToSpeech::QTextToSpeech(const QString &engine, QObject *parent)
    : QTextToSpeech(engine, QVariantMap(), parent)
{
}

/*!
    \since 6.4

    Loads a text-to-speech engine from a plug-in that matches parameter \a engine and
    constructs a QTextToSpeech object as the child of \a parent, passing \a params
    through to the engine.

    If \a engine is empty, the default engine plug-in is used. The default
    engine is platform-specific. Which key/value pairs in \a params are supported
    depends on the engine. See \l{Qt TextToSpeech Engines}{the engine documentation}
    for details. Unsupported entries will be ignored.

    If the engine initializes correctly, the \l state of the engine will be set
    to QTextToSpeech::Ready. If the plugin fails to load, or if the engine fails to
    initialize, the engine's \l state will be set to QTextToSpeech::Error.

    \sa availableEngines()
*/
QTextToSpeech::QTextToSpeech(const QString &engine, const QVariantMap &params, QObject *parent)
    : QObject(*new QTextToSpeechPrivate(this), parent)
{
    Q_D(QTextToSpeech);
    // allow QDeclarativeTextToSpeech to skip initialization until the component
    // is complete
    if (engine != u"none"_s)
        d->setEngineProvider(engine, params);
    else
        d->m_providerName = engine;
}

/*!
    Destroys this QTextToSpeech object, stopping any speech.
*/
QTextToSpeech::~QTextToSpeech()
{
    stop(QTextToSpeech::BoundaryHint::Immediate);
}

/*!
    \qmlproperty string TextToSpeech::engine

    The engine used to synthesize text to speech.

    Changing the engine stops any ongoing speech.

    On most platforms, changing the engine will update the list of
    \l{availableLocales()}{available locales} and \l{availableVoices()}{available voices}.
*/

/*!
    \property QTextToSpeech::engine
    \brief the engine used to synthesize text to speech.
    \since 6.4

    Changing the engine stops any ongoing speech.

    On most platforms, changing the engine will update the list of
    \l{availableLocales()}{available locales} and \l{availableVoices()}{available voices}.
*/

/*!
    \since 6.4
    Sets the engine used by this QTextToSpeech object to \a engine, passing
    \a params through to the engine constructor.

    \return whether \a engine could be set successfully.

    Which key/value pairs in \a params are supported depends on the engine.
    See \l{Qt TextToSpeech Engines}{the engine documentation} for details.
    Unsupported entries will be ignored.
*/
bool QTextToSpeech::setEngine(const QString &engine, const QVariantMap &params)
{
    Q_D(QTextToSpeech);
    if (d->m_providerName == engine && params.isEmpty())
        return true;

    // read values from the old engine
    if (d->m_engine) {
        d->m_storedPitch = d->m_engine->pitch();
        d->m_storedRate = d->m_engine->rate();
        d->m_storedVolume = d->m_engine->volume();
    }

    d->setEngineProvider(engine, params);

    emit engineChanged(d->m_providerName);
    d->updateState(d->m_engine ? d->m_engine->state()
                               : QTextToSpeech::Error);

    // Restore values from the previous engine, or from
    // property setters before the engine was initialized.
    if (d->m_engine) {
        if (!qIsNaN(d->m_storedPitch))
            d->m_engine->setPitch(d->m_storedPitch);
        if (!qIsNaN(d->m_storedRate))
            d->m_engine->setRate(d->m_storedRate);
        if (!qIsNaN(d->m_storedVolume))
            d->m_engine->setVolume(d->m_storedVolume);

        // setting the engine might have changed these values
        if (double realPitch = pitch(); d->m_storedPitch != realPitch)
            emit pitchChanged(realPitch);
        if (double realRate = rate(); d->m_storedRate != realRate)
            emit rateChanged(realRate);
        if (double realVolume = volume(); d->m_storedVolume != realVolume)
            emit volumeChanged(realVolume);

        emit localeChanged(locale());
        emit voiceChanged(voice());
    }
    return d->m_engine.get();
}

QString QTextToSpeech::engine() const
{
    Q_D(const QTextToSpeech);
    return d->m_providerName;
}

/*!
    \enum QTextToSpeech::Capability
    \since 6.6
    \brief This enum describes the capabilities of a text-to-speech engine.

    \value None                 The engine implements none of the capabilities.
    \value Speak                The engine can play audio output from text.
    \value PauseResume          The engine can pause and then resume the audo output.
    \value WordByWordProgress   The engine emits the sayingWord() signal for
                                each word that gets spoken.
    \value Synthesize           The engine can \l{synthesize()}{synthesize} PCM
                                audio data from text.

    \sa engineCapabilities()
*/

/*!
    \qmlproperty enumeration TextToSpeech::engineCapabilities
    \brief This property holds the capabilities implemented by the current engine.
    \since 6.6

    \sa engine, QTextToSpeech::Capability
*/

/*!
    \property QTextToSpeech::engineCapabilities
    \brief the capabilities implemented by the current engine
    \since 6.6

    \sa engine
*/
QTextToSpeech::Capabilities QTextToSpeech::engineCapabilities() const
{
    Q_D(const QTextToSpeech);

    QTextToSpeech::Capabilities caps = d->m_engine
                                     ? d->m_engine->capabilities()
                                     : QTextToSpeech::Capability::None;
    if (caps != QTextToSpeech::Capability::None)
        return caps;
    if (d->m_providerName.isEmpty()) {
        qCritical() << "No engine set.";
        return caps;
    }

    const QMetaEnum capEnum = QMetaEnum::fromType<QTextToSpeech::Capabilities>();
    const auto plugin = QTextToSpeechPrivate::plugins().value(d->m_providerName);
    const auto capNames = plugin.value(QStringLiteral("Capabilities")).toArray();

    // compatibility: plugins that don't set Features can only speak
    if (capNames.isEmpty())
        caps |= QTextToSpeech::Capability::Speak;

    for (const auto capName : capNames) {
        const auto capString = capName.toString().toUtf8();
        bool ok = false;
        const QTextToSpeech::Capability capFlag = QTextToSpeech::Capability(capEnum.keyToValue(capString.data(), &ok));
        if (!ok) {
            qWarning("Unknown capability: '%s' doesn't map to any QTextToSpeech::Capability value",
                     capString.constData());
        } else {
            caps |= capFlag;
        }
    }
    return caps;
}

/*!
    \qmlmethod list<string> TextToSpeech::availableEngines()

    Holds the list of supported text-to-speech engine plug-ins.
*/

/*!
    Gets the list of supported text-to-speech engine plug-ins.

    \sa engine
*/
QStringList QTextToSpeech::availableEngines()
{
    return QTextToSpeechPrivate::plugins().keys();
}

/*!
    \qmlproperty enumeration TextToSpeech::state
    \brief This property holds the current state of the speech synthesizer.

    \sa QTextToSpeech::State say() stop() pause()

    \snippet quickspeech/Main.qml stateChanged
*/

/*!
    \property QTextToSpeech::state
    \brief the current state of the speech synthesizer.

    \snippet hello_speak/mainwindow.cpp stateChanged

    Use \l say() to start synthesizing text with the current \l voice and \l locale.
*/
QTextToSpeech::State QTextToSpeech::state() const
{
    Q_D(const QTextToSpeech);
    return d->m_state;
}

/*!
    \qmlsignal TextToSpeech::aboutToSynthesize(number id)

    \since 6.6

    This signal gets emitted just before the engine starts to synthesize the
    speech audio for \a id. Applications can use this signal to make last-minute
    changes to \l voice attributes, or to track the process of text enqueued
    via enqueue().

    \sa enqueue(), voice
*/

/*!
    \fn void QTextToSpeech::aboutToSynthesize(qsizetype id)
    \since 6.6

    This signal gets emitted just before the engine starts to synthesize the
    speech audio for \a id. The \a id is the value returned by a call to enqueue(),
    Applications can use this signal to make last-minute changes to \l voice
    attributes, or to track the process of text enqueued via enqueue().

    \sa enqueue(), synthesize(), voice
*/

/*!
    \qmlsignal TextToSpeech::sayingWord(string word, int id, int start, int length)
    \since 6.6

    This signal is emitted when the \a word, which is the slice of text indicated
    by \a start and \a length in the utterance \a id, gets played to the audio device.

    \note This signal requires that the engine has the
    \l {QTextToSpeech::Capability::}{WordByWordProgress} capability.

    The following code highlights the word that is spoken in a TextArea \c input:
    \snippet quickspeech/Main.qml sayingWord

    \sa QTextToSpeech::Capability, say()
*/

/*!
    \fn void QTextToSpeech::sayingWord(const QString &word, qsizetype id, qsizetype start, qsizetype length)
    \since 6.6

    This signal is emitted when the \a word, which is the slice of text indicated
    by \a start and \a length in the utterance \a id, gets played to the audio device.

    \note This signal requires that the engine has the
    \l {QTextToSpeech::Capability::}{WordByWordProgress} capability.

    \sa Capability, say()
*/

/*!
    \qmlsignal void TextToSpeech::errorOccurred(enumeration reason, string errorString)

    This signal is emitted after an error occurred and the \l state has been set to
    \c TextToSpeech.Error. The \a reason parameter specifies the type of error,
    and the \a errorString provides a human-readable error description.

    \sa state errorReason(), errorString()
*/

/*!
    \fn void QTextToSpeech::errorOccurred(QTextToSpeech::ErrorReason reason, const QString &errorString)

    This signal is emitted after an error occurred and the \l state has been set to
    QTextToSpeech::Error. The \a reason parameter specifies the type of error,
    and the \a errorString provides a human-readable error description.

    QTextToSpeech::ErrorReason is not a registered metatype, so for queued
    connections, you will have to register it with Q_DECLARE_METATYPE() and
    qRegisterMetaType().

    \sa errorReason(), errorString(), {Creating Custom Qt Types}
*/

/*!
    \qmlmethod enumeration TextToSpeech::errorReason()
    \return the reason why the engine has reported an error.

    \sa QTextToSpeech::ErrorReason
*/

/*!
    \return the reason why the engine has reported an error.

    \sa state errorOccurred()
*/
QTextToSpeech::ErrorReason QTextToSpeech::errorReason() const
{
    Q_D(const QTextToSpeech);
    if (d->m_engine)
        return d->m_engine->errorReason();
    return QTextToSpeech::ErrorReason::Initialization;
}

/*!
    \qmlmethod string TextToSpeech::errorString()
    \return the current engine error message.
*/

/*!
    \return the current engine error message.

    \sa errorOccurred()
*/
QString QTextToSpeech::errorString() const
{
    Q_D(const QTextToSpeech);
    if (d->m_engine)
        return d->m_engine->errorString();
    return tr("Text to speech engine not initialized");
}

/*!
    \qmlmethod void TextToSpeech::say(string text)

    Starts synthesizing the \a text.

    This function starts sythesizing the speech asynchronously, and reads the text to the
    default audio output device.

    \snippet quickspeech/Main.qml say0

    \note All in-progress readings are stopped before beginning to read the recently
    synthesized text.

    The current state is available using the \l state property, and is
    set to \l {QTextToSpeech::Speaking} once the reading starts. When the reading is done,
    \l state will be set to \l {QTextToSpeech::Ready}.

    \sa stop(), pause(), resume()
*/

/*!
    Starts speaking the \a text.

    This function starts sythesizing the speech asynchronously, and reads the text to the
    default audio output device.

    \snippet hello_speak/mainwindow.cpp say

    \note All in-progress readings are stopped before beginning to read the recently
    synthesized text.

    The current state is available using the \l state property, and is
    set to \l Speaking once the reading starts. When the reading is done,
    \l state will be set to \l Ready.

    \sa enqueue(), stop(), pause(), resume(), synthesize()
*/
void QTextToSpeech::say(const QString &text)
{
    Q_D(QTextToSpeech);
    d->m_pendingUtterances = {};
    d->m_utteranceCounter = 1;
    if (d->m_engine) {
        emit aboutToSynthesize(0);
        d->m_engine->say(text);
    }
}

/*!
    \qmlmethod int TextToSpeech::enqueue(string utterance)
    \since 6.6

    Adds \a utterance to the queue of text to be spoken, and starts speaking.

    If the engine's \l state is currently \c Ready, \a utterance will be spoken
    immediately. Otherwise, the engine will start to speak \a utterance once it
    has finished speaking the current text.

    Each time the engine proceeds to the next text entry in the queue, the
    aboutToSynthesize() signal gets emitted. This allows applications to keep
    track of the progress, and to make last-minute changes to voice attributes.

    Calling stop() clears the queue.

    \sa say(), stop(), aboutToSynthesize()
*/

/*!
    \since 6.6

    Adds \a utterance to the queue of texts to be spoken, and starts speaking.
    Returns the index of the text in the queue, or -1 in case of an error.

    If the engine's \l state is currently \c Ready, \a utterance will be spoken
    immediately. Otherwise, the engine will start to speak \a utterance once it
    has finished speaking the current text.

    Each time the engine proceeds to the next text entry in the queue, the
    aboutToSynthesize() signal gets emitted. This allows applications to keep
    track of the progress, and to make last-minute changes to voice attributes.

    Calling stop() clears the queue. To pause the engine at the end of a text,
    use the \l {QTextToSpeech::BoundaryHint::}{Utterance} boundary hint.

    \sa say(), stop(), aboutToSynthesize(), synthesize()
*/
qsizetype QTextToSpeech::enqueue(const QString &utterance)
{
    Q_D(QTextToSpeech);
    if (!d->m_engine || utterance.isEmpty())
        return -1;

    switch (d->m_engine->state()) {
    case QTextToSpeech::Error:
        return -1;
    case QTextToSpeech::Ready:
        emit aboutToSynthesize(0);
        d->m_engine->say(utterance);
        break;
    case QTextToSpeech::Speaking:
    case QTextToSpeech::Synthesizing:
    case QTextToSpeech::Paused:
        d->m_pendingUtterances.enqueue(utterance);
        break;
    }

    return d->m_utteranceCounter++;
}

/*!
    \fn template<typename Functor> void QTextToSpeech::synthesize(
            const QString &text, Functor &&functor)
    \fn template<typename Functor> void QTextToSpeech::synthesize(
            const QString &text, const QObject *context, Functor &&functor)
    \since 6.6

    Synthesizes the \a text into raw audio data.

    This function synthesizes the speech asynchronously into raw audio data.
    When data is available, the \a functor will be called as
    \c {functor(QAudioFormat format, QByteArray bytes)}, with \c format
    describing the \l {QAudioFormat}{format} of the data in \c bytes;
    or as \c {functor(QAudioBuffer &buffer)}.

    The \l state property is set to \l Synthesizing when the synthesis starts,
    and to \l Ready once the synthesis is finished. While synthesizing, the
    \a functor might be called multiple times, possibly with changing values
    for \c format.

    The \a functor can be a callable, like a lambda or free function, with an
    optional \a context object:

    \code
    tts.synthesize("Hello world", [](const QAudioFormat &format, const QByteArray &bytes){
        // process data according to format
    });
    \endcode

    or a member function of the \a context object:

    \code
    struct PCMProcessor : QObject
    {
        void processData(const QAudioFormat &format, const QByteArray &bytes)
        {
            // process data according to format
        }
    } processor;
    tts.synthesize("Hello world", &processor, &PCMProcessor::processData);
    \endcode

    If \a context is destroyed, then the \a functor will no longer get called.

    \note This API requires that the engine has the
    \l {QTextToSpeech::Capability::}{Synthesize} capability.

    \sa say(), stop()
*/

/*!
    \internal

    Handles the engine's synthesized() signal to call \a slotObj on the \a context
    object. The slot object and the temporary connection are stored and released
    in updateState() when the state of the engine transitions back to Ready.
*/
void QTextToSpeech::synthesizeImpl(const QString &text,
                                   QtPrivate::QSlotObjectBase *slotObj, const QObject *context,
                                   SynthesizeOverload overload)
{
    Q_D(QTextToSpeech);
    Q_ASSERT(slotObj);
    if (d->m_slotObject)
        d->m_slotObject->destroyIfLastRef();
    d->m_slotObject = slotObj;
    const auto receive = [d, context, overload](const QAudioFormat &format, const QByteArray &bytes){
        Q_ASSERT(d->m_slotObject);
        if (overload == SynthesizeOverload::AudioBuffer) {
            const QAudioBuffer buffer(bytes, format);
            void *args[] = {nullptr, const_cast<QAudioBuffer *>(&buffer)};
            d->m_slotObject->call(const_cast<QObject *>(context), args);
        } else {
            void *args[] = {nullptr,
                            const_cast<QAudioFormat *>(&format),
                            const_cast<QByteArray *>(&bytes)};
            d->m_slotObject->call(const_cast<QObject *>(context), args);
        }
    };
    d->m_synthesizeConnection = connect(d->m_engine.get(), &QTextToSpeechEngine::synthesized,
                                        context ? context : this, receive);

    if (!d->m_engine)
        return;

    if (d->m_engine->state() == QTextToSpeech::Synthesizing)
        d->m_pendingUtterances.enqueue(text);
    else
        d->m_engine->synthesize(text);
}

/*!
    \qmlmethod void TextToSpeech::stop(BoundaryHint boundaryHint)

    Stops the current reading at \a boundaryHint, and clears the
    queue of pending texts.

    The reading cannot be resumed. Whether the \a boundaryHint is
    respected depends on the engine.

    \sa say(), enqueue(), pause(), QTextToSpeech::BoundaryHint
*/

/*!
    Stops the current reading at \a boundaryHint, and clears the
    queue of pending texts.

    The reading cannot be resumed. Whether the \a boundaryHint is
    respected depends on the engine.

    \sa say(), enqueue(), pause()
*/
void QTextToSpeech::stop(BoundaryHint boundaryHint)
{
    Q_D(QTextToSpeech);
    d->m_pendingUtterances = {};
    d->m_utteranceCounter = 0;
    if (d->m_engine) {
        if (boundaryHint == QTextToSpeech::BoundaryHint::Immediate)
            d->disconnectSynthesizeFunctor();
        d->m_engine->stop(boundaryHint);
    }
}

/*!
    \qmlmethod void TextToSpeech::pause(BoundaryHint boundaryHint)

    Pauses the current speech at \a boundaryHint.

    Whether the \a boundaryHint is respected depends on the  \l engine.

    \sa resume(), QTextToSpeech::BoundaryHint,
        {QTextToSpeech::Capabilities}{PauseResume}
*/

/*!
    Pauses the current speech at \a boundaryHint.

    Whether the \a boundaryHint is respected depends on the  \l engine.

    \sa resume(), {QTextToSpeech::Capabilities}{PauseResume}
*/
void QTextToSpeech::pause(BoundaryHint boundaryHint)
{
    Q_D(QTextToSpeech);
    if (!d->m_engine || d->m_state != QTextToSpeech::Speaking)
        return;

    if (boundaryHint == BoundaryHint::Utterance) {
        if (d->m_pendingUtterances.isEmpty() || !d->m_pendingUtterances.first().isEmpty())
            d->m_pendingUtterances.prepend(QString());
    }
    // pause called in response to aboutToSynthesize
    if (d->m_engine->state() == QTextToSpeech::Ready) {
        d->updateState(QTextToSpeech::Paused);
    } else {
        d->m_engine->pause(boundaryHint);
    }
}

/*!
    \qmlmethod void TextToSpeech::resume()

    Resume speaking after \l pause() has been called.

    \sa pause()
*/

/*!
    Resume speaking after \l pause() has been called.

    \note On Android, resuming paused speech will restart from the beginning.
    This is a limitation of the underlying text-to-speech engine.

    \sa pause()
*/
void QTextToSpeech::resume()
{
    Q_D(QTextToSpeech);
    if (d->m_state != QTextToSpeech::Paused)
        return;

    if (d->m_engine) {
        // If we are pausing before proceeding with the next utterance,
        // then continue with the next pending text.
        if (d->m_engine->state() == QTextToSpeech::Ready)
            d->updateState(QTextToSpeech::Ready);
        else
            d->m_engine->resume();
    }
}

/*!
    \qmlproperty double TextToSpeech::pitch

    This property hold the voice pitch, ranging from -1.0 to 1.0.

    The default of 0.0 is the normal speech pitch.
*/

/*!
    \property QTextToSpeech::pitch
    \brief the voice pitch, ranging from -1.0 to 1.0.

    The default of 0.0 is the normal speech pitch.
*/

void QTextToSpeech::setPitch(double pitch)
{
    Q_D(QTextToSpeech);
    pitch = qBound(-1.0, pitch, 1.0);

    if (!d->m_engine) {
        if (d->m_storedPitch != pitch) {
            d->m_storedPitch = pitch;
            emit pitchChanged(pitch);
        }
        return;
    }
    if (d->m_engine->pitch() == pitch)
        return;
    if (d->m_engine && d->m_engine->setPitch(pitch))
        emit pitchChanged(pitch);
}

double QTextToSpeech::pitch() const
{
    Q_D(const QTextToSpeech);
    if (d->m_engine)
        return d->m_engine->pitch();
    return qIsNaN(d->m_storedPitch) ? 0.0 : d->m_storedPitch;
}

/*!
    \qmlproperty double TextToSpeech::rate

    \brief This property holds the current voice rate, ranging from -1.0 to 1.0.

    The default of 0.0 is the normal speech flow.
*/

/*!
    \property QTextToSpeech::rate
    \brief the current voice rate, ranging from -1.0 to 1.0.

    The default value of 0.0 is normal speech flow.
*/
void QTextToSpeech::setRate(double rate)
{
    Q_D(QTextToSpeech);
    rate = qBound(-1.0, rate, 1.0);

    if (!d->m_engine) {
        if (d->m_storedRate != rate) {
            d->m_storedRate = rate;
            emit rateChanged(rate);
        }
        return;
    }
    if (d->m_engine->rate() == rate)
        return;
    if (d->m_engine && d->m_engine->setRate(rate))
        emit rateChanged(rate);
}

double QTextToSpeech::rate() const
{
    Q_D(const QTextToSpeech);
    if (d->m_engine)
        return d->m_engine->rate();
    return qIsNaN(d->m_storedRate) ? 0.0 : d->m_storedRate;
}

/*!
    \qmlproperty double TextToSpeech::volume

    \brief This property holds the current volume, ranging from 0.0 to 1.0.

    The default value is the platform's default volume.
*/

/*!
    \property QTextToSpeech::volume
    \brief the current volume, ranging from 0.0 to 1.0.

    The default value is the platform's default volume.
*/
void QTextToSpeech::setVolume(double volume)
{
    Q_D(QTextToSpeech);
    volume = qBound(0.0, volume, 1.0);

    if (!d->m_engine) {
        if (d->m_storedVolume != volume) {
            d->m_storedVolume = volume;
            emit volumeChanged(volume);
        }
        return;
    }
    if (d->m_engine->volume() == volume)
        return;
    if (d->m_engine->setVolume(volume))
        emit volumeChanged(volume);
}

double QTextToSpeech::volume() const
{
    Q_D(const QTextToSpeech);
    if (d->m_engine)
        return d->m_engine->volume();
    return qIsNaN(d->m_storedVolume) ? 0.0 : d->m_storedVolume;
}

/*!
    \qmlproperty locale TextToSpeech::locale

    \brief This property holds the current locale in use.

    By default, the system locale is used.

    \sa voice
*/

/*!
    \property QTextToSpeech::locale
    \brief the current locale in use.

    By default, the system locale is used.

    On some platforms, changing the locale will update the list of
    \l{availableVoices()}{available voices}, and if the current voice is not
    available with the new locale, a new voice will be set.

    \sa voice, findVoices()
*/
void QTextToSpeech::setLocale(const QLocale &locale)
{
    Q_D(QTextToSpeech);
    if (!d->m_engine)
        return;

    if (d->m_engine->locale() == locale)
        return;

    const QVoice oldVoice = voice();
    if (d->m_engine->setLocale(locale)) {
        emit localeChanged(locale);
        if (const QVoice newVoice = d->m_engine->voice(); oldVoice != newVoice)
            emit voiceChanged(newVoice);
    }
}

QLocale QTextToSpeech::locale() const
{
    Q_D(const QTextToSpeech);
    if (d->m_engine)
        return d->m_engine->locale();
    return QLocale();
}

/*!
    \qmlmethod list<voice> TextToSpeech::availableLocales()

    Holds the list of locales that are supported by the active \l engine.
*/

/*!
    \return the list of locales that are supported by the active \l engine.

    \sa availableVoices(), findVoices()
*/
QList<QLocale> QTextToSpeech::availableLocales() const
{
    Q_D(const QTextToSpeech);
    if (d->m_engine)
        return d->m_engine->availableLocales();
    return QList<QLocale>();
}

/*!
    \qmlproperty voice TextToSpeech::voice

    \brief This property holds the voice that will be used for the speech.

    The voice needs to be one of the \l{availableVoices()}{voices available} for
    the engine.

    On some platforms, setting the voice changes other voice attributes such
    as \l locale, \l pitch, and so on. These changes trigger the emission of
    signals.
*/

/*!
    \property QTextToSpeech::voice
    \brief the voice that will be used for the speech.

    The voice needs to be one of the \l{availableVoices()}{voices available} for
    the engine.

    On some platforms, setting the voice changes other voice attributes such
    as \l locale, \l pitch, and so on. These changes trigger the emission of
    signals.

    \sa findVoices()
*/
void QTextToSpeech::setVoice(const QVoice &voice)
{
    Q_D(QTextToSpeech);
    if (!d->m_engine)
        return;

    if (d->m_engine->voice() == voice)
        return;

    const QLocale oldLocale = locale();
    if (d->m_engine->setVoice(voice)) {
        emit voiceChanged(voice);
        if (const QLocale newLocale = d->m_engine->locale(); newLocale != oldLocale)
            emit localeChanged(newLocale);
    }
}

QVoice QTextToSpeech::voice() const
{
    Q_D(const QTextToSpeech);
    if (d->m_engine)
        return d->m_engine->voice();
    return QVoice();
}

/*!
    \qmlmethod list<voice> TextToSpeech::availableVoices()

    Holds the list of voices available for the current \l locale.
*/

/*!
    \return the list of voices available for the current \l locale.

    \note If no locale has been set, the system locale is used.

    \sa availableLocales(), findVoices()
*/
QList<QVoice> QTextToSpeech::availableVoices() const
{
    Q_D(const QTextToSpeech);
    if (d->m_engine)
        return d->m_engine->availableVoices();
    return QList<QVoice>();
}

/*!
    \fn template<typename ...Args> QList<QVoice> QTextToSpeech::findVoices(Args &&...args) const
    \since 6.6

    \return the list of voices that match the criteria in \a args.

    The arguments in \a args are processed in order to assemble the list of voices that
    match all of them. An argument of type QString is matched against the \l{QVoice::}{name},
    of the voice, an argument of type QLocale is matched agains the voice's
    \l {QVoice::}{locale}, etc. It is possible to specify only the \l {QLocale::}{Language} or
    \l {QLocale::}{Territory} of the desired voices, and the name can be matched against a
    \l {QRegularExpression}{regular expression}.

    This function returns all voices if the list of criteria is empty. Multiple criteria
    of the same type are not possible and will result in a compile-time error.

    \note Unless \a args includes the current \l {QTextToSpeech::}{locale}, this function
    might need to change the locale of the engine to get the list of all voices. This is
    engine specific, but might impact ongoing speech synthesis. It is therefore advisable
    to not call this function unless the \l {QTextToSpeech::}{state} is
    \l {QTextToSpeech::State}{Ready}.

    \sa availableVoices()
*/

/*!
    \qmlmethod list<voice> TextToSpeech::findVoices(map criteria)
    \since 6.6

    Returns the list of voices that match all the specified \a criteria.

    \a criteria is a map from voice property name to property value, supporting
    combinations of search criteria such as:

    \code
    let daniel = tts.findVoices({
        "name": "Daniel"
    })
    let maleEnglish = tts.findVoices({
        "gender": Voice.Male,
        "language": Qt.locale('en')
    })
    \endcode

    \sa VoiceSelector
*/

/*!
    \internal

    Returns the list of all voices. This requires iterating through all locales,
    unless \a locale is set, in which case we can just go through the voices for
    that locale.
*/
QList<QVoice> QTextToSpeech::allVoices(const QLocale *locale) const
{
    Q_D(const QTextToSpeech);
    if (!d->m_engine)
        return {};

    const QVoice oldVoice = d->m_engine->voice();

    QList<QVoice> voices;
    QSignalBlocker blockSignals(const_cast<QTextToSpeech *>(this));
    const QList<QLocale> allLocales = locale ? QList<QLocale>{*locale} : availableLocales();
    for (const auto &l : allLocales) {
        if (d->m_engine->locale() != l)
            d->m_engine->setLocale(l);
        voices << d->m_engine->availableVoices();
    }

    // reset back to old voice, which will have changed when we changed the
    // engine's locale.
    if (d->m_engine->voice() != oldVoice)
        d->m_engine->setVoice(oldVoice);

    return voices;
}

QT_END_NAMESPACE
