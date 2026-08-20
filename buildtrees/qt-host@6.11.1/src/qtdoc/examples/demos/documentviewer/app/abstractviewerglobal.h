// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ABSTRACTVIEWERGLOBAL_H
#define ABSTRACTVIEWERGLOBAL_H

#if defined(QT_SHARED) || !defined(QT_STATIC)
#  if defined(BUILD_ABSTRACTVIEWER_LIB)
#    define ABSTRACTVIEWER_EXPORT Q_DECL_EXPORT
#  else
#    define ABSTRACTVIEWER_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define ABSTRACTVIEWER_EXPORT
#endif

#endif // ABSTRACTVIEWERGLOBAL_H
