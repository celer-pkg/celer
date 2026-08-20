// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECH_H
#define QTEXTTOSPEECH_H

#include <QtCore/qlocale.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>
#include <QtTextToSpeech/qtexttospeech_global.h>
#include <QtTextToSpeech/qvoice.h>

#include <QtCore/qmap.h>

#include <QtCore/q20type_traits.h>

QT_BEGIN_NAMESPACE

class QAudioFormat;
class QAudioBuffer;

class QTextToSpeechPrivate;
class Q_TEXTTOSPEECH_EXPORT QTextToSpeech : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString engine READ engine WRITE setEngine NOTIFY engineChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)
    Q_PROPERTY(double rate READ rate WRITE setRate NOTIFY rateChanged FINAL)
    Q_PROPERTY(double pitch READ pitch WRITE setPitch NOTIFY pitchChanged FINAL)
    Q_PROPERTY(QLocale locale READ locale WRITE setLocale NOTIFY localeChanged FINAL)
    Q_PROPERTY(QVoice voice READ voice WRITE setVoice NOTIFY voiceChanged FINAL)
    Q_PROPERTY(Capabilities engineCapabilities READ engineCapabilities NOTIFY engineChanged REVISION(6, 6) FINAL)
    Q_DECLARE_PRIVATE(QTextToSpeech)

public:
    enum State {
        Ready,
        Speaking,
        Paused,
        Error,
        Synthesizing,
    };
    Q_ENUM(State)

    enum class ErrorReason {
        NoError,
        Initialization,
        Configuration,
        Input,
        Playback,
    };
    Q_ENUM(ErrorReason)

    enum class BoundaryHint {
        Default,
        Immediate,
        Word,
        Sentence,
        Utterance,
    };
    Q_ENUM(BoundaryHint)

    enum class Capability {
        None                = 0,
        Speak               = 1 << 0,
        PauseResume         = 1 << 1,
        WordByWordProgress  = 1 << 2,
        Synthesize          = 1 << 3,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)
    Q_FLAG(Capabilities)

    explicit QTextToSpeech(QObject *parent = nullptr);
    explicit QTextToSpeech(const QString &engine, QObject *parent = nullptr);
    explicit QTextToSpeech(const QString &engine, const QVariantMap &params,
                           QObject *parent = nullptr);
    ~QTextToSpeech() override;

    Q_INVOKABLE bool setEngine(const QString &engine, const QVariantMap &params = QVariantMap());
    QString engine() const;
    QTextToSpeech::Capabilities engineCapabilities() const;

    QTextToSpeech::State state() const;
    Q_INVOKABLE QTextToSpeech::ErrorReason errorReason() const;
    Q_INVOKABLE QString errorString() const;

    Q_INVOKABLE QList<QLocale> availableLocales() const;
    QLocale locale() const;

    QVoice voice() const;
    Q_INVOKABLE QList<QVoice> availableVoices() const;

    double rate() const;
    double pitch() const;
    double volume() const;

    Q_INVOKABLE static QStringList availableEngines();

    template <typename Functor>
    void synthesize(const QString &text,
#ifdef Q_QDOC
                    const QObject *receiver,
#else
                    const typename QtPrivate::ContextTypeForFunctor<Functor>::ContextType *receiver,
# endif // Q_QDOC
                    Functor &&func)
    {
        using Prototype2 = void(*)(QAudioFormat, QByteArray);
        using Prototype1 = void(*)(QAudioBuffer);
        if constexpr (qxp::is_detected_v<CompatibleCallbackTest2, Functor>) {
            synthesizeImpl(text, QtPrivate::makeCallableObject<Prototype2>(std::forward<Functor>(func)),
                           receiver, SynthesizeOverload::AudioFormatByteArray);
        } else if constexpr (qxp::is_detected_v<CompatibleCallbackTest1, Functor>) {
            synthesizeImpl(text, QtPrivate::makeCallableObject<Prototype1>(std::forward<Functor>(func)),
                           receiver, SynthesizeOverload::AudioBuffer);
        } else {
            static_assert(QtPrivate::type_dependent_false<Functor>(),
                          "Incompatible functor signature, must be either "
                          "(QAudioFormat, QByteArray) or (QAudioBuffer)!");
        }
    }

    // synthesize to a functor or function pointer (without context)
    template <typename Functor>
    void synthesize(const QString &text, Functor &&func)
    {
        synthesize(text, nullptr, std::forward<Functor>(func));
    }

    template <typename ...Args>
    QList<QVoice> findVoices(Args &&...args) const
    {
        // if any of the arguments is a locale, then we can avoid iterating through all
        // and only have to search through the voices for that locale.
        QLocale locale;
        QLocale *plocale = nullptr;
        if constexpr (std::disjunction_v<std::is_same<q20::remove_cvref_t<Args>, QLocale>...>) {
            locale = std::get<QLocale>(std::make_tuple(args...));
            plocale = &locale;
        }

        auto voices = allVoices(plocale);
        if constexpr (sizeof...(args) > 0)
            voices.removeIf([&](const QVoice &voice) -> bool { return !voiceMatches(voice, args...); });
        return voices;
    }

