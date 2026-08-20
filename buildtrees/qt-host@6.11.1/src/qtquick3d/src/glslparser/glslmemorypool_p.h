// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_GLSLMEMORYPOOL_H
#define QSSG_GLSLMEMORYPOOL_H

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

#include <QtQuick3DGlslParser/private/glsl_p.h>

#include <cstddef>
#include <new>

QT_BEGIN_NAMESPACE

namespace GLSL {

class MemoryPool;

class Q_QUICK3DGLSLPARSER_EXPORT MemoryPool
{
    MemoryPool(const MemoryPool &other);
    void operator =(const MemoryPool &other);

public:
    MemoryPool();
    ~MemoryPool();

    void reset();

    inline void *allocate(size_t size)
    {
        size = (size + 7) & ~7;
        if (_ptr && (_ptr + size < _end)) {
            void *addr = _ptr;
            _ptr += size;
            return addr;
        }
        return allocate_helper(size);
    }

private:
    void *allocate_helper(size_t size);

private:
    char **_blocks;
    int _allocatedBlocks;
    int _blockCount;
    char *_ptr;
    char *_end;

    enum
    {
        BLOCK_SIZE = 8 * 1024,
        DEFAULT_BLOCK_COUNT = 8
    };
};

class Q_QUICK3DGLSLPARSER_EXPORT Managed
{
    Managed(const Managed &other);
    void operator = (const Managed &other);

public:
    Managed();
    virtual ~Managed();

    void *operator new(size_t size, MemoryPool *pool);
    void operator delete(void *);
    void operator delete(void *, MemoryPool *);
};

} // namespace GLSL

QT_END_NAMESPACE

#endif // QSSG_GLSLMEMORYPOOL_H
