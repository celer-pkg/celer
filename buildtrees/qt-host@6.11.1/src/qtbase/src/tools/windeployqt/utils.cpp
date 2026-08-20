// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "utils.h"

#include "peheaderinfo.h"

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QTemporaryFile>
#include <QtCore/QScopedPointer>
#include <QtCore/QScopedArrayPointer>
#include <QtCore/QStandardPaths>

#if defined(Q_OS_WIN)
#  include <QtCore/qt_windows.h>
#  include <shlwapi.h>
#endif  // Q_OS_WIN

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

int optVerboseLevel = 1;

bool isBuildDirectory(Platform platform, const QString &dirName)
{
    return (platform.testFlag(Msvc) || platform.testFlag(ClangMsvc))
        && (dirName == "debug"_L1 || dirName == "release"_L1);
}

// Create a symbolic link by changing to the source directory to make sure the
// link uses relative paths only (QFile::link() otherwise uses the absolute path).
bool createSymbolicLink(const QFileInfo &source, const QString &target, QString *errorMessage)
{
    const QString oldDirectory = QDir::currentPath();
    if (!QDir::setCurrent(source.absolutePath())) {
        *errorMessage = QStringLiteral("Unable to change to directory %1.").arg(QDir::toNativeSeparators(source.absolutePath()));
        return false;
    }
    QFile file(source.fileName());
    const bool success = file.link(target);
    QDir::setCurrent(oldDirectory);
    if (!success) {
        *errorMessage = QString::fromLatin1("Failed to create symbolic link %1 -> %2: %3")
                        .arg(QDir::toNativeSeparators(source.absoluteFilePath()),
                             QDir::toNativeSeparators(target), file.errorString());
        return false;
    }
    return true;
}

bool createDirectory(const QString &directory, QString *errorMessage, bool dryRun)
{
    const QFileInfo fi(directory);
    if (fi.isDir())
        return true;
    if (fi.exists()) {
        *errorMessage = QString::fromLatin1("%1 already exists and is not a directory.").
                        arg(QDir::toNativeSeparators(directory));
        return false;
    }
    if (optVerboseLevel)
        std::wcout << "Creating " << QDir::toNativeSeparators(directory) << "...\n";
    if (!dryRun) {
        QDir dir;
        if (!dir.mkpath(directory)) {
            *errorMessage = QString::fromLatin1("Could not create directory %1.")
                                    .arg(QDir::toNativeSeparators(directory));
            return false;
        }
    }
    return true;
}

// Find shared libraries matching debug/Platform in a directory, return relative names.
QStringList findSharedLibraries(const QDir &directory, Platform platform,
                                DebugMatchMode debugMatchMode,
                                const QString &prefix)
{
    QString nameFilter = prefix;
    if (nameFilter.isEmpty())
        nameFilter += u'*';
    if (debugMatchMode == MatchDebug && platformHasDebugSuffix(platform))
        nameFilter += u'd';
    nameFilter += sharedLibrarySuffix();
    QStringList result;
    QString errorMessage;
    const QFileInfoList &dlls = directory.entryInfoList(QStringList(nameFilter), QDir::Files);
    for (const QFileInfo &dllFi : dlls) {
        const QString dllPath = dllFi.absoluteFilePath();
        bool matches = true;
        if (debugMatchMode != MatchDebugOrRelease && (platform & WindowsBased)) {
            PeHeaderInfoStruct info;
            if (readPeExecutableInfo(dllPath, &errorMessage, &info)) {
                matches = info.isDebug == (debugMatchMode == MatchDebug);
            } else {
                std::wcerr << "Warning: Unable to read " << QDir::toNativeSeparators(dllPath)
                           << ": " << errorMessage;
            }
        } // Windows
        if (matches)
            result += dllFi.fileName();
    } // for
    return result;
}

// Case-Normalize file name via GetShortPathNameW()/GetLongPathNameW()
QString normalizeFileName(const QString &name)
{
    wchar_t shortBuffer[MAX_PATH];
    const QString nativeFileName = QDir::toNativeSeparators(name);
    if (!GetShortPathNameW(reinterpret_cast<LPCWSTR>(nativeFileName.utf16()), shortBuffer, MAX_PATH))
        return name;
    wchar_t result[MAX_PATH];
    if (!GetLongPathNameW(shortBuffer, result, MAX_PATH))
        return name;
    return QDir::fromNativeSeparators(QString::fromWCharArray(result));
}

