// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "recentfiles.h"
#include <QFileInfo>
#include <QSettings>

// Test if file exists and can be opened
static bool testFileAccess(const QString &fileName)
{
    return QFileInfo(fileName).isReadable();
}

void RecentFiles::clear()
{
    if (isEmpty())
        return;

    m_files.clear();
    emit countChanged(0);
}

void RecentFiles::addFile(const QString &fileName, EmitPolicy policy)
{
    if (!testFileAccess(fileName))
        return;

    // Remember size, as cleanup can result in a change without size change
    const qsizetype c = m_files.count();

    // Clean dangling and duplicate files
    for (qsizetype i = 0; i < m_files.size(); ) {
        const QString &file = m_files.at(i);
        if (!testFileAccess(file)) {
            removeFile(file, RemoveReason::Other);
        } else if (file == fileName) {
            removeFile(file, RemoveReason::Duplicate);
        } else {
            ++i;
        }
    }

    // Cut tail
    while (m_files.count() > m_maxFiles)
        removeFile((m_files.count() - 1), RemoveReason::Other);

    m_files.prepend(fileName);

    switch (policy) {
    case EmitPolicy::NeverEmit:
        return;

    case EmitPolicy::EmitWhenChanged:
        emit changed();

        if (c != m_files.count())
            emit countChanged(m_files.count());

        return;
    }
}

void RecentFiles::addFiles(const QStringList &files)
{
    if (files.isEmpty())
        return;

    if (files.count() == 1) {
        addFile(files.at(0));
        return;
    }

    const qsizetype c = m_files.count();

    for (const auto &file : files)
        addFile(file, EmitPolicy::NeverEmit);

    emit changed();
    if (m_files.count() != c)
        emit countChanged(m_files.count());
}

void RecentFiles::removeFile(qsizetype index, RemoveReason reason)
{
    if (index < 0 || index >= m_files.count())
        return;

    m_files.remove(index);

    // No emit for duplicate removal, add emits changed later.
    if (reason != RemoveReason::Duplicate)
        emit changed();
}

void RecentFiles::saveSettings(QSettings &settings, const QString &key) const
{
    settings.beginGroup(key);
    settings.setValue(s_maxFiles, maxFiles());
    if (!isEmpty()) {
        settings.beginWriteArray(s_fileNames, m_files.count());
        for (int index = 0; index < m_files.count(); ++index) {
            settings.setArrayIndex(index);
            settings.setValue(s_file, m_files.at(index));
        }
        settings.endArray();
    }
    settings.endGroup();
}

bool RecentFiles::restoreFromSettings(QSettings &settings, const QString &key)
{
    settings.beginGroup(key);
    const qsizetype mxFiles = settings.value(s_maxFiles, maxFiles()).toLongLong();
    setMaxFiles(mxFiles);
    m_files.clear(); // clear list without emitting
    const int numberFiles = settings.beginReadArray(s_fileNames);
    for (int index = 0; index < numberFiles; ++index) {
        settings.setArrayIndex(index);
        const QString absoluteFilePath = settings.value(s_file).toString();
        addFile(absoluteFilePath, EmitPolicy::NeverEmit);
    }
    settings.endArray();
    settings.endGroup();
    if (!m_files.isEmpty())
        emit changed();

    return true;
}
