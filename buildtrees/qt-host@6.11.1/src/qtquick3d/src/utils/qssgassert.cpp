// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgassert_p.h"

QT_BEGIN_NAMESPACE

/*!
    Collection of assert checks that causes a soft or hard assert depending on the build. Unlike Q_ASSERT(),
    which is a no-op for non-debug build, QSSG_ASSERT() etc., will print a warning in non-developer builds (soft assert)
    or terminate on developer-build (hard assert).

    \macro QSSG_ASSERT(condition, action)
    \internal

    The assert will be fatal in developer builds if \a condition is not met. In non-developer builds
    the assert is "soft" and will instead print a warning with the reason and location of the assert
    before execution \a action. The \a action can be for example be: \c return, \c break or \c continue.

    For example, writing:

    \badcode
    QSSG_ASSERT(ptr != nullptr, return);
    \endcode

    other actions are of course possible, e.g., in a loop it might be better to do:

    \badcode
    QSSG_ASSERT(ptr != nullptr, continue);
    \endcode

    is the equivalent to:

    \badcode
    Q_ASSERT(ptr != nullptr);
    if (ptr != nullptr) {
        qWarning() << "Something unexpected here, proceeding will be fatal!";
        return;
    }
    \endcode

    \sa QSSG_ASSERT_X
*/

/*!
    \macro QSSG_ASSERT_X(condition, message, action)
    \internal

    Same as \l QSSG_ASSERT() but with a custom \a message that will be print if \a condition is not met.
*/

/*!
    \macro QSSG_CHECK(condition)
    \internal

    Similar to \l QSSG_ASSERT but without an action. Convenient when the \a condition is expected to be valid,
    but it's not immediately fatal if the current code path continues.

    \badcode
    QSSG_CHECK(ptr != nullptr);
    \endcode

    is the equivalent to:

    \badcode
    Q_ASSERT(ptr != nullptr);
    if (ptr != nullptr)
       qWarning() << "Something unexpected here, will probably not work as expected!";
    \endcode

    \sa QSSG_CHECK_X
*/

/*!
    \macro QSSG_CHECK_X(condition, message)
    \internal

    Same as \l QSSG_CHECK() but with a custom \a message that will be print if \a condition is not met.
*/

/*!
    \macro QSSG_GUARD(condition)
    \internal

    Check that returns the result of \a condition. As with the other assert functions, a call to QSSG_GUARD, when \a condition
    is not met, is fatal for developer builds.

    \badcode

    if (QSSG_GUARD(ptr != nullptr)) {
       ... // OK
    } else {
       ... // We shouldn't be here!
    }

    \endcode

    is the equivalent to:

    \badcode
    if (ptr != nullptr) {
        ... // OK
    } else {
        Q_ASSERT(ptr != nullptr);
        qWarning() << "Something unexpected here!";
    }
    \endcode

    \sa QSSG_GUARD_X
*/

/*!
    \macro QSSG_GUARD_X(condition, message)
    \internal

    Same as \l QSSG_GUARD() but with a custom \a message that will be print if \a condition is not met.
*/

/*!
    \macro QSSG_DEBUG_COND(condition)
    \internal

    Macro for condition that should only be run in debug builds. In releases build the macro
    produces an "almost-no-op" condition (always true) and the \a condition is never run.
    Can e.g., be combined with the assert checks to add potentially expensive sanity checks
    that should only be run in debug builds.

    \badcode
    QSSG_CEHCK(QSSG_DEBUG_COND(!list.contains(...)));
    \endcode

    In a release build the \c QSSG_DEBUG_COND will never return \c false and the \a condition will never
    be evaluated.

    \note DO NOT make surrounding code depend on the \a condition being evaluated or called!.

    \note Unlike the assert checks, this macro does not change behavior in relation to developer-builds.
*/

void qssgWriteAssertLocation(const char *msg)
{
#if defined(QT_BUILD_INTERNAL)
    qFatal("ASSERT: %s", msg);
#else
    qWarning("Unexpected condition met: %s", msg);
#endif
}

QT_END_NAMESPACE
