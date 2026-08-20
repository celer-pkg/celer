// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qshaderrewriter_p.h"
#include <QDebug>

QT_BEGIN_NAMESPACE

namespace QShaderRewriter {

struct Tokenizer {

    enum Token {
        Token_Void,
        Token_OpenBrace,
        Token_CloseBrace,
        Token_SemiColon,
        Token_Identifier,
        Token_Unspecified,

        Token_EOF
    };

    static const char *NAMES[];

    void initialize(const QByteArray &input);
    Token next();

    const char *pos;
    const char *identifier;
};

const char *Tokenizer::NAMES[] = {
    "Void",
    "OpenBrace",
    "CloseBrace",
    "SemiColon",
    "Identifier",
    "Unspecified",
    "EOF"
};

void Tokenizer::initialize(const QByteArray &input)
{
    pos = input.constData();
    identifier = pos;
}

Tokenizer::Token Tokenizer::next()
{
    while (*pos != 0) {
        char c = *pos++;
        switch (c) {
        case 0:
            return Token_EOF;

        case '/':
            if (*pos == '/') {
                // '//' comment
                ++pos;
                while (*pos != 0 && *pos != '\n') ++pos;
                if (*pos != 0) ++pos; // skip the newline

            } else if (*pos == '*') {
                // /* */ comment
                ++pos;
                while (*pos != 0 && *pos != '*' && pos[1] != '/') ++pos;
                if (*pos != 0) pos += 2;
            }
            break;

        case '#': {
            while (*pos != 0) {
                if (*pos == '\n') {
                    ++pos;
                    break;
                } else if (*pos == '\\') {
                    ++pos;
                    while (*pos != 0 && (*pos == ' ' || *pos == '\t'))
                        ++pos;
                    if (*pos != 0 && (*pos == '\n' || (*pos == '\r' && pos[1] == '\n')))
                        pos+=2;
                } else {
                    ++pos;
                }
            }
            break;
        }

        case ';': return Token_SemiColon;
        case '{': return Token_OpenBrace;
        case '}': return Token_CloseBrace;

        case ' ':
        case '\n':
        case '\r': break;

        case 'v': {
            if (*pos == 'o' && pos[1] == 'i' && pos[2] == 'd') {
                pos += 3;
                return Token_Void;
            }
            Q_FALLTHROUGH();
        }
        default:
            // Identifier...
            if ((c >= 'a' && c <= 'z' ) || (c >= 'A' && c <= 'Z' ) || c == '_') {
                identifier = pos - 1;
                while (*pos != 0 && ((*pos >= 'a' && *pos <= 'z')
                                     || (*pos >= 'A' && *pos <= 'Z')
                                     || *pos == '_'
                                     || (*pos >= '0' && *pos <= '9'))) {
                    ++pos;
                }
                return Token_Identifier;
            } else {
                return Token_Unspecified;
            }
        }
    }

    return Token_EOF;
}

void debugTokenizer(const QByteArray &input)
{
    Tokenizer tok;
    tok.initialize(input);
    Tokenizer::Token t = tok.next();
    while (t != Tokenizer::Token_EOF) {
        if (t == Tokenizer::Token_Identifier)
            qDebug() << Tokenizer::NAMES[t] << QByteArray::fromRawData(tok.identifier, tok.pos - tok.identifier);
        else
            qDebug() << Tokenizer::NAMES[t];

        t = tok.next();
    }
}

QByteArray addZAdjustment(const QByteArray &input, int vertexInputLocation)
{
    Tokenizer tok;
    tok.initialize(input);

    Tokenizer::Token lt = tok.next();
    Tokenizer::Token t = tok.next();

    // First find "void main() { ... "
    const char* voidPos = input.constData();
    while (t != Tokenizer::Token_EOF) {
        if (lt == Tokenizer::Token_Void && t == Tokenizer::Token_Identifier) {
            if (qstrncmp("main", tok.identifier, 4) == 0)
                break;
        }
        voidPos = tok.pos - 4;
        lt = t;
        t = tok.next();
    }

    QByteArray result;
    result.reserve(1024);
    result += QByteArray::fromRawData(input.constData(), voidPos - input.constData());

    result += QByteArrayLiteral("layout(location = ");
    result += QByteArray::number(vertexInputLocation);
    result += QByteArrayLiteral(") in float _qt_order;\n");

    // Find first brace '{'
    while (t != Tokenizer::Token_EOF && t != Tokenizer::Token_OpenBrace) t = tok.next();
    int braceDepth = 1;
    t = tok.next();

    // Find matching brace and insert our code there...
    while (t != Tokenizer::Token_EOF) {
        switch (t) {
        case Tokenizer::Token_CloseBrace:
            braceDepth--;
            if (braceDepth == 0) {
                result += QByteArray::fromRawData(voidPos, tok.pos - 1 - voidPos);
                result += QByteArrayLiteral("    gl_Position.z = _qt_order * gl_Position.w;\n");
                result += QByteArray(tok.pos - 1);
                return result;
            }
            break;
        case Tokenizer::Token_OpenBrace:
            ++braceDepth;
            break;
        default:
            break;
        }
        t = tok.next();
    }
    return QByteArray();
}

} // namespace

QT_END_NAMESPACE
