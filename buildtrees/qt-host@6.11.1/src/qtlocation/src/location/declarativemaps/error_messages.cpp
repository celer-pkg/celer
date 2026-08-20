// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#include "error_messages_p.h"

QT_BEGIN_NAMESPACE

const char CONTEXT_NAME[] = "QtLocationQML";

//to-be-translated error string

const char PLUGIN_PROPERTY_NOT_SET[] = QT_TRANSLATE_NOOP("QtLocationQML", "Plugin property is not set.");
const char PLUGIN_ERROR[] = QT_TRANSLATE_NOOP("QtLocationQML", "Plugin Error (%1): %2");
const char PLUGIN_PROVIDER_ERROR[] = QT_TRANSLATE_NOOP("QtLocationQML", "Plugin Error (%1): Could not instantiate provider");
const char PLUGIN_NOT_VALID[] = QT_TRANSLATE_NOOP("QtLocationQML", "Plugin is not valid");
const char CATEGORIES_NOT_INITIALIZED[] = QT_TRANSLATE_NOOP("QtLocationQML", "Unable to initialize categories");
const char UNABLE_TO_MAKE_REQUEST[] = QT_TRANSLATE_NOOP("QtLocationQML", "Unable to create request");
const char INDEX_OUT_OF_RANGE[] = QT_TRANSLATE_NOOP("QtLocationQML", "Index '%1' out of range");

QT_END_NAMESPACE
