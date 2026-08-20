// Copyright (C) 2017-2020 Ford Motor Company.
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:parsing-build-time-only

#ifndef QREGEXPARSER_H
#define QREGEXPARSER_H

#include <QtCore/qshareddata.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qvariant.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qmap.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

struct MatchCandidate {
    MatchCandidate(const QString &n, const QString &t, int i) : name(n), matchText(t), index(i) {}
    QString name;
    QString matchText;
    int index;
};

template <typename _Parser, typename _Table>
class QRegexParser: protected _Table
{
public:
    QRegexParser(int maxMatchLen=4096);
    virtual ~QRegexParser();

    virtual bool parse();

    virtual void reset() {}

    inline QVariant &sym(int index);

    void setBuffer(const QString &buffer);

    void setBufferFromDevice(QIODevice *device);

    void setDebug();

    QString errorString() const
    {
        return m_errorString;
    }

    void setErrorString(const QString &error)
    {
        m_errorString = error;
        qWarning() << m_errorString;
    }

    inline const QMap<QString, QString>& captured() const
    {
        return m_captured;
    }

    inline bool isDebug() const
    {
        return m_debug;
    }

    inline int lineNumber() const
    {
        return m_lineno;
    }

private:
    int nextToken();

    inline bool consumeRule(int rule)
    {
        return static_cast<_Parser*> (this)->consumeRule(rule);
    }

    enum { DefaultStackSize = 128 };

    struct Data: public QSharedData
    {
        Data(): stackSize (DefaultStackSize), tos (0) {}

        QVarLengthArray<int, DefaultStackSize> stateStack;
        QVarLengthArray<QVariant, DefaultStackSize> parseStack;
        int stackSize;
        int tos;

        void reallocateStack() {
            stackSize <<= 1;
            stateStack.resize(stackSize);
            parseStack.resize(stackSize);
        }
    };

    inline QString escapeString(QString s)
    {
        return s.replace(QLatin1Char('\n'), QLatin1String("\\n")).replace(QLatin1Char('\t'), QLatin1String("\\t"));
    }

    QSharedDataPointer<Data> d;

    QList<QRegularExpression> m_regexes;
    QMap<QChar, QList<int> > regexCandidates;
    QList<int> m_tokens;
    QString m_buffer, m_lastMatchText;
    int m_loc, m_lastNewlinePosition;
    int m_lineno;
    int m_debug;
    QStringList m_tokenNames;
    QMap<QString, QString> m_captured;
    int m_maxMatchLen;
    QString m_errorString;
    QList<QMap<int, QString>> m_names; //storage for match names
};

template <typename _Parser, typename _Table>
inline QVariant &QRegexParser<_Parser, _Table>::sym(int n)
{
    return d->parseStack [d->tos + n - 1];
}

template <typename _Parser, typename _Table>
QRegexParser<_Parser, _Table>::~QRegexParser()
{
}

template <typename _Parser, typename _Table>
bool QRegexParser<_Parser, _Table>::parse()
{
    m_errorString.clear();
    reset();
    const int INITIAL_STATE = 0;

    d->tos = 0;
    d->reallocateStack();

    int act = d->stateStack[++d->tos] = INITIAL_STATE;
    int token = -1;

    Q_FOREVER {
        if (token == -1 && - _Table::TERMINAL_COUNT != _Table::action_index[act])
            token = nextToken();

        act = _Table::t_action(act, token);

        if (d->stateStack[d->tos] == _Table::ACCEPT_STATE)
            return true;

        else if (act > 0) {
            if (++d->tos == d->stackSize)
                d->reallocateStack();

            d->parseStack[d->tos] = d->parseStack[d->tos - 1];
            d->stateStack[d->tos] = act;
            token = -1;
        }

        else if (act < 0) {
            int r = - act - 1;
            d->tos -= _Table::rhs[r];
            act = d->stateStack[d->tos++];
            if (!consumeRule(r))
                return false;
            act = d->stateStack[d->tos] = _Table::nt_action(act, _Table::lhs[r] - _Table::TERMINAL_COUNT);
        }

        else break;
    }

    setErrorString(QStringLiteral("Unknown token encountered"));
    return false;
}