static inline void appendToCommandLine(const QString &argument, QString *commandLine)
{
    const bool needsQuote = argument.contains(u' ');
    if (!commandLine->isEmpty())
        commandLine->append(u' ');
    if (needsQuote)
        commandLine->append(u'"');
    commandLine->append(argument);
    if (needsQuote)
        commandLine->append(u'"');
}

bool runProcess(const QString &binary, const QStringList &args,
                const QString &workingDirectory,
                unsigned long *exitCode, QByteArray *stdOut, QByteArray *stdErr,
                QString *errorMessage, int timeout)
{
    if (exitCode)
        *exitCode = 0;

    QProcess process;
    process.setProgram(binary);
    process.setArguments(args);
    process.setWorkingDirectory(workingDirectory);

    // Output the command if requested.
    if (optVerboseLevel > 1) {
        QString commandLine;
        appendToCommandLine(binary, &commandLine);
        for (const QString &a : args)
            appendToCommandLine(a, &commandLine);
        std::wcout << "Running: " << commandLine << '\n';
    }

    process.start();
    if (!process.waitForStarted() || !process.waitForFinished(timeout)) {
        if (errorMessage)
            *errorMessage = process.errorString();
        return false;
    }

    if (stdOut)
        *stdOut = process.readAllStandardOutput();
    if (stdErr)
        *stdErr = process.readAllStandardError();

    return true;
}

// Find a file in the path using ShellAPI. This can be used to locate DLLs which
// QStandardPaths cannot do.
QString findInPath(const QString &file)
{
#if defined(Q_OS_WIN)
    if (file.size() < MAX_PATH -  1) {
        wchar_t buffer[MAX_PATH];
        file.toWCharArray(buffer);
        buffer[file.size()] = 0;
        if (PathFindOnPath(buffer, NULL))
            return QDir::cleanPath(QString::fromWCharArray(buffer));
    }
    return QString();
#else // Q_OS_WIN
    return QStandardPaths::findExecutable(file);
#endif // !Q_OS_WIN
}

const char *qmakeInfixKey = "QT_INFIX";

QMap<QString, QString> queryQtPaths(const QString &qtpathsBinary, QString *errorMessage)
{
    const QString binary = !qtpathsBinary.isEmpty() ? qtpathsBinary : QStringLiteral("qtpaths");
    const QString colonSpace = QStringLiteral(": ");
    QByteArray stdOut;
    QByteArray stdErr;
    unsigned long exitCode = 0;
    if (!runProcess(binary, QStringList(QStringLiteral("-query")), QString(), &exitCode, &stdOut,
                    &stdErr, errorMessage)) {
        *errorMessage = QStringLiteral("Error running binary ") + binary + colonSpace + *errorMessage;
        return QMap<QString, QString>();
    }
    if (exitCode) {
        *errorMessage = binary + QStringLiteral(" returns ") + QString::number(exitCode)
            + colonSpace + QString::fromLocal8Bit(stdErr);
        return QMap<QString, QString>();
    }
    const QString output = QString::fromLocal8Bit(stdOut).trimmed().remove(u'\r');
    QMap<QString, QString> result;
    const qsizetype size = output.size();
    for (qsizetype pos = 0; pos < size; ) {
        const qsizetype colonPos = output.indexOf(u':', pos);
        if (colonPos < 0)
            break;
        qsizetype endPos = output.indexOf(u'\n', colonPos + 1);
        if (endPos < 0)
            endPos = size;
        const QString key = output.mid(pos, colonPos - pos);
        const QString value = output.mid(colonPos + 1, endPos - colonPos - 1);
        result.insert(key, value);
        pos = endPos + 1;
    }
    QFile qconfigPriFile(result.value(QStringLiteral("QT_HOST_DATA")) + QStringLiteral("/mkspecs/qconfig.pri"));
    if (qconfigPriFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray lineArray;
        while (qconfigPriFile.readLineInto(&lineArray)) {
            QByteArrayView line = QByteArrayView(lineArray);
            if (line.startsWith("QT_LIBINFIX")) {
                const int pos = line.indexOf('=');
                if (pos >= 0) {
                    const QString infix = QString::fromUtf8(line.right(line.size() - pos - 1).trimmed());
                    if (!infix.isEmpty())
                        result.insert(QLatin1StringView(qmakeInfixKey), infix);
                }
                break;
            }
        }
    } else {
        std::wcerr << "Warning: Unable to read " << QDir::toNativeSeparators(qconfigPriFile.fileName())
            << colonSpace << qconfigPriFile.errorString()<< '\n';
    }
    return result;
}

