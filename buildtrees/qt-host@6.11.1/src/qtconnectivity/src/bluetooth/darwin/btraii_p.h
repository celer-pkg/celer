// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTRAII_P_H
#define BTRAII_P_H

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

#include <QtCore/private/qglobal_p.h>

#include <utility>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

enum class RetainPolicy
{
    noInitialRetain,
    doInitialRetain
};

// The class StrongReference and its descendant ScopedGuard
// are RAII classes dealing with raw pointers to NSObject class
// and its descendants (and thus hiding Objective-C's retain/
// release semantics). The header itself is meant to be included
// into *.cpp files so it's a pure C++ code without any Objective-C
// syntax. Thus it's a bit clunky - the type information is 'erased'
// and has to be enforced by the code using these smart pointers.
// That's because these types are Objective-C classes - thus require
// Objective-C compiler to work. Member-function template 'getAs' is
// a convenience shortcut giving the desired pointer type in
// Objective-C++ files (*.mm).
class StrongReference
{
public:
    StrongReference() = default;
    StrongReference(void *object, RetainPolicy policy);
    StrongReference(const StrongReference &other);
    StrongReference(StrongReference &&other);

    ~StrongReference();

    StrongReference &operator = (const StrongReference &other) noexcept;
    StrongReference &operator = (StrongReference &&other) noexcept;

    void swap(StrongReference &other) noexcept
    {
        std::swap(objCInstance, other.objCInstance);
    }

    void *release();

    void reset();
    void reset(void *newInstance, RetainPolicy policy);

    template<class ObjCType>
    ObjCType *getAs() const
    {
        return static_cast<ObjCType *>(objCInstance);
    }

    operator bool() const
    {
        return !!objCInstance;
    }

private:
    void *objCInstance = nullptr;
};

class ScopedPointer final : public StrongReference
{
public:
    ScopedPointer() = default;
    ScopedPointer(void *instance, RetainPolicy policy)
        : StrongReference(instance, policy)
    {
    }

private:
    Q_DISABLE_COPY_MOVE(ScopedPointer)
};

} // namespace DarwinBluetooth

QT_END_NAMESPACE

#endif // BTRAII_P_H