public Q_SLOTS:
    void say(const QString &text);
    qsizetype enqueue(const QString &text);
    void stop(QTextToSpeech::BoundaryHint boundaryHint = QTextToSpeech::BoundaryHint::Default);
    void pause(QTextToSpeech::BoundaryHint boundaryHint = QTextToSpeech::BoundaryHint::Default);
    void resume();

    void setLocale(const QLocale &locale);

    void setRate(double rate);
    void setPitch(double pitch);
    void setVolume(double volume);
    void setVoice(const QVoice &voice);

Q_SIGNALS:
    void engineChanged(const QString &engine);
    void stateChanged(QTextToSpeech::State state);
    void errorOccurred(QTextToSpeech::ErrorReason error, const QString &errorString);
    void localeChanged(const QLocale &locale);
    void rateChanged(double rate);
    void pitchChanged(double pitch);
    void volumeChanged(double volume);
    void voiceChanged(const QVoice &voice);

    void sayingWord(const QString &word, qsizetype id, qsizetype start, qsizetype length);
    void aboutToSynthesize(qsizetype id);

protected:
    QList<QVoice> allVoices(const QLocale *locale) const;

private:
    template <typename Functor>
    using CompatibleCallbackTest2 = decltype(QtPrivate::makeCallableObject<void(*)(QAudioFormat, QByteArray)>(std::declval<Functor>()));
    template <typename Functor>
    using CompatibleCallbackTest1 = decltype(QtPrivate::makeCallableObject<void(*)(QAudioBuffer)>(std::declval<Functor>()));

    enum class SynthesizeOverload {
        AudioFormatByteArray,
        AudioBuffer
    };

    void synthesizeImpl(const QString &text,
                        QtPrivate::QSlotObjectBase *slotObj, const QObject *context,
                        SynthesizeOverload overload);

    // Helper type to find the index of a type in a tuple, which allows
    // us to generate a compile-time error if there are multiple criteria
    // of the same type.
    template <typename T, typename Tuple> struct LastIndexOf;
    template <typename T, typename ...Ts>
    struct LastIndexOf<T, std::tuple<Ts...>> {
        template <qsizetype... Is>
        static constexpr qsizetype lastIndexOf(std::integer_sequence<qsizetype, Is...>) {
            return std::max({(std::is_same_v<T, Ts> ? Is : -1)...});
        }
        static constexpr qsizetype value =
            lastIndexOf(std::make_integer_sequence<qsizetype, sizeof...(Ts)>{});
    };

    template <typename Arg0, typename ...Args>
    bool voiceMatches(const QVoice &voice, Arg0 &&arg0, Args &&...args) const {
        using ArgType = q20::remove_cvref_t<Arg0>;
        bool matches = [&]{
            if constexpr (std::is_same_v<ArgType, QLocale>) {
                return voice.locale() == arg0;
            } else if constexpr (std::is_same_v<ArgType, QLocale::Language>) {
                return voice.locale().language() == arg0;
            } else if constexpr (std::is_same_v<ArgType, QLocale::Territory>) {
                return voice.locale().territory() == arg0;
            } else if constexpr (std::is_same_v<ArgType, QVoice::Gender>) {
                return voice.gender() == arg0;
            } else if constexpr (std::is_same_v<ArgType, QVoice::Age>) {
                return voice.age() == arg0;
            } else if constexpr (std::disjunction_v<std::is_convertible<ArgType, QString>,
                                                    std::is_convertible<ArgType, QStringView>>) {
                return voice.name() == arg0;
            } else if constexpr (std::is_same_v<ArgType, QRegularExpression>) {
                return arg0.match(voice.name()).hasMatch();
            } else {
                static_assert(QtPrivate::type_dependent_false<Arg0>(),
                              "Type cannot be matched to a QVoice property!");
                return false;
            }
        }();
        if constexpr (sizeof...(args) > 0) {
            static_assert(LastIndexOf<ArgType, std::tuple<q20::remove_cvref_t<Args>...>>::value == -1,
                          "Using multiple criteria of the same type is not supported");
            matches = matches && voiceMatches(voice, args...);
        }
        return matches;
    }

    Q_DISABLE_COPY(QTextToSpeech)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QTextToSpeech::Capabilities)

QT_END_NAMESPACE

#endif
