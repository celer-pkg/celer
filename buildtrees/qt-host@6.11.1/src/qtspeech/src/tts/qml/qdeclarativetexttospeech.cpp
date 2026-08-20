// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativetexttospeech_p.h"
#include "qvoiceselectorattached_p.h"

#include <QtCore/qregularexpression.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class QDeclarativeTextToSpeech
    \internal
*/

/*!
    \internal

    Constructor that delays the initialization of the text-to-speech
    engine until the QML engine sets the engine.
*/
QDeclarativeTextToSpeech::QDeclarativeTextToSpeech(QObject *parent)
    : QTextToSpeech(u"none"_s, parent)
{}

/*!
    Intercept the calls to QTextToSpeech::engine/setEngine so that we can
    delay the setting of the engine until the component is completely parsed.
*/
QString QDeclarativeTextToSpeech::engine() const
{
    if (!m_engine.isEmpty())
        return m_engine;
    return QTextToSpeech::engine();
}

void QDeclarativeTextToSpeech::setEngine(const QString &engine)
{
    if (m_engine == engine)
        return;

    m_engine = engine;
    if (m_complete)
        QTextToSpeech::setEngine(m_engine, m_engineParameters);
    emit engineChanged(m_engine);
}

/*!
    \qmlproperty map TextToSpeech::engineParameters
    \brief This property holds engine-specific parameters.

    \sa engine
*/
QVariantMap QDeclarativeTextToSpeech::engineParameters() const
{
    return m_engineParameters;
}

void QDeclarativeTextToSpeech::setEngineParameters(const QVariantMap &parameters)
{
    if (m_engineParameters == parameters)
        return;

    m_engineParameters = parameters;
    // if changed after initialization, then we need to recreate the engine
    if (m_complete)
        QTextToSpeech::setEngine(QTextToSpeech::engine(), m_engineParameters);
    emit engineParametersChanged();
}

void QDeclarativeTextToSpeech::classBegin()
{
}

void QDeclarativeTextToSpeech::componentComplete()
{
    m_complete = true;
    QTextToSpeech::setEngine(m_engine, m_engineParameters);
    selectVoice();
}

void QDeclarativeTextToSpeech::selectVoice()
{
    if (!m_complete || !m_voiceSelector)
        return;

    if (state() != QTextToSpeech::Ready) {
        // for asynchronously initialized engines we have to wait for it to be ready
        connect(this, &QTextToSpeech::stateChanged, this, &QDeclarativeTextToSpeech::selectVoice,
                Qt::SingleShotConnection);
    } else {
        auto voices = findVoices(m_voiceSelector->selectionCriteria());
        if (!voices.isEmpty())
            setVoice(voices.first());
    }
}


QList<QVoice> QDeclarativeTextToSpeech::findVoices(const QVariantMap &criteria) const
{
    const QLocale *plocale = nullptr;
    // if we limit by locale, then limit the search to the voices for that
    if (const auto &it = criteria.find(QLatin1String("locale")); it != criteria.end()) {
        if (it->metaType() == QMetaType::fromType<QLocale>())
            plocale = static_cast<const QLocale *>(it->constData());
    }
    QList<QVoice> voices = allVoices(plocale);

    voices.removeIf([&criteria](const QVoice &voice){
        const QMetaObject &mo = QVoice::staticMetaObject;
        for (const auto &[key, value] : criteria.asKeyValueRange()) {
            const int propertyIndex = mo.indexOfProperty(key.toUtf8().constData());
            if (propertyIndex < 0) {
                qWarning("QVoice doesn't have a property %s!", qPrintable(key));
            } else {
                const QMetaProperty prop = mo.property(propertyIndex);
                const QVariant voiceValue = prop.readOnGadget(&voice);
                if (voiceValue.metaType() == QMetaType::fromType<QLocale::Language>()) {
                    if (voiceValue.value<QLocale::Language>() != value.toLocale().language())
                        return true;
                } else if (value.metaType() == QMetaType::fromType<QRegularExpression>()) {
                    if (!value.value<QRegularExpression>().match(voiceValue.toString()).hasMatch())
                        return true;
                } else if (voiceValue != value) {
                    return true;
                }
            }
        }
        return false;
    });

    return voices;
}


QT_END_NAMESPACE
