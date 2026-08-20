// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qvoice.h"
#include "qvoice_p.h"
#include "qtexttospeech.h"

QT_BEGIN_NAMESPACE

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QVoicePrivate)

/*!
    \class QVoice
    \brief The QVoice class represents a particular voice.
    \inmodule QtTextToSpeech

    To get a voice that is supported by the current text-to-speech engine,
    use \l QTextToSpeech::availableVoices() or \l QTextToSpeech::findVoices().
*/

/*!
    \qmlvaluetype voice
    \inqmlmodule QtTextToSpeech
    \brief The voice type represents a particular voice.

    To get a voice that is supported by the current text-to-speech engine,
    use \l TextToSpeech::availableVoices().
*/

/*!
    \enum QVoice::Age

    The age of a voice.

    \value Child    Voice of a child
    \value Teenager Voice of a teenager
    \value Adult    Voice of an adult
    \value Senior   Voice of a senior
    \value Other    Voice of unknown age
*/

/*!
    \enum QVoice::Gender

    The gender of a voice.

    \value Male    Voice of a male
    \value Female  Voice of a female
    \value Unknown Voice of unknown gender
*/

/*!
    Constructs an empty QVoice.

    Application code cannot construct arbitrary voice instances.
    Use \l{QTextToSpeech::availableVoices()} or \l{QTextToSpeech::findVoices()}
    instead to select a supported voice.
*/
QVoice::QVoice()
    : d(nullptr)
{
}

/*!
    Copy-constructs a QVoice from \a other.
*/
QVoice::QVoice(const QVoice &other) noexcept
    : d(other.d)
{}

/*!
    Destroys the QVoice instance.
*/
QVoice::~QVoice()
{}

/*!
    \fn QVoice::QVoice(QVoice &&other)

    Constructs a QVoice object by moving from \a other.
*/

/*!
    \fn QVoice &QVoice::operator=(QVoice &&other)
    Moves \a other into this QVoice object.
*/

/*!
    Assigns \a other to this QVoice object.
*/
QVoice &QVoice::operator=(const QVoice &other) noexcept
{
    d = other.d;
    return *this;
}

/*!
    \internal
*/
QVoice::QVoice(const QString &name, const QLocale &locale, Gender gender,
               Age age, const QVariant &data)
    :d(new QVoicePrivate(name, locale, gender, age, data))
{
}


/*!
    \internal
    Compares all attributes of this voice with \a other.
    Returns \c true if all of them match.
*/
bool QVoice::isEqual(const QVoice &other) const noexcept
{
    if (d == other.d)
        return true;
    if (!d || !other.d)
        return false;

    return d->data == other.d->data
        && d->name == other.d->name
        && d->locale == other.d->locale
        && d->gender == other.d->gender
        && d->age == other.d->age;
}

/*!
    \fn void QVoice::swap(QVoice &other) noexcept
    \since 6.4

    Swaps \a other with this voice. This operation is very fast and never fails.
*/

/*!
    \fn bool QVoice::operator==(const QVoice &lhs, const QVoice &rhs)
    \return whether the \a lhs voice and the \a rhs voice are identical.

    Two voices are identical if \l name, \l locale, \l gender, and \l age
    are identical, and if they belong to the same text-to-speech engine.
*/

/*!
    \fn bool QVoice::operator!=(const QVoice &lhs, const QVoice &rhs)
    \return whether the \a lhs voice and the \a rhs voice are different.
*/

/*!
    \fn QDataStream &QVoice::operator<<(QDataStream &stream, const QVoice &voice)
    \since 6.4

    Serializes \a voice to data stream \a stream.

    \sa {Serializing Qt Data Types}
*/

/*!
    \fn QDataStream &QVoice::operator>>(QDataStream &stream, QVoice &voice)
    \since 6.4

    Deserializes \a voice from data stream \a stream.

    \sa {Serializing Qt Data Types}
*/

/*!
    \qmlproperty string voice::name
    \brief This property holds the name of the voice.
*/

/*!
    \property QVoice::name
    \brief the name of a voice
*/
QString QVoice::name() const
{
    return d ? d->name : QString();
}