// Update a file or directory.
bool updateFile(const QString &sourceFileName, const QStringList &nameFilters,
                const QString &targetDirectory, unsigned flags, JsonOutput *json, QString *errorMessage)
{
    const QFileInfo sourceFileInfo(sourceFileName);
    const QString targetFileName = targetDirectory + u'/' + sourceFileInfo.fileName();
    if (optVerboseLevel > 1)
        std::wcout << "Checking " << sourceFileName << ", " << targetFileName<< '\n';

    if (!sourceFileInfo.exists()) {
        *errorMessage = QString::fromLatin1("%1 does not exist.").arg(QDir::toNativeSeparators(sourceFileName));
        return false;
    }

    if (sourceFileInfo.isSymLink()) {
        *errorMessage = QString::fromLatin1("Symbolic links are not supported (%1).")
                        .arg(QDir::toNativeSeparators(sourceFileName));
        return false;
    }

    const QFileInfo targetFileInfo(targetFileName);

    if (sourceFileInfo.isDir()) {
        if (targetFileInfo.exists()) {
            if (!targetFileInfo.isDir()) {
                *errorMessage = QString::fromLatin1("%1 already exists and is not a directory.")
                                .arg(QDir::toNativeSeparators(targetFileName));
                return false;
            } // Not a directory.
        } else { // exists.
            QDir d(targetDirectory);
            if (optVerboseLevel)
                std::wcout << "Creating " << QDir::toNativeSeparators(targetFileName) << ".\n";
            if (!(flags & SkipUpdateFile) && !d.mkdir(sourceFileInfo.fileName())) {
                *errorMessage = QString::fromLatin1("Cannot create directory %1 under %2.")
                                .arg(sourceFileInfo.fileName(), QDir::toNativeSeparators(targetDirectory));
                return false;
            }
        }
        // Recurse into directory
        QDir dir(sourceFileName);
        const QFileInfoList allEntries = dir.entryInfoList(nameFilters, QDir::Files)
            + dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo &entryFi : allEntries) {
            if (!updateFile(entryFi.absoluteFilePath(), nameFilters, targetFileName, flags, json, errorMessage))
                return false;
        }
        return true;
    } // Source is directory.

    if (targetFileInfo.exists()) {
        if (!(flags & ForceUpdateFile)
            && targetFileInfo.lastModified() >= sourceFileInfo.lastModified()) {
            if (optVerboseLevel)
                std::wcout << sourceFileInfo.fileName() << " is up to date.\n";
            if (json)
                json->addFile(sourceFileName, targetDirectory);
            return true;
        }
        QFile targetFile(targetFileName);
        if (!(flags & SkipUpdateFile) && !targetFile.remove()) {
            *errorMessage = QString::fromLatin1("Cannot remove existing file %1: %2")
                            .arg(QDir::toNativeSeparators(targetFileName), targetFile.errorString());
            return false;
        }
    } // target exists
    QFile file(sourceFileName);
    if (optVerboseLevel)
        std::wcout << "Updating " << sourceFileInfo.fileName() << ".\n";
    if (!(flags & SkipUpdateFile) && !file.copy(targetFileName)) {
        *errorMessage = QString::fromLatin1("Cannot copy %1 to %2: %3")
                .arg(QDir::toNativeSeparators(sourceFileName),
                     QDir::toNativeSeparators(targetFileName),
                     file.errorString());
        return false;
    }
    if (json)
        json->addFile(sourceFileName, targetDirectory);
    return true;
}

#ifdef Q_OS_WIN
// Read a PE executable and determine word size, debug flags, and arch
bool readPeExecutableInfo(const QString &peExecutableFileName, QString *errorMessage,
                          PeHeaderInfoStruct *headerInfo)
{
    bool result = false;
    if (!headerInfo) {
        *errorMessage = QString::fromLatin1("Mandatory parameter missing. Please provide headerInfo");
        return result;
    }

    PeHeaderInfo *peHeaderInfo = PeHeaderInfoCache::peHeaderInfo(peExecutableFileName);
    if (!peHeaderInfo->isValid()) {
        *errorMessage = peHeaderInfo->errorMessage();
        return result;
    }

    headerInfo->wordSize = peHeaderInfo->wordSize();
    headerInfo->isDebug = peHeaderInfo->isDebug();
    headerInfo->machineArch = peHeaderInfo->machineArch();

    result = true;
    if (optVerboseLevel > 1) {
        std::wcout << __FUNCTION__ << ": " << QDir::toNativeSeparators(peExecutableFileName)
        << ' ' << headerInfo->wordSize << " bit";
        std::wcout << (headerInfo->isDebug ? ", debug" : ", release");
        std::wcout << '\n';
    }


    return result;
}

