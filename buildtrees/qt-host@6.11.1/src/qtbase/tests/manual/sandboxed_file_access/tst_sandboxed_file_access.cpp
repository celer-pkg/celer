// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore>
#include <QtWidgets>
#include <QtTest>

#include <Foundation/Foundation.h>

#if defined(Q_OS_MACOS) && defined(QT_BUILD_INTERNAL)
#include <private/qcore_mac_p.h>
Q_CONSTRUCTOR_FUNCTION(qt_mac_ensureResponsible);
#endif

class tst_SandboxedFileAccess : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void alwaysAccessibleLocations();

    void standardPaths_data();
    void standardPaths();

    void readSingleFile();
    void writeSingleFile();
    void writeSingleFileNonCanonical();

    void removeFile();
    void trashFile();

    void readFileAfterRestart();

    void directoryAccess();

    void securityScopedTargetFile();

    void fileOpenEvent();

private:
    void writeFile(const QString &fileName);
    QByteArray readFile(const QString &fileName);

    QString getFileName(QFileDialog::AcceptMode, QFileDialog::FileMode,
        const QString &action = QString(), const QString &fileName = QString());

    QString sandboxPath() const
    {
        return QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    }

    QString bundlePath() const
    {
        QString path = QCoreApplication::applicationDirPath();
#if defined(Q_OS_MACOS)
        path.remove("/Contents/MacOS");
#endif
        return path;
    }

    QStringList m_persistedFileNames;
    QPointer<QWidget> m_widget;
};

void tst_SandboxedFileAccess::initTestCase()
{
    qDebug() << "📦 App bundle" << bundlePath();
    qDebug() << "🔐 App container" << sandboxPath();

    m_widget = new QWidget;
    m_widget->show();
    QVERIFY(QTest::qWaitForWindowExposed(m_widget));
}

void tst_SandboxedFileAccess::cleanupTestCase()
{
    NSURL *appSupportDir = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)).toNSURL();
    NSURL *bookmarksFile = [appSupportDir URLByAppendingPathComponent:@"SecurityScopedBookmarks.plist"];
    NSError *error = nullptr;
    NSMutableDictionary *bookmarks = [[NSDictionary dictionaryWithContentsOfURL:bookmarksFile
                        error:&error] mutableCopy];
    for (NSString *path in bookmarks.allKeys) {
        if (m_persistedFileNames.contains(QString::fromNSString(path))) {
            qDebug() << "Keeping knowledge of persisted path" << path;
            continue;
        }
        qDebug() << "Wiping knowledge of path" << path;
        [bookmarks removeObjectForKey:path];
    }
    [bookmarks writeToURL:bookmarksFile error:&error];

    qGuiApp->quit();
}

void tst_SandboxedFileAccess::alwaysAccessibleLocations()
{
    readFile(QCoreApplication::applicationFilePath());

    // The documents location is inside the sandbox and writable on both iOS and macOS
    auto documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    writeFile(documents + "/test-writable-file.txt");
}

void tst_SandboxedFileAccess::standardPaths_data()
{
    QTest::addColumn<QStandardPaths::StandardLocation>("location");
    auto standardLocations = QMetaEnum::fromType<QStandardPaths::StandardLocation>();
    for (int i = 0; i < standardLocations.keyCount(); ++i)
        QTest::newRow(standardLocations.key(i)) << QStandardPaths::StandardLocation(standardLocations.value(i));
}

void tst_SandboxedFileAccess::standardPaths()
{
    QFETCH(QStandardPaths::StandardLocation, location);
    auto writableLocation = QStandardPaths::writableLocation(location);

    if (writableLocation.isEmpty())
        QSKIP("There's no writable location for this location");

    QFileInfo info(writableLocation);
    if (info.isSymLink() && !info.symLinkTarget().startsWith(sandboxPath()))
        QSKIP("This location is a symlink to outside the sandbox and requires access");

    QVERIFY(QDir().mkpath(writableLocation));

#if !defined(Q_OS_MACOS)
    QEXPECT_FAIL("HomeLocation", "The sandbox root is not writable on iOS", Abort);
#endif
    writeFile(writableLocation + QString("/test-writable-file-%1.txt").arg(QTest::currentDataTag()));
}

