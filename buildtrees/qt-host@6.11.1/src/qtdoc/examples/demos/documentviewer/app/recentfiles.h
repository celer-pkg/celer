// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef RECENTFILES_H
#define RECENTFILES_H

#include <QMenu>
#include <QFile>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

class RecentFiles : public QObject
{
    Q_OBJECT
public:
    enum class RemoveReason {
        Other,
        Duplicate
    };
    Q_ENUM(RemoveReason)

    using QObject::QObject;

    // Access to QStringList member functions
    const QStringList recentFiles() const {return m_files; }
    bool isEmpty() const { return m_files.isEmpty(); }

    // Properties
    qsizetype maxFiles() const { return m_maxFiles; }
    void setMaxFiles(qsizetype maxFiles) { m_maxFiles = maxFiles; }

public slots:
    void addFile(const QString &fileName) { addFile(fileName, EmitPolicy::EmitWhenChanged); }
    void addFiles(const QStringList &fileNames);
    void removeFile(const QString &fileName) { removeFile(m_files.indexOf(fileName)); }
    void removeFile(qsizetype index) {removeFile(index, RemoveReason::Other); }
    void saveSettings(QSettings &settings, const QString &key) const;
    bool restoreFromSettings(QSettings &settings, const QString &key);
    void clear();

signals:
    void countChanged(int count);
    void changed();

private:
    // Private removers with reason
    void removeFile(qsizetype index, RemoveReason reason);
    void removeFile(const QString &fileName, RemoveReason reason) {removeFile(m_files.indexOf(fileName), reason); }

    // Private adder with emit policy
    enum class EmitPolicy {
        EmitWhenChanged,
        NeverEmit
    };

    void addFile(const QString &fileName, EmitPolicy policy);

    qsizetype m_maxFiles = 10;

    QStringList m_files;

    // Constexprs for settings
    static constexpr QLatin1StringView s_maxFiles = QLatin1StringView("maxFiles");
    static constexpr QLatin1StringView s_openMode = QLatin1StringView("openMode");
    static constexpr QLatin1StringView s_fileNames = QLatin1StringView("fileNames");
    static constexpr QLatin1StringView s_file = QLatin1StringView("file");
};

#endif // RECENTFILES_H