// Read a PE executable and determine dependent libraries.
bool readPeExecutableDependencies(const QString &peExecutableFileName, QString *errorMessage,
                      QStringList *dependentLibraries)
{
    bool result = false;
    if (!dependentLibraries) {
        *errorMessage = QString::fromLatin1("Mandatory parameter missing. Provide dependentLibraries");
        return result;
    }

    dependentLibraries->clear();

    PeHeaderInfo *peHeaderInfo = PeHeaderInfoCache::peHeaderInfo(peExecutableFileName);
    if (!peHeaderInfo->isValid()) {
        *errorMessage = peHeaderInfo->errorMessage();
        return result;
    }

    *dependentLibraries = peHeaderInfo->dependentLibs();

    result = true;
    if (optVerboseLevel > 1) {
        std::wcout << __FUNCTION__ << ": " << QDir::toNativeSeparators(peExecutableFileName);
        std::wcout << ", dependent libraries: ";
        if (optVerboseLevel > 2)
            std::wcout << dependentLibraries->join(u' ');
        else
            std::wcout << dependentLibraries->size();
        std::wcout << '\n';
    }

    return result;
}

QString findD3dCompiler(Platform platform, const QString &qtBinDir, unsigned wordSize)
{
    const QString prefix = QStringLiteral("D3Dcompiler_");
    const QString suffix = QLatin1StringView(windowsSharedLibrarySuffix);
    // Get the DLL from Kit 8.0 onwards
    const QString kitDir = QString::fromLocal8Bit(qgetenv("WindowsSdkDir"));
    if (!kitDir.isEmpty()) {
        QString redistDirPath = QDir::cleanPath(kitDir) + QStringLiteral("/Redist/D3D/");
        if (platform.testFlag(ArmBased)) {
            redistDirPath += QStringLiteral("arm");
        } else {
            redistDirPath += wordSize == 32 ? QStringLiteral("x86") : QStringLiteral("x64");
        }
        QDir redistDir(redistDirPath);
        if (redistDir.exists()) {
            const QFileInfoList files = redistDir.entryInfoList(QStringList(prefix + u'*' + suffix), QDir::Files);
            if (!files.isEmpty())
                return files.front().absoluteFilePath();
        }
    }
    QStringList candidateVersions;
    for (int i = 47 ; i >= 40 ; --i)
        candidateVersions.append(prefix + QString::number(i) + suffix);
    // Check the bin directory of the Qt SDK (in case it is shadowed by the
    // Windows system directory in PATH).
    for (const QString &candidate : std::as_const(candidateVersions)) {
        const QFileInfo fi(qtBinDir + u'/' + candidate);
        if (fi.isFile())
            return fi.absoluteFilePath();
    }
    // Find the latest D3D compiler DLL in path (Windows 8.1 has d3dcompiler_47).
    if (platform.testFlag(IntelBased)) {
        QString errorMessage;
        PeHeaderInfoStruct info;
        for (const QString &candidate : std::as_const(candidateVersions)) {
            const QString dll = findInPath(candidate);
            if (!dll.isEmpty()
                && readPeExecutableInfo(dll, &errorMessage, &info)
                && info.wordSize == wordSize) {
                return dll;
            }
        }
    }
    return QString();
}