void tst_SandboxedFileAccess::readSingleFile()
{
    QString filePath = getFileName(QFileDialog::AcceptOpen,
        QFileDialog::ExistingFile, "Choose file to read");
    readFile(filePath);

    {
        QFile file(QCoreApplication::applicationFilePath());
        QVERIFY(file.open(QFile::ReadOnly));
        QByteArray plistContent = file.read(100);
        file.close();

        // Check that setFileName can target a security scoped file
        file.setFileName(filePath);
        QVERIFY(file.open(QFile::ReadOnly));
        QVERIFY(file.isReadable());
        QCOMPARE_NE(file.read(100), plistContent);
    }

    QDir dir;
    QString fileName;

    {
        QFileInfo info(filePath);
        dir = info.path();
        fileName = info.fileName();
        QVERIFY(dir.exists());
        QVERIFY(!fileName.isEmpty());
    }

    // Check that we're able to access files via non-canonical paths
    readFile(dir.absolutePath() + "/../" + dir.dirName() + "/" + fileName);
}

QByteArray tst_SandboxedFileAccess::readFile(const QString &fileName)
{
    QFile file(fileName);
    QVERIFY(file.exists());
    QVERIFY(file.open(QFile::ReadOnly));
    QVERIFY(file.isReadable());
    QByteArray data = file.read(100);
    QVERIFY(!data.isEmpty());
    return data;
}

void tst_SandboxedFileAccess::writeSingleFile()
{
    QString filePath = getFileName(QFileDialog::AcceptSave, QFileDialog::AnyFile,
        "Choose a file to write", "write-single-file.txt");
    writeFile(filePath);
    readFile(filePath);
}

void tst_SandboxedFileAccess::writeSingleFileNonCanonical()
{
    QString filePath = getFileName(QFileDialog::AcceptSave, QFileDialog::AnyFile,
        "Choose a file to write", "write-single-file-non-canonical.txt");
    QDir dir;
    QString fileName;

    {
        QFileInfo info(filePath);
        dir = info.path();
        fileName = info.fileName();
        QVERIFY(dir.exists());
        QVERIFY(!fileName.isEmpty());
    }

    writeFile(dir.absolutePath() + "/../" + dir.dirName() + "/" + fileName);
    readFile(filePath);
}

void tst_SandboxedFileAccess::writeFile(const QString &fileName)
{
    QFile file(fileName);
    QVERIFY(file.open(QFile::WriteOnly));
    QVERIFY(file.isWritable());
    QVERIFY(file.write("Hello world"));
}

void tst_SandboxedFileAccess::removeFile()
{
    QString fileName = getFileName(QFileDialog::AcceptSave, QFileDialog::AnyFile,
        "Choose a file to write and then remove", "write-and-remove-file.txt");
    writeFile(fileName);

    {
        QFile file(fileName);
        QVERIFY(file.remove());
    }
}

void tst_SandboxedFileAccess::trashFile()
{
    QString fileName = getFileName(QFileDialog::AcceptSave, QFileDialog::AnyFile,
        "Choose a file to write and then trash", "write-and-trash-file.txt");
    writeFile(fileName);

    {
        QFile file(fileName);
        QVERIFY(file.moveToTrash());
    }
}

void tst_SandboxedFileAccess::readFileAfterRestart()
{
    // Every other restart of the app will save a file or load a previously saved file

    QSettings settings;
    QString savedFile = settings.value("savedFile").toString();
    if (savedFile.isEmpty()) {
        QString filePath = getFileName(QFileDialog::AcceptSave, QFileDialog::AnyFile,
            "Choose a file to write for reading after restart", "write-and-read-after-restart.txt");
        qDebug() << "Writing" << filePath << "and saving to preferences";
        writeFile(filePath);
        settings.setValue("savedFile", filePath);
        m_persistedFileNames << filePath;
    } else {
        qDebug() << "Loading" << savedFile << "from preferences";
        settings.remove("savedFile"); // Remove up front, in case this fails
        readFile(savedFile);
        QFile file(savedFile);
        QVERIFY(file.remove());
    }
}

