// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:dynamic-buffer-handling

#ifndef QPRIVATELINEARBUFFER_P_H
#define QPRIVATELINEARBUFFER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qtconfigmacros.h>

QT_BEGIN_NAMESPACE

// This is QIODevice's read buffer, optimised for read(), isEmpty() and getChar()
class QPrivateLinearBuffer
{
public:
    QPrivateLinearBuffer() : len(0), first(0), buf(0), capacity(0) {
    }
    ~QPrivateLinearBuffer() {
        delete [] buf;
    }
    void clear() {
        first = buf;
        len = 0;
    }
    qsizetype size() const
    {
        return len;
    }
    bool isEmpty() const {
        return len == 0;
    }
    void skip(qsizetype n)
    {
        if (n >= len) {
            clear();
        } else {
            len -= n;
            first += n;
        }
    }
    int getChar() {
        if (len == 0)
            return -1;
        int ch = uchar(*first);
        len--;
        first++;
        return ch;
    }
    qsizetype read(char* target, qsizetype size)
    {
        qsizetype r = (std::min)(size, len);
        memcpy(target, first, r);
        len -= r;
        first += r;
        return r;
    }
    char* reserve(qsizetype size)
    {
        makeSpace(size + len, freeSpaceAtEnd);
        char* writePtr = first + len;
        len += size;
        return writePtr;
    }
    void chop(qsizetype size)
    {
        if (size >= len) {
            clear();
        } else {
            len -= size;
        }
    }
    QByteArray readAll() {
        char* f = first;
        qsizetype l = len;
        clear();
        return QByteArray(f, l);
    }
    qsizetype readLine(char* target, qsizetype size)
    {
        qsizetype r = (std::min)(size, len);
        char* eol = static_cast<char*>(memchr(first, '\n', r));
        if (eol)
            r = 1+(eol-first);
        memcpy(target, first, r);
        len -= r;
        first += r;
        return r;
    }
    bool canReadLine() const {
        return memchr(first, '\n', len);
    }
    void ungetChar(char c) {
        if (first == buf) {
            // underflow, the existing valid data needs to move to the end of the (potentially bigger) buffer
            makeSpace(len+1, freeSpaceAtStart);
        }
        first--;
        len++;
        *first = c;
    }
    void ungetBlock(const char* block, qsizetype size)
    {
        if ((first - buf) < size) {
            // underflow, the existing valid data needs to move to the end of the (potentially bigger) buffer
            makeSpace(len + size, freeSpaceAtStart);
        }
        first -= size;
        len += size;
        memcpy(first, block, size);
    }

private:
    enum FreeSpacePos {freeSpaceAtStart, freeSpaceAtEnd};
    void makeSpace(size_t required, FreeSpacePos where) {
        size_t newCapacity = (std::max)(capacity, size_t(QPRIVATELINEARBUFFER_BUFFERSIZE));
        while (newCapacity < required)
            newCapacity *= 2;
        const qsizetype moveOffset = (where == freeSpaceAtEnd) ? 0 : qsizetype(newCapacity) - len;
        if (newCapacity > capacity) {
            // allocate more space
            char* newBuf = new char[newCapacity];
            memmove(newBuf + moveOffset, first, len);
            delete [] buf;
            buf = newBuf;
            capacity = newCapacity;
        } else {
            // shift any existing data to make space
            memmove(buf + moveOffset, first, len);
        }
        first = buf + moveOffset;
    }

private:
    // length of the unread data
    qsizetype len;
    // start of the unread data
    char* first;
    // the allocated buffer
    char* buf;
    // allocated buffer size
    size_t capacity;
};

QT_END_NAMESPACE

#endif // QPRIVATELINEARBUFFER_P_H