QStringList findDxc(Platform platform, const QString &qtBinDir, unsigned wordSize)
{
    QStringList results;
    const QString kitDir = QString::fromLocal8Bit(qgetenv("WindowsSdkDir"));
    const QString suffix = QLatin1StringView(windowsSharedLibrarySuffix);
    for (QString prefix : { QStringLiteral("dxcompiler"), QStringLiteral("dxil") }) {
        QString name = prefix + suffix;
        if (!kitDir.isEmpty()) {
            QString redistDirPath = QDir::cleanPath(kitDir) + QStringLiteral("/Redist/D3D/");
            if (platform.testFlag(ArmBased)) {
                redistDirPath += wordSize == 32 ? QStringLiteral("arm") : QStringLiteral("arm64");
            } else {
                redistDirPath += wordSize == 32 ? QStringLiteral("x86") : QStringLiteral("x64");
            }
            QDir redistDir(redistDirPath);
            if (redistDir.exists()) {
                const QFileInfoList files = redistDir.entryInfoList(QStringList(prefix + u'*' + suffix), QDir::Files);
                if (!files.isEmpty()) {
                    results.append(files.front().absoluteFilePath());
                    continue;
                }
            }
        }
        // Check the bin directory of the Qt SDK (in case it is shadowed by the
        // Windows system directory in PATH).
        const QFileInfo fi(qtBinDir + u'/' + name);
        if (fi.isFile()) {
            results.append(fi.absoluteFilePath());
            continue;
        }
        // Try to find it in the PATH (e.g. the Vulkan SDK ships these, even if Windows itself doesn't).
        if (platform.testFlag(IntelBased)) {
            QString errorMessage;
            PeHeaderInfoStruct info;
            const QString dll = findInPath(name);
            if (!dll.isEmpty()
                && readPeExecutableInfo(dll, &errorMessage, &info)
                && info.wordSize == wordSize)
            {
                results.append(dll);
                continue;
            }
        }
    }
    return results;
}

#else // Q_OS_WIN

bool readPeExecutableInfo(const QString &, QString *errorMessage,
                      PeHeaderInfoStruct *)
{
    *errorMessage = QStringLiteral("Not implemented.");
    return false;
}

bool readPeExecutableDependencies(const QString &, QString *, QStringList *)
{
    *errorMessage = QStringLiteral("Not implemented.");
    return false;
}

QString findD3dCompiler(Platform, const QString &, unsigned)
{
    return QString();
}

QStringList findDxc(Platform, const QString &, unsigned)
{
    return QStringList();
}

#endif // !Q_OS_WIN

// Search for "qt_prfxpath=xxxx" in \a path, and replace it with "qt_prfxpath=."
bool patchQtCore(const QString &path, QString *errorMessage)
{
    if (optVerboseLevel)
        std::wcout << "Patching " << QFileInfo(path).fileName() << "...\n";

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        *errorMessage = QString::fromLatin1("Unable to patch %1: %2").arg(
                    QDir::toNativeSeparators(path), file.errorString());
        return false;
    }
    const QByteArray oldContent = file.readAll();

    if (oldContent.isEmpty()) {
        *errorMessage = QString::fromLatin1("Unable to patch %1: Could not read file content").arg(
                    QDir::toNativeSeparators(path));
        return false;
    }
    file.close();

    QByteArray content = oldContent;

    QByteArray prfxpath("qt_prfxpath=");
    int startPos = content.indexOf(prfxpath);
    if (startPos == -1) {
        *errorMessage = QString::fromLatin1(
                    "Unable to patch %1: Could not locate pattern \"qt_prfxpath=\"").arg(
                    QDir::toNativeSeparators(path));
        return false;
    }
    startPos += prfxpath.length();
    int endPos = content.indexOf(char(0), startPos);
    if (endPos == -1) {
        *errorMessage = QString::fromLatin1("Unable to patch %1: Internal error").arg(
                    QDir::toNativeSeparators(path));
        return false;
    }

    QByteArray replacement = QByteArray(endPos - startPos, char(0));
    replacement[0] = '.';
    content.replace(startPos, endPos - startPos, replacement);
    if (content == oldContent)
        return true;

    if (!file.open(QIODevice::WriteOnly)
        || (file.write(content) != content.size())) {
        *errorMessage = QString::fromLatin1("Unable to patch %1: Could not write to file: %2").arg(
                    QDir::toNativeSeparators(path), file.errorString());
        return false;
    }
    return true;
}

#ifdef Q_OS_WIN
QString getArchString(unsigned short machineArch)
{
    switch (machineArch) {
        case IMAGE_FILE_MACHINE_I386:
            return QStringLiteral("x86");
        case IMAGE_FILE_MACHINE_ARM:
            return QStringLiteral("arm");
        case IMAGE_FILE_MACHINE_AMD64:
            return QStringLiteral("x64");
        case IMAGE_FILE_MACHINE_ARM64:
            return QStringLiteral("arm64");
        default:
            break;
    }
    return QString();
}
#endif // Q_OS_WIN

QT_END_NAMESPACE
