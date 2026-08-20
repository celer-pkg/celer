// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant reason:default

#include "qquickliteralbindingcheck_p.h"
#include "qquickvaluetypefromstringcheck_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QQmlJSStructuredTypeError QQuickLiteralBindingCheck::check(const QString &typeName,
                                                           const QString &value) const
{
    return QQuickValueTypeFromStringCheck::check(typeName, value);
}

void QQuickLiteralBindingCheck::onBinding(
        const QQmlSA::Element &element, const QString &propertyName, const QQmlSA::Binding &binding,
        const QQmlSA::Element &bindingScope, const QQmlSA::Element &value)
{
    Q_UNUSED(value);
    Q_UNUSED(element);

    const auto property = getProperty(propertyName, binding, bindingScope);
    if (!property.isValid())
        return;

    if (const auto propertyType = property.type())
        warnOnCheckedBinding(binding, propertyType);
}


QT_END_NAMESPACE
