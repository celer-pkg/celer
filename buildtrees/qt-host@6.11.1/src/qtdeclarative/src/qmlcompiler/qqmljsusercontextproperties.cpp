// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant

#include "qqmljsusercontextproperties_p.h"

QT_BEGIN_NAMESPACE

namespace QQmlJS {

using namespace Qt::StringLiterals;

UserContextProperties::UserContextProperties(const QQmlToolingSettings &settings)
{
    auto listFromSettings = [&settings](QLatin1String key) -> QStringList {
        if (!settings.isSet(key))
            return {};
        return settings.value(key).toString().split(u',', Qt::SkipEmptyParts);
    };

    setUnqualifiedAccessDisabled(listFromSettings(s_unqualifiedAccessDisabledKey));
    setOnUsagedWarned(listFromSettings(s_onUsageWarnedKey));

    if (settings.isSet(s_disableHeuristic))
        setIsHeuristicDisabled(settings.value(s_disableHeuristic).toBool());
}

} // namespace QQmlJS

QT_END_NAMESPACE