template <typename _Parser, typename _Table>
QRegexParser<_Parser, _Table>::QRegexParser(int maxMatchLen) : d(new Data()), m_loc(0), m_lastNewlinePosition(0), m_lineno(1), m_debug(0), m_maxMatchLen(maxMatchLen)
{
    QRegularExpression re(QStringLiteral("\\[([_a-zA-Z][_0-9a-zA-Z]*)(,\\s*M)?\\](.+)$"));
    re.optimize();
    QMap<QString, int> token_lookup;
    QMap<int, QString> names;
    for (int i = 1; i < _Table::lhs[0]; i++) {
        const QString text = QLatin1String(_Table::spell[i]);
        names.clear();
        QRegularExpressionMatch match = re.match(text, 0, QRegularExpression::NormalMatch, QRegularExpression::DontCheckSubjectStringMatchOption);
        if (match.hasMatch()) {
            const QString token = match.captured(1);
            const bool multiline = match.captured(2).size() > 0;
            const QString pattern = match.captured(3);
            m_tokenNames.append(token);
            int index = i;
            if (token_lookup.contains(token))
                index = token_lookup[token];
            else
                token_lookup[token] = i;
            QRegularExpression pat(pattern);
            if (multiline)
                pat.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);

            if (!pat.isValid())
                qCritical() << "Pattern error for token #" << i << "for" << text << "pattern =" << pat << ":" << pat.errorString();
            else {
                pat.optimize();
                int counter = 0;
                const auto namedCaptureGroups = pat.namedCaptureGroups();
                for (const QString &name : namedCaptureGroups) {
                    if (!name.isEmpty())
                        names.insert(counter, name);
                    ++counter;
                }
                m_names.append(names);
                m_regexes.append(pat);
                if (token.startsWith(QLatin1String("ignore")))
                    m_tokens.append(-1);
                else
                    m_tokens.append(index);
            }
        } else {
            qCritical() << "Error parsing regex at token #" << i << "for" << text << "Invalid syntax";
        }
    }
}

template <typename _Parser, typename _Table>
void QRegexParser<_Parser, _Table>::setBuffer(const QString &buffer)
{
    m_buffer = buffer;
}

template <typename _Parser, typename _Table>
void QRegexParser<_Parser, _Table>::setBufferFromDevice(QIODevice *device)
{
    QTextStream in(device);
    m_buffer = in.readAll();
}

template <typename _Parser, typename _Table>
void QRegexParser<_Parser, _Table>::setDebug()
{
    m_debug = true;
    for (int r = 0; r < _Table::RULE_COUNT; ++r)
    {
        int ridx = _Table::rule_index[r];
        int _rhs = _Table::rhs[r];
        qDebug("%3d) %s ::=", r + 1, _Table::spell[_Table::rule_info[ridx]]);
        ++ridx;
        for (int i = ridx; i < ridx + _rhs; ++i)
        {
            int symbol = _Table::rule_info[i];
            if (symbol > 0 && symbol < _Table::lhs[0])
                qDebug("     token_%s (pattern = %s)",qPrintable(m_tokenNames[symbol-1]),qPrintable(m_regexes[symbol-1].pattern()));
            else if (const char *name = _Table::spell[symbol])
                qDebug("     %s", name);
            else
                qDebug("     #%d", symbol);
        }
        qDebug();
    }
}

