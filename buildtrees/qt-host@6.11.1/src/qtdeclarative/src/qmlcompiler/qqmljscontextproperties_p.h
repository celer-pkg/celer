// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant

#ifndef QQMLJSCONTEXTPROPERTIES_P_H
#define QQMLJSCONTEXTPROPERTIES_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include <qtqmlcompilerexports.h>

#include <QtCore/qstring.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/private/qflatmap_p.h>

#include <QtQml/private/qqmljssourcelocation_p.h>

#include <QtQmlToolingSettings/private/qqmltoolingsettings_p.h>

QT_BEGIN_NAMESPACE

class QSettings;

namespace QQmlJS {
class LoggerCategory;

struct Q_QMLCOMPILER_EXPORT HeuristicContextProperty
{
    QString filename = {};
    SourceLocation location = SourceLocation{};

    friend bool comparesEqual(const HeuristicContextProperty &a,
                              const HeuristicContextProperty &b) noexcept
    {
        return a.filename == b.filename && a.location == b.location;
    }
    Q_DECLARE_EQUALITY_COMPARABLE(HeuristicContextProperty);
};

class Q_QMLCOMPILER_EXPORT HeuristicContextProperties
{
public:
    bool contains(const QString &name) const { return m_properties.contains(name); }
    qsizetype size() const { return m_properties.size(); }
    bool isValid() const { return !m_properties.isEmpty(); }
    QList<HeuristicContextProperty> definitionsForName(const QString &name) const;
    void writeCache(const QString &folder) const;

    void add(const QString &name, const HeuristicContextProperty &property);

    static HeuristicContextProperties collectFromCppSourceDirs(const QList<QString> &cppSourceDirs);
    static HeuristicContextProperties collectFrom(QSettings *settings);

    friend bool comparesEqual(const HeuristicContextProperties &a,
                              const HeuristicContextProperties &b) noexcept
    {
        return std::equal(a.m_properties.begin(), a.m_properties.end(), b.m_properties.begin(),
                          b.m_properties.end());
    }
    Q_DECLARE_EQUALITY_COMPARABLE(HeuristicContextProperties);

private:
    struct Entry
    {
        QString name;
        QList<HeuristicContextProperty> definitions;

        friend bool comparesEqual(const Entry &a, const Entry &b) noexcept
        {
            return a.name == b.name && a.definitions == b.definitions;
        }
        Q_DECLARE_EQUALITY_COMPARABLE(Entry);
    };

    void collectFromDirs(const QList<QString> &dirs);
    void collectFromFile(const QString &file);
    void grepFallback(const QList<QString> &rootUrls);
#if QT_CONFIG(process) && !defined(Q_OS_WINDOWS)
    void parseGrepOutput(const QString &output);
#endif

    QFlatMap<QString, QList<HeuristicContextProperty>> m_properties;
};

} // namespace QQmlJS

QT_END_NAMESPACE

#endif // QQMLJSCONTEXTPROPERTIES_P_H
