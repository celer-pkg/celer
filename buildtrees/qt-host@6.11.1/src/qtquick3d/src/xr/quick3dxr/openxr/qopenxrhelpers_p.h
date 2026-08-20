// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QOPENXRHELPERS_H
#define QOPENXRHELPERS_H

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


#include <openxr/openxr.h>
#include <QtQuick3DXr/qtquick3dxrglobal.h>
#include <QQuaternion>
#include <QString>
#include <QVector3D>

QT_BEGIN_NAMESPACE

namespace OpenXRHelpers
{
QString getXrResultAsString(XrResult result, XrInstance instance);
bool checkXrResult(XrResult result, XrInstance instance);
bool resolveXrFunction(XrInstance instance, const char *name, PFN_xrVoidFunction *function);

inline QQuaternion toQQuaternion(const XrQuaternionf &q)
{
    return { q.w, q.x, q.y, q.z };
}

inline QVector3D toQVector(const XrVector3f &v)
{
    return { v.x * 100, v.y * 100, v.z * 100 };
}

/*!
 * \brief Safe call to OpenXR function
 * \param f - OpenXR function pointer that returns a XrResult
 * \param args - arguments for the function pointed to by f
 * \return a XrResult indicating the result of the function call.
 *
 * \note This function is used to "safely" call OpenXR functions without having to check if the function pointer is nullptr.
 *       If the function pointer is nullptr XR_ERROR_FUNCTION_UNSUPPORTED is returned.
 *
 * \note If the function returns XR_ERROR_FUNCTION_UNSUPPORTED the caller should avoid calling the function again.
 */

template <typename T>
[[nodiscard]] XrResult safeCall();
template <typename... A>
[[nodiscard]] XrResult safeCall(XrResult (XRAPI_PTR *f)(A...), A... args)
{
    return f ? f(args...) : XR_ERROR_FUNCTION_UNSUPPORTED;
}

}

QT_END_NAMESPACE

#endif // QOPENXRHELPERS_H