void tst_SandboxedFileAccess::directoryAccess()
{
    // Every other restart of the app will re-establish access to the folder,
    // or re-use previous access.

    QSettings settings;
    QString directory = settings.value("savedDirectory").toString();
    if (directory.isEmpty()) {
        directory = getFileName(QFileDialog::AcceptOpen, QFileDialog::Directory,
            "Choose a directory we can create some files in");
        auto canonical = QFileInfo(directory).canonicalFilePath();
        QVERIFY(!canonical.isEmpty());
        directory = canonical;
        settings.setValue("savedDirectory", directory);
        m_persistedFileNames << QFileInfo(directory).canonicalFilePath();
    } else {
        settings.remove("savedDirectory");
    }
    settings.sync();

    QString fileInDir;

    {
        QDir dir(directory);
        QVERIFY(dir.exists());
        QVERIFY(dir.isReadable());
        fileInDir = dir.filePath("file-in-dir.txt");
    }

    writeFile(fileInDir);
    readFile(fileInDir);

    {
        QDir dir(directory);
        QVERIFY(dir.count() > 0);
        QVERIFY(dir.entryList().contains("file-in-dir.txt"));
    }

    {
        QDir dir(directory);
        QVERIFY(dir.mkdir("subdirectory"));
        QVERIFY(dir.entryList().contains("subdirectory"));
        fileInDir = dir.filePath("subdirectory/file-in-subdir.txt");
    }

    writeFile(fileInDir);
    readFile(fileInDir);

    // Check that we can write to a non-canonical path within the directory
    // we have access to, and then read it from the canonical path.
    writeFile(directory + "/subdirectory/../non-existing-non-canonical.txt");
    readFile(directory + "/non-existing-non-canonical.txt");

    {
        QDir dir(directory);
        QVERIFY(dir.cd("subdirectory"));
        dir.removeRecursively();
    }
}

void tst_SandboxedFileAccess::securityScopedTargetFile()
{
    // This is a non-security scoped file
    auto documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString sourceFilePath = documents + "/test-security-scoped-target-file.txt";
    writeFile(sourceFilePath);
    QFile sourceFile(sourceFilePath);

    QString directory = getFileName(QFileDialog::AcceptOpen, QFileDialog::Directory,
        "Choose a directory we can link/copy some to");

    QString subDirectory;
    {
        QDir dir(directory);
        QVERIFY(dir.mkdir("subdirectory"));
        QVERIFY(dir.entryList().contains("subdirectory"));
        subDirectory = dir.filePath("subdirectory");
    }

    QVERIFY(sourceFile.copy(subDirectory + "/copied-file.txt"));
    QVERIFY(sourceFile.link(subDirectory + "/linked-file.txt"));
    QVERIFY(sourceFile.rename(subDirectory + "/renamed-file.txt"));

    {
        QDir dir(directory);
        QVERIFY(dir.cd("subdirectory"));
        dir.removeRecursively();
    }
}

void tst_SandboxedFileAccess::fileOpenEvent()
{
    struct OpenEventFilter : public QObject
    {
        bool eventFilter(QObject *watched, QEvent *event) override
        {
            if (event->type() == QEvent::FileOpen) {
                QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
                fileName = openEvent->file();
            }

            return QObject::eventFilter(watched, event);
        }

        QString fileName;
    };

    OpenEventFilter openEventFilter;
    qGuiApp->installEventFilter(&openEventFilter);

    m_widget->setLayout(new QVBoxLayout);
    QLabel label;
    label.setWordWrap(true);
    m_widget->layout()->addWidget(&label);
#if defined(Q_OS_MACOS)
    label.setText("Drag a text file to the app's Dock icon, or open in the app via Finder's 'Open With' menu");
#else
    label.setText("Open the Files app, and choose 'Open With' or share a text document with this app");
#endif
    label.show();

    QTRY_VERIFY_WITH_TIMEOUT(!openEventFilter.fileName.isNull(), 30s);
    label.setText("Got file: " + openEventFilter.fileName);

    readFile(openEventFilter.fileName);

    QTest::qWait(3000);
}

QString tst_SandboxedFileAccess::getFileName(QFileDialog::AcceptMode acceptMode, QFileDialog::FileMode fileMode,
    const QString &action, const QString &fileName)
{
    QFileDialog dialog(m_widget);
    dialog.setAcceptMode(acceptMode);
    dialog.setFileMode(fileMode);
    dialog.setWindowTitle(action);
    dialog.setLabelText(QFileDialog::Accept, action);
    dialog.selectFile(fileName);
    if (!action.isEmpty())
        qDebug() << "ℹ️" << action;
    dialog.exec();
    auto selectedFiles = dialog.selectedFiles();
    return selectedFiles.count() ? selectedFiles.first() : QString();
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    tst_SandboxedFileAccess testObject;

    // Run tests with QApp running
    int testExecResult = 0;
    QMetaObject::invokeMethod(&testObject, [&]{
        testExecResult = QTest::qExec(&testObject, argc, argv);
    }, Qt::QueuedConnection);

    [[maybe_unused]] int appExecResult = app.exec();
    return testExecResult;
}

#include "tst_sandboxed_file_access.moc"
