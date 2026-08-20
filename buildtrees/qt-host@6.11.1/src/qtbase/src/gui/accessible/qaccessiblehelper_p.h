// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QACCESSIBLEHELPER_P_H
#define QACCESSIBLEHELPER_P_H

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

#include <QtCore/QString>
#include <QtGui/qaccessible.h>
#include <QtGui/qtguiglobal.h>

class QTextCursor;

QT_BEGIN_NAMESPACE

Q_GUI_EXPORT QString qt_accStripAmp(const QString &text);

Q_GUI_EXPORT QString qt_accTextBeforeOffsetHelper(const QAccessibleTextInterface &textInterface,
                                                  const QTextCursor &textCursor, int offset,
                                                  QAccessible::TextBoundaryType boundaryType,
                                                  int *startOffset, int *endOffset);

Q_GUI_EXPORT QString qt_accTextAfterOffsetHelper(const QAccessibleTextInterface &textInterface,
                                                 const QTextCursor &textCursor, int offset,
                                                 QAccessible::TextBoundaryType boundaryType,
                                                 int *startOffset, int *endOffset);

Q_GUI_EXPORT QString qt_accTextAtOffsetHelper(const QAccessibleTextInterface &textInterface,
                                              const QTextCursor &textCursor, int offset,
                                              QAccessible::TextBoundaryType boundaryType,
                                              int *startOffset, int *endOffset);

QT_END_NAMESPACE

#endif // QACCESSIBLEHELPER_P_H
