// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser
// ### de-inline more functions

#ifndef QTEXTCODEC_H
#define QTEXTCODEC_H

#include <QtCore/qlist.h>
#include <QtCore/qstringconverter.h>

#include <QtCore5Compat/qcore5global.h>

QT_REQUIRE_CONFIG(textcodec);

QT_BEGIN_NAMESPACE

class QTextDecoder;
class QTextEncoder;

class Q_CORE5COMPAT_EXPORT QTextCodec : public QStringConverterBase
{
    Q_DISABLE_COPY(QTextCodec)
public:
    using ConversionFlags = QStringConverterBase::Flags;
    using ConverterState = QStringConverterBase::State;

    static constexpr Flag ConvertInvalidToNull = Flag::ConvertInvalidToNull;
    static constexpr Flag DefaultConversion = Flag::WriteBom;
    static constexpr Flag IgnoreHeader = Flag::ConvertInitialBom;

    static QTextCodec* codecForName(const QByteArray &name);
    static QTextCodec* codecForName(const char *name) { return codecForName(QByteArray(name)); }
    static QTextCodec* codecForMib(int mib);

    static QList<QByteArray> availableCodecs();
    static QList<int> availableMibs();

    static QTextCodec* codecForLocale();
    static void setCodecForLocale(QTextCodec *c);

    static QTextCodec *codecForTr() { return codecForMib(106); /* Utf8 */ }

    static QTextCodec *codecForHtml(const QByteArray &ba);
    static QTextCodec *codecForHtml(const QByteArray &ba, QTextCodec *defaultCodec);

    static QTextCodec *codecForUtfText(const QByteArray &ba);
    static QTextCodec *codecForUtfText(const QByteArray &ba, QTextCodec *defaultCodec);

    bool canEncode(QChar) const;
    bool canEncode(const QString&) const;
    bool canEncode(QStringView) const;

    QString toUnicode(const QByteArray&) const;
    QString toUnicode(const char* chars) const;
    QByteArray fromUnicode(const QString& uc) const;
    QByteArray fromUnicode(QStringView uc) const;

    QString toUnicode(const char *in, int length, ConverterState *state = nullptr) const
    {
        if (in == nullptr)
            return {};
        if (length <= 0)
            return QLatin1String("");
        return convertToUnicode(in, length, state);
    }
    QByteArray fromUnicode(const QChar *in, int length, ConverterState *state = nullptr) const
    {
        if (in == nullptr)
            return {};
        if (length <= 0)
            return QByteArray("");
        return convertFromUnicode(in, length, state);
    }

    QTextDecoder* makeDecoder(ConversionFlags flags = DefaultConversion) const;
    QTextEncoder* makeEncoder(ConversionFlags flags = DefaultConversion) const;

    virtual QByteArray name() const = 0;
    virtual QList<QByteArray> aliases() const;
    virtual int mibEnum() const = 0;

protected:
    virtual QString convertToUnicode(const char *in, int length, ConverterState *state) const = 0;
    virtual QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const = 0;

    QTextCodec();
    virtual ~QTextCodec();

private:
    friend struct QTextCodecData;
};

class Q_CORE5COMPAT_EXPORT QTextEncoder
{
    Q_DISABLE_COPY(QTextEncoder)
public:
    explicit QTextEncoder(const QTextCodec *codec)
        : c(codec), state(QTextCodec::DefaultConversion) {}
    explicit QTextEncoder(const QTextCodec *codec, QTextCodec::ConversionFlags flags);
    ~QTextEncoder();
    QByteArray fromUnicode(const QString& str);
    QByteArray fromUnicode(QStringView str);
    QByteArray fromUnicode(const QChar *uc, int len);
    bool hasFailure() const;
private:
    const QTextCodec *c = nullptr;
    QTextCodec::ConverterState state;
};

class Q_CORE5COMPAT_EXPORT QTextDecoder
{
    Q_DISABLE_COPY(QTextDecoder)
public:
    explicit QTextDecoder(const QTextCodec *codec)
        : c(codec), state(QTextCodec::DefaultConversion) {}
    explicit QTextDecoder(const QTextCodec *codec, QTextCodec::ConversionFlags flags);
    ~QTextDecoder();
    QString toUnicode(const char* chars, int len);
    QString toUnicode(const QByteArray &ba);
    void toUnicode(QString *target, const char *chars, int len);
    bool hasFailure() const;
    bool needsMoreData() const;
private:
    const QTextCodec *c = nullptr;
    QTextCodec::ConverterState state;
};

namespace Qt
{
    Q_CORE5COMPAT_EXPORT QTextCodec *codecForHtml(const QByteArray &ba);
}

QT_END_NAMESPACE

#endif // QTEXTCODEC_H
