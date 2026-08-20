// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#ifndef QT_STATIC
#  if defined(QT_BUILD_REPCLIB_LIB)
#    define Q_REPCLIB_EXPORT Q_DECL_EXPORT
#  else
#    define Q_REPCLIB_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_REPCLIB_EXPORT
#endif
