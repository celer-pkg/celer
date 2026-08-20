// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QVOICE_H
#define QVOICE_H

#include <QtTextToSpeech/qtexttospeech_global.h>
#include <QtCore/qlocale.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QVoicePrivate;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QVoicePrivate, Q_TEXTTOSPEECH_EXPORT)

class Q_TEXTTOSPEECH_EXPORT QVoice
{
    Q_GADGET
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(Gender gender READ gender CONSTANT)
    Q_PROPERTY(Age age READ age CONSTANT)
    Q_PROPERTY(QLocale locale READ locale CONSTANT)
    Q_PROPERTY(QLocale::Language language READ language STORED false REVISION(6, 6))

public:
    enum Gender {
        Male,
        Female,
        Unknown
    };
    Q_ENUM(Gender)

    enum Age {
        Child,
        Teenager,
        Adult,
        Senior,
        Other
    };
    Q_ENUM(Age)

    QVoice();
    ~QVoice();
    QVoice(const QVoice &other) noexcept;
    QVoice &operator=(const QVoice &other) noexcept;
    QVoice(QVoice &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QVoice)

    void swap(QVoice &other) noexcept
    { d.swap(other.d); }

    friend inline bool operator==(const QVoice &lhs, const QVoice &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QVoice &lhs, const QVoice &rhs) noexcept
    { return !lhs.isEqual(rhs); }

#ifndef QT_NO_DATASTREAM
    friend inline QDataStream &operator<<(QDataStream &str, const QVoice &voice)
    { return voice.writeTo(str); }
    friend inline QDataStream &operator>>(QDataStream &str, QVoice &voice)
    { return voice.readFrom(str); }
#endif

    QString name() const;
    QLocale locale() const;
    Gender gender() const;
    Age age() const;

    QLocale::Language language() const { return locale().language(); }

    static QString genderName(QVoice::Gender gender);
    static QString ageName(QVoice::Age age);

private:
    QVoice(const QString &name, const QLocale &loc, Gender gender, Age age, const QVariant &data);
    bool isEqual(const QVoice &other) const noexcept;
#ifndef QT_NO_DATASTREAM
    QDataStream &writeTo(QDataStream &) const;
    QDataStream &readFrom(QDataStream &);
#endif

    QVariant data() const;

    QExplicitlySharedDataPointer<QVoicePrivate> d;
    friend class QVoicePrivate;
    friend class QTextToSpeechEngine;
    friend Q_TEXTTOSPEECH_EXPORT QDebug operator<<(QDebug, const QVoice &);
};

#ifndef QT_NO_DEBUG_STREAM
Q_TEXTTOSPEECH_EXPORT QDebug operator<<(QDebug, const QVoice &);
#endif

Q_DECLARE_SHARED(QVoice)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QVoice)
Q_DECLARE_METATYPE(QVoice::Age)
Q_DECLARE_METATYPE(QVoice::Gender)

#endif
