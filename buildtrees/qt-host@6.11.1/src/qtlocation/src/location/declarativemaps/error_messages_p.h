// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef ERROR_MESSAGES_H
#define ERROR_MESSAGES_H

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


#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

extern const char CONTEXT_NAME[];
extern const char PLUGIN_PROPERTY_NOT_SET[];
extern const char PLUGIN_ERROR[];
extern const char PLUGIN_PROVIDER_ERROR[];
extern const char PLUGIN_NOT_VALID[];
extern const char CATEGORIES_NOT_INITIALIZED[];
extern const char UNABLE_TO_MAKE_REQUEST[];
extern const char INDEX_OUT_OF_RANGE[];

QT_END_NAMESPACE

#endif // ERROR_MESSAGES_H