/*!
    \qmlproperty enumerator voice::language
    \brief This property holds the language of the voice.
    \since 6.6

    This is the \l{QLocale::}{language} attribute of the voice's \l locale.
*/

/*!
    \property QVoice::language
    \brief the language of the voice
    \since 6.6

    This is the \l{QLocale::}{language} attribute of the voice's \l locale.
*/

/*!
    \qmlproperty locale voice::locale
    \brief This property holds the locale of the voice.

    The locale includes the language and the territory (i.e. accent or dialect)
    of the voice.

    \a language
*/

/*!
    \property QVoice::locale
    \brief the locale of the voice
    \since 6.4

    The locale includes the language and the territory (i.e. accent or dialect)
    of the voice.
*/
QLocale QVoice::locale() const
{
    return d ? d->locale : QLocale();
}

/*!
    \qmlproperty enumeration voice::gender
    \brief This property holds the gender of the voice.

    \sa QVoice::Gender
*/

/*!
    \property QVoice::gender
    \brief the gender of a voice
*/
QVoice::Gender QVoice::gender() const
{
    return d ? d->gender : QVoice::Unknown;
}

/*!
    \qmlproperty enumeration voice::age
    \brief This property holds the age of the voice.

    \sa QVoice::Age
*/

/*!
    \property QVoice::age
    \brief the age of a voice
*/
QVoice::Age QVoice::age() const
{
    return d ? d->age : QVoice::Other;
}

/*!
    \internal
*/
QVariant QVoice::data() const
{
    return d ? d->data : QVariant();
}

/*!̈́
    Returns the \a gender name of a voice.
*/
QString QVoice::genderName(QVoice::Gender gender)
{
    QString retval;
    switch (gender) {
    case QVoice::Male:
        retval = QTextToSpeech::tr("Male", "Gender of a voice");
        break;
    case QVoice::Female:
        retval = QTextToSpeech::tr("Female", "Gender of a voice");
        break;
    case QVoice::Unknown:
        retval = QTextToSpeech::tr("Unknown Gender", "Voice gender is unknown");
        break;
    }
    return retval;
}

/*!
    Returns a string representing the \a age class of a voice.
*/
QString QVoice::ageName(QVoice::Age age)
{
    QString retval;
    switch (age) {
    case QVoice::Child:
        retval = QTextToSpeech::tr("Child", "Age of a voice");
        break;
    case QVoice::Teenager:
        retval = QTextToSpeech::tr("Teenager", "Age of a voice");
        break;
    case QVoice::Adult:
        retval = QTextToSpeech::tr("Adult", "Age of a voice");
        break;
    case QVoice::Senior:
        retval = QTextToSpeech::tr("Senior", "Age of a voice");
        break;
    case QVoice::Other:
        retval = QTextToSpeech::tr("Other Age", "Unknown age of a voice");
        break;
    }
    return retval;
}

#ifndef QT_NO_DATASTREAM
QDataStream &QVoice::writeTo(QDataStream &stream) const
{
    stream << name() << locale() << int(gender()) << int(age()) << data();
    return stream;
}

QDataStream &QVoice::readFrom(QDataStream &stream)
{
    if (!d)
        d.reset(new QVoicePrivate);

    int g, a;
    stream >> d->name >> d->locale >> g >> a >> d->data;
    d->gender = Gender(g);
    d->age = Age(a);
    return stream;
}
#endif

#ifndef QT_NO_DEBUG_STREAM

/*!
   \fn QDebug QVoice::operator<<(QDebug debug, const QVoice &voice)
   \since 6.4

    Writes information about \a voice to the \a debug stream.

   \sa QDebug
 */
QDebug operator<<(QDebug dbg, const QVoice &voice)
{
    QDebugStateSaver state(dbg);
    dbg.noquote().nospace();
    dbg << "QVoice(name: " << voice.name()
             << ", locale: " << voice.locale()
             << ", gender: " << QVoice::genderName(voice.gender())
             << ", age: " << QVoice::ageName(voice.age())
             << "; data: " << voice.data()
        << ")";
    return dbg;
}
#endif

QT_END_NAMESPACE
