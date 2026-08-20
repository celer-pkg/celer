// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant reason:default

#ifndef QDSLINTPLUGIN_H
#define QDSLINTPLUGIN_H

#include <QtCore/qplugin.h>

#include <QtQmlCompiler/qqmlsa.h>
#include "qqmlsaconstants.h"

QT_BEGIN_NAMESPACE

class QmlLintQdsPlugin : public QObject, public QQmlSA::LintPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QmlLintPluginInterface_iid FILE "plugin.json")
    Q_INTERFACES(QQmlSA::LintPlugin)

public:
    void registerPasses(QQmlSA::PassManager *manager, const QQmlSA::Element &rootElement) override;
};

QT_END_NAMESPACE

#endif // QDSLINTPLUGIN_H
