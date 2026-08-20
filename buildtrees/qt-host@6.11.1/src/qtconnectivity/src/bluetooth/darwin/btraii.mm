// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "btraii_p.h"

#include <qdebug.h>

#include <Foundation/Foundation.h>

#include <utility>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

StrongReference::StrongReference(void *object, RetainPolicy policy)
    : objCInstance(object)
{
    if (objCInstance && policy == RetainPolicy::doInitialRetain)
        objCInstance = [getAs<NSObject>() retain];
}

StrongReference::StrongReference(const StrongReference &other)
{
    if ((objCInstance = other.getAs<NSObject>()))
        objCInstance = [other.getAs<NSObject>() retain];
}

StrongReference::StrongReference(StrongReference &&other)
{
    std::swap(objCInstance, other.objCInstance);
}

StrongReference::~StrongReference()
{
    [getAs<NSObject>() release];
}

StrongReference &StrongReference::operator = (const StrongReference &other) noexcept
{
    if (this != &other) {
        [getAs<NSObject>() release];
        objCInstance = [other.getAs<NSObject>() retain];
    }

    return *this;
}

StrongReference &StrongReference::operator = (StrongReference &&other) noexcept
{
    swap(other);

    return *this;
}

void *StrongReference::release()
{
    void *released = objCInstance;
    objCInstance = nullptr;

    return released;
}

void StrongReference::reset()
{
    [getAs<NSObject>() release];
    objCInstance = nullptr;
}

void StrongReference::reset(void *obj, RetainPolicy policy)
{
    [getAs<NSObject>() release];
    objCInstance = obj;

    if (objCInstance && policy == RetainPolicy::doInitialRetain)
        objCInstance = [getAs<NSObject>() retain];
}

} // namespace DarwinBluetooth

QT_END_NAMESPACE
