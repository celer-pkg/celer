// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [1]
void *operator new[](size_t size)
{
    return malloc(size);
}

void *operator new(size_t size)
{
    return malloc(size);
}

void operator delete[](void *ptr)
{
    free(ptr);
}

void operator delete[](void *ptr, size_t)
{
    free(ptr);
}

void operator delete(void *ptr)
{
    free(ptr);
}

void operator delete(void *ptr, size_t)
{
    free(ptr);
}
//! [1]
