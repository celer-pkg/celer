// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qvoiceselectorattached_p.h"
#include "qdeclarativetexttospeech_p.h"

#include <QtQml/qqmlinfo.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \qmltype VoiceSelector
    \inqmlmodule QtTextToSpeech
    \since 6.6
    \brief Provides attached properties for selecting the voice of a TextToSpeech
    element.

    The type provides a set of properties that are by default unset. The TextToSpeech
    element will choose the first voice that matches all the set property values.
    If no voice is found that matches all criteria, then the voice doesn't change.

    When setting individual properties within this group after the TextToSpeech object
    has been initialized, then you have to call the select() method to trigger the
    selection of a voice.

    \sa TextToSpeech::voice, TextToSpeech::availableVoices()
*/

QVoiceSelectorAttached *QVoiceSelectorAttached::qmlAttachedProperties(QObject *obj)
{
    QVoiceSelectorAttached *attached = nullptr;
    if (QDeclarativeTextToSpeech *tts = qobject_cast<QDeclarativeTextToSpeech *>(obj)) {
        Q_ASSERT(!tts->m_voiceSelector);
        attached = new QVoiceSelectorAttached(tts);
        tts->m_voiceSelector = attached;
    } else {
        qCritical("A VoiceSelector can only be attached to a TextToSpeech element!");
    }
    return attached;
}

QVoiceSelectorAttached::QVoiceSelectorAttached(QDeclarativeTextToSpeech *tts)
    : QObject(tts), m_tts(tts)
{}

/*!
    \qmlmethod void VoiceSelector::select()

    Activates the selection of the voice based on the specified criteria.

    \note This method only needs to be called if the selection criteria are
    modified after the TextToSpeech object has been instantiated.
*/
void QVoiceSelectorAttached::select()
{
    m_tts->selectVoice();
}

/*!
    \qmlproperty variant VoiceSelector::name
    \brief This property specifies which name the selected voice should have.

    The property can be a string, or a regular expression.
*/

QVariant QVoiceSelectorAttached::name() const
{
    return m_criteria.value(u"name"_s);
}

void QVoiceSelectorAttached::setName(const QVariant &name)
{
    if (!name.isValid()) {
        m_criteria.remove(u"name"_s);
        return;
    }

    QVariant &m_name = m_criteria[u"name"_s];
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged();
}

/*!
    \qmlproperty enumerator VoiceSelector::gender
    \brief This property specifies which \l{QVoice::Gender}{gender} the selected
           voice should have.
*/

QVoice::Gender QVoiceSelectorAttached::gender() const
{
    return m_criteria.value(u"gender"_s).value<QVoice::Gender>();
}

void QVoiceSelectorAttached::setGender(QVoice::Gender gender)
{
    QVariant &m_gender = m_criteria[u"gender"_s];
    if (m_gender == gender)
        return;

    m_gender = gender;
    emit genderChanged();
}

/*!
    \qmlproperty enumerator VoiceSelector::age
    \brief This property specifies which \l{QVoice::Age}{age} the selected voice
           should have.
*/

QVoice::Age QVoiceSelectorAttached::age() const
{
    return m_criteria.value(u"age"_s).value<QVoice::Age>();
}

void QVoiceSelectorAttached::setAge(QVoice::Age age)
{
    QVariant &m_age = m_criteria[u"age"_s];
    if (m_age == age)
        return;
    m_age = age;
    emit ageChanged();
}

/*!
    \qmlproperty locale VoiceSelector::locale
    \brief This property specifies which locale the selected voice should have.

    If this property is set, then both the language and the territory of the
    voice need to match.

    \sa language
*/
QLocale QVoiceSelectorAttached::locale() const
{
    return m_criteria.value(u"locale"_s).toLocale();
}

void QVoiceSelectorAttached::setLocale(const QLocale &locale)
{
    QVariant &m_locale = m_criteria[u"locale"_s];
    if (m_locale == locale)
        return;

    m_locale = locale;
    emit localeChanged();
}

/*!
    \qmlproperty locale VoiceSelector::language
    \brief This property specifies which language the selected voice should have.

    The property is of type locale, but only the language component of the locale
    will be considered, the territory will be ignored.

    \sa locale
*/
QLocale QVoiceSelectorAttached::language() const
{
    const auto &it = m_criteria.find(u"language"_s);
    if (it == m_criteria.end())
        return locale();
    return (*it).value<QLocale>();
}

void QVoiceSelectorAttached::setLanguage(const QLocale &language)
{
    QVariant &m_language = m_criteria[u"language"_s];
    if (m_language == language)
        return;

    m_language = language;
    emit languageChanged();
}

QT_END_NAMESPACE