template <typename _Parser, typename _Table>
int QRegexParser<_Parser, _Table>::nextToken()
{
    const QStringView buffer { m_buffer };
    static const QRegularExpression newline(QLatin1String("(\\n)"));
    int token = -1;
    while (token < 0)
    {
        if (m_loc == buffer.size())
            return _Table::EOF_SYMBOL;

        //Check m_lastMatchText for newlines and update m_lineno
        //This isn't necessary, but being able to provide the line # and character #
        //where the match is failing sure makes building/debugging grammars easier.
        QRegularExpressionMatchIterator  matches = newline.globalMatch(m_lastMatchText);
        while (matches.hasNext()) {
            m_lineno++;
            QRegularExpressionMatch match = matches.next();
            if (!matches.hasNext())
                m_lastNewlinePosition += match.capturedEnd();
        }

        if (m_debug) {
            qDebug();
            qDebug() << "nextToken loop, line =" << m_lineno
                << "line position =" << m_loc - m_lastNewlinePosition
                << "next 5 characters =" << escapeString(buffer.mid(m_loc, 5).toString());
        }
        int best = -1, maxLen = -1;
        QRegularExpressionMatch bestRegex;

        //Find the longest match.
        //If more than one are the same (longest) length, return the first one in
        //the order defined.
        QList<MatchCandidate> candidates;
        //We used PCRE's PartialMatch to eliminate most of the regexes by the first
        //character, so we keep a regexCandidates map with the list of possible regexes
        //based on initial characters found so far.
        const QChar nextChar = buffer.at(m_loc);
        //Populate the list if we haven't seeen this character before
        if (!regexCandidates.contains(nextChar)) {
            const QStringView tmp = buffer.mid(m_loc,1);
            int i = 0;
            regexCandidates[nextChar] = QList<int>();
            for (const QRegularExpression &re : std::as_const(m_regexes))
            {
                QRegularExpressionMatch match = re.matchView(tmp, 0, QRegularExpression::PartialPreferFirstMatch, QRegularExpression::DontCheckSubjectStringMatchOption);
                //qDebug() << nextChar << tmp << match.hasMatch() << match.hasPartialMatch() << re.pattern();
                if (match.hasMatch() || match.hasPartialMatch())
                    regexCandidates[nextChar] << i;
                i++;
            }
        }
        const auto indices = regexCandidates.value(nextChar);
        for (int i : indices)
        {
            //Seems like I should be able to run the regex on the entire string, but performance is horrible
            //unless I use a substring.
            //QRegularExpressionMatch match = m_regexes[i].match(m_buffer, m_loc, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
            QRegularExpressionMatch match = m_regexes.at(i).matchView(buffer.mid(m_loc, m_maxMatchLen), 0, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption | QRegularExpression::DontCheckSubjectStringMatchOption);
            if (match.hasMatch()) {
                if (m_debug)
                    candidates << MatchCandidate(m_tokenNames[i], match.captured(), i);
                if (match.capturedLength() > maxLen) {
                    best = i;
                    maxLen = match.capturedLength();
                    bestRegex = match;
                }
            }
        }
        if (best < 0) {
            setErrorString(QLatin1String("Error generating tokens from file, next characters >%1<").arg(buffer.mid(m_loc, 15)));
            return -1;
        } else {
            const QMap<int, QString> &map = m_names.at(best);
            if (!map.isEmpty())
                m_captured.clear();
            for (auto iter = map.cbegin(), end = map.cend(); iter != end; ++iter)
                m_captured.insert(iter.value(), bestRegex.captured(iter.key()));
            if (m_debug) {
                qDebug() << "Match candidates:";
                for (const MatchCandidate &m : std::as_const(candidates)) {
                    QLatin1String result = m.index == best ? QLatin1String(" * ") : QLatin1String("   ");
                    qDebug() << qPrintable(result) << qPrintable(m.name) << qPrintable(escapeString(m.matchText));
                }
            }
            m_loc += maxLen;
            if (m_tokens.at(best) >= 0)
                token = m_tokens.at(best);
            m_lastMatchText = bestRegex.captured(0);
        }
    }
    return token;
}

QT_END_NAMESPACE

#endif // QREGEXPARSER_H
