// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:insignificant

#ifndef QSCXMLGLOBALS_H
#define QSCXMLGLOBALS_H
#include <QtCore/qglobal.h>

#if defined(BUILD_QSCXMLC)
#  define Q_SCXML_EXPORT
#else
#  include <QtScxml/qtscxmlexports.h>
#endif

// Silence syncqt
QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

#endif // QSCXMLGLOBALS_H

