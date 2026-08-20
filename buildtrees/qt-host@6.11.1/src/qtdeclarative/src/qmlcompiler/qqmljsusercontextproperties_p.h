// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant

#ifndef QQMLJSUSERCONTEXTPROPERTIES_P_H
#define QQMLJSUSERCONTEXTPROPERTIES_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include <QtQmlToolingSettings/private/qqmltoolingsettings_p.h>
#include <qtqmlcompilerexports.h>

QT_BEGIN_NAMESPACE

namespace QQmlJS {

class Q_QMLCOMPILER_EXPORT UserContextProperties
{
public:
    static constexpr inline QLatin1StringView s_unqualifiedAccessDisabledKey =
            QLatin1String("disableUnqualifiedAccess");
    static constexpr inline QLatin1StringView s_onUsageWarnedKey = QLatin1String("warnOnUsage");
    static constexpr inline QLatin1StringView s_disableHeuristic =
            QLatin1String("disableHeuristic");

    UserContextProperties() = default;
    UserContextProperties(const QQmlToolingSettings &settings);

    bool isUnqualifiedAccessDisabled(const QString &name) const
    {
        return m_unqualifiedAccessDisabled.contains(name);
    }
    QList<QString> unqualifiedAccessDisabled() const { return m_unqualifiedAccessDisabled; }

    bool isOnUsageWarned(const QString &name) const { return m_onUsageWarned.contains(name); }
    QList<QString> onUsageWarned() const { return m_onUsageWarned; }

    bool isHeuristicDisabled() const { return m_disableHeuristic; }

private:
    void setUnqualifiedAccessDisabled(const QList<QString> &unqualifiedAccessDisabled)
    {
        m_unqualifiedAccessDisabled = unqualifiedAccessDisabled;
    }
    void setOnUsagedWarned(const QList<QString> &onUsageWarned) { m_onUsageWarned = onUsageWarned; }
    void setIsHeuristicDisabled(bool newDisableHeuristic)
    {
        m_disableHeuristic = newDisableHeuristic;
    }

    QList<QString> m_unqualifiedAccessDisabled = {};
    QList<QString> m_onUsageWarned = {};
    bool m_disableHeuristic = false;
};

} // namespace QQmlJS

QT_END_NAMESPACE

#endif // QQMLJSUSERCONTEXTPROPERTIES_P_H
