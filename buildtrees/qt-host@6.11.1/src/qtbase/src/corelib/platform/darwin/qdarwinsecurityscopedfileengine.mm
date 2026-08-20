// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdarwinsecurityscopedfileengine_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qreadwritelock.h>
#include <QtCore/qscopedvaluerollback.h>

#include <QtCore/private/qcore_mac_p.h>
#include <QtCore/private/qfsfileengine_p.h>
#include <QtCore/private/qfilesystemengine_p.h>

#include <thread>
#include <mutex>

#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/NSURL.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcSecEngine, "qt.core.io.security-scoped-fileengine", QtCriticalMsg)

template<typename T> class BackgroundLoader;

/*
    File engine handler for security scoped file paths.

    Installs itself as soon as QtCore is loaded if the application
    is sandboxed (optionally on macOS, and always on iOS and friends).
*/
class SecurityScopedFileEngineHandler : public QAbstractFileEngineHandler
{
public:
    SecurityScopedFileEngineHandler();
    ~SecurityScopedFileEngineHandler();

    void registerPossiblySecurityScopedURL(NSURL *url);

    std::unique_ptr<QAbstractFileEngine> create(const QString &fileName) const override;

    static BackgroundLoader<SecurityScopedFileEngineHandler>& get();

private:
    Q_DISABLE_COPY_MOVE(SecurityScopedFileEngineHandler)

    void saveBookmark(NSURL *url);
    void saveBookmarks();

    NSURL *bookmarksFile() const;

    static NSString *cacheKeyForUrl(NSURL *url);
    static NSString *cacheKeyForPath(const QString &url);

    NSMutableDictionary *m_bookmarks = nullptr;
    mutable QReadWriteLock m_bookmarkLock;

    friend class SecurityScopedFileEngine;
};

/*
    Helper class for asynchronous instantiation of types.
*/
template<typename T>
class BackgroundLoader
{
public:
    explicit BackgroundLoader(bool shouldLoad) {
        if (shouldLoad) {
            m_thread = std::thread([this]() {
                m_instance = std::make_unique<T>();
            });
        }
    }

    ~BackgroundLoader()
    {
        std::scoped_lock lock(m_mutex);
        if (m_thread.joinable())
            m_thread.join();
    }

    T* operator->() const
    {
        std::scoped_lock lock(m_mutex);
        if (m_thread.joinable())
            m_thread.join();
        return m_instance.get();
    }

    explicit operator bool() const
    {
        std::scoped_lock lock(m_mutex);
        return m_thread.joinable() || m_instance;
    }

private:
    mutable std::mutex m_mutex;
    mutable std::thread m_thread;
    std::unique_ptr<T> m_instance;
};

/*
    Thread-safe background-loading of optional security scoped handler,
    with the ability to kick off instantiation early during program load.
*/
BackgroundLoader<SecurityScopedFileEngineHandler>& SecurityScopedFileEngineHandler::get()
{
    using Handler = BackgroundLoader<SecurityScopedFileEngineHandler>;
    static Handler handler = []() -> Handler {
        if (!qt_apple_isSandboxed())
            return Handler{false};

        qCInfo(lcSecEngine) << "Application sandbox is active. Registering security-scoped file engine.";
        return Handler{true};
    }();
    return handler;
}

static void initializeSecurityScopedFileEngineHandler()
{
    // Kick off loading of bookmarks early in the background
    std::ignore = SecurityScopedFileEngineHandler::get();
}
Q_CONSTRUCTOR_FUNCTION(initializeSecurityScopedFileEngineHandler);

/*
    Registration function for possibly security scoped URLs.

    Entry points that might provide security scoped URLs such as file
    dialogs or drag-and-drop should use this function to ensure that
    the security scoped file engine handler knows about the URL.
*/
QUrl qt_apple_urlFromPossiblySecurityScopedURL(NSURL *url)
{
    if (auto &handler = SecurityScopedFileEngineHandler::get())
        handler->registerPossiblySecurityScopedURL(url);

    // Note: The URL itself doesn't encode any of the bookmark data,
    // neither in the scheme or as fragments or query parameters,
    // as it's all handled by the bookmark cache in the file engine.
    return QUrl(QString::fromNSString(url.absoluteString)
        .normalized(QString::NormalizationForm_C));
}

static bool checkIfResourceIsReachable(NSURL *url)
{
    NSError *error = nullptr;
    if ([url checkResourceIsReachableAndReturnError:&error])
        return true;

    // Our goal is to check whether the file exists or not, and if
    // not, defer creating a bookmark for it. If we get any other
    // error we want to know.
    if (![error.domain isEqualToString:NSCocoaErrorDomain] || error.code != NSFileReadNoSuchFileError) {
        qCWarning(lcSecEngine) << "Unexpected" << error
            << "when resolving reachability for" << url;
    }

    return false;
}

/*
    File engine for maintaining access lifetime of security-scoped
    resources on sandboxed Apple platforms.

    Note that there isn't necessarily a 1:1 relationship between
    the file being operated on by the QFSFileEngine and the security
    scoped resource that allows access to it, for example in the
    case of a folder giving access to all files (and sub-folders)
    within it.
*/
class SecurityScopedFileEngine : public QFSFileEngine
{
    Q_DECLARE_PRIVATE(QFSFileEngine)
public:
    SecurityScopedFileEngine(const QString &fileName, NSURL *securityScopedUrl)
        : QFSFileEngine(fileName)
        , m_securityScopedUrl([securityScopedUrl retain])
    {
        startAccessingSecurityScopedResource();
    }

    ~SecurityScopedFileEngine()
    {
        stopAccessingSecurityScopedResource();
        [m_securityScopedUrl release];
    }

    void setFileName(const QString &fileName) override
    {
        QFileSystemEntry entry(fileName);
        setFileEntry(std::move(entry));
    }

    void setFileEntry(QFileSystemEntry &&entry) override
    {
        // We can't rely on the new entry being accessible under the same
        // security scope as the original path, or even that the new path
        // is a security scoped resource, so stop access here, and start
        // access for the new resource below if needed.
        stopAccessingSecurityScopedResource();
        [m_securityScopedUrl release];
        m_securityScopedUrl = nil;

        const QString fileName = entry.filePath();
        QFSFileEngine::setFileEntry(std::move(entry));

        // The new path may not be a security scoped resource, but if it is
        // we need to establish access to it. The only way to do that is to
        // actually create an engine for it, including resolving bookmarks.
        auto newEngine = SecurityScopedFileEngineHandler::get()->create(fileName);
        if (auto *engine = dynamic_cast<SecurityScopedFileEngine*>(newEngine.get())) {
            m_securityScopedUrl = [engine->m_securityScopedUrl retain];
            startAccessingSecurityScopedResource();
        }
    }

private:
    void startAccessingSecurityScopedResource()
    {
        if ([m_securityScopedUrl startAccessingSecurityScopedResource]) {
            qCDebug(lcSecEngine) << "Started accessing" << m_securityScopedUrl.path
                                 << "on behalf of" << fileName(DefaultName);

            m_securityScopeWasReachable = securityScopeIsReachable();
        } else {
            qCWarning(lcSecEngine) << "Unexpectedly using security scoped"
                << "file engine for" << m_securityScopedUrl.path
                << "on behalf of" << fileName(DefaultName)
                << "without needing scoped access";
        }
    }

    void stopAccessingSecurityScopedResource()
    {
        if (!m_securityScopeWasReachable && securityScopeIsReachable()) {
            // The security scoped URL didn't exist when we first started
            // accessing it, but it does now, so persist a bookmark for it.
            qCDebug(lcSecEngine) << "Security scoped resource has been created. Saving bookmark.";
            SecurityScopedFileEngineHandler::get()->saveBookmark(m_securityScopedUrl);
        }

        // Note: Stopping access is a no-op if we didn't have access
        [m_securityScopedUrl stopAccessingSecurityScopedResource];
        qCDebug(lcSecEngine) << "Stopped accessing" << m_securityScopedUrl.path
                             << "on behalf of" << fileName(DefaultName);
    }

    bool securityScopeIsReachable() const
    {
        return checkIfResourceIsReachable(m_securityScopedUrl);
    }

    // See note above about relationship to fileName
    NSURL *m_securityScopedUrl = nullptr;
    bool m_securityScopeWasReachable = false;
};

// ----------------------------------------------------------------------

SecurityScopedFileEngineHandler::SecurityScopedFileEngineHandler()
{
    QMacAutoReleasePool pool;

    NSURL *savedBookmarks = bookmarksFile();
    if ([NSFileManager.defaultManager fileExistsAtPath:savedBookmarks.path]) {
        NSError *error = nullptr;
        m_bookmarks = [[NSDictionary dictionaryWithContentsOfURL:savedBookmarks
            error:&error] mutableCopy];

        if (error) {
            qCWarning(lcSecEngine) << "Failed to load bookmarks from"
                 << savedBookmarks << ":" << error;
        } else {
            qCInfo(lcSecEngine) << "Loaded existing bookmarks for" << m_bookmarks.allKeys;
        }
    }

    if (!m_bookmarks)
        m_bookmarks = [NSMutableDictionary new];
}

SecurityScopedFileEngineHandler::~SecurityScopedFileEngineHandler()
{
    [m_bookmarks release];
}

void SecurityScopedFileEngineHandler::registerPossiblySecurityScopedURL(NSURL *url)
{
    QMacAutoReleasePool pool;

    // Start accessing the resource, to check if it's security scoped,
    // and allow us to create a bookmark for it on both macOS and iOS.
    if (![url startAccessingSecurityScopedResource])
        return; // All good, not security scoped

    if (checkIfResourceIsReachable(url)) {
        // We can access the resource, which means it exists, so we can
        // create a persistent bookmark for it right away. We want to do
        // this as soon as possible, so that if the app is terminated the
        // user can continue working on the file without the app needing
        // to ask for access again via a file dialog.
        saveBookmark(url);
    } else {
        // The file isn't accessible, likely because it doesn't exist.
        // As we can only create security scoped bookmarks for files
        // that exist we store the URL itself for now, and save it to
        // a bookmark later when we detect that the file has been created.
        qCInfo(lcSecEngine) << "Resource is not reachable."
                            << "Registering URL" << url << "instead";
        QWriteLocker locker(&m_bookmarkLock);
        m_bookmarks[cacheKeyForUrl(url)] = url;
    }

    // Balance access from above
    [url stopAccessingSecurityScopedResource];

#if defined(Q_OS_MACOS)
    // On macOS, unlike iOS, URLs from file dialogs, etc, come with implicit
    // access already, and we are expected to balance this access with an
    // explicit stopAccessingSecurityScopedResource. We release the last
    // access here to unify the behavior between macOS and iOS, and then
    // leave it up to the SecurityScopedFileEngine to regain access, where
    // we know the lifetime of resource use, and when to release access.
    [url stopAccessingSecurityScopedResource];
#endif
}

std::unique_ptr<QAbstractFileEngine> SecurityScopedFileEngineHandler::create(const QString &fileName) const
{
    QMacAutoReleasePool pool;

    static thread_local bool recursionGuard = false;
    if (recursionGuard)
        return nullptr;

    if (fileName.isEmpty())
        return nullptr;

    QFileSystemEntry fileSystemEntry(fileName);
    QFileSystemMetaData metaData;

    {
        // Check if there's another engine that claims to handle the given file name.
        // This covers non-QFSFileEngines like QTemporaryFileEngine, and QResourceFileEngine.
        // If there isn't one, we'll get nullptr back, and know that we can access the
        // file via our special QFSFileEngine.
        QScopedValueRollback<bool> rollback(recursionGuard, true);
        if (auto engine = QFileSystemEngine::createLegacyEngine(fileSystemEntry, metaData)) {
            // Shortcut the logic of the createLegacyEngine call we're in by
            // just returning this engine now.
            qCDebug(lcSecEngine) << "Preferring non-QFSFileEngine engine"
                                 << engine.get() << "for" << fileName;
            return engine;
        }
    }

    // We're mapping the file name to existing bookmarks below, so make sure
    // we use as close as we can get to the canonical path. For files that
    // do not exist we fall back to the cleaned absolute path.
    auto canonicalEntry = QFileSystemEngine::canonicalName(fileSystemEntry, metaData);
    if (canonicalEntry.isEmpty())
        canonicalEntry = QFileSystemEngine::absoluteName(fileSystemEntry);

    if (canonicalEntry.isRelative()) {
        // We try to map relative paths to absolute above, but doing so requires
        // knowledge of the current working directory, which we only have if the
        // working directory has already started access through other means. We
        // can't explicitly start access of the working directory here, as doing
        // so requires its name, which we can't get from getcwd() without access.
        // Fortunately all of the entry points of security scoped URLs such as
        // file dialogs or drag-and-drop give us absolute paths, and APIs like
        // QDir::filePath() will construct absolute URLs without needing the
        // current working directory.
        qCWarning(lcSecEngine) << "Could not resolve" << fileSystemEntry.filePath()
                               << "against current working working directory";
        return nullptr;
    }

    // Clean the path as well, to remove any trailing slashes for directories
    QString filePath = QDir::cleanPath(canonicalEntry.filePath());

    // Files inside the sandbox container can always be accessed directly
    static const QString sandboxRoot = QString::fromNSString(NSHomeDirectory());
    if (filePath.startsWith(sandboxRoot))
        return nullptr;

    // The same applies to files inside the application's own bundle
    static const QString bundleRoot = QString::fromNSString(NSBundle.mainBundle.bundlePath);
    if (filePath.startsWith(bundleRoot))
        return nullptr;

    qCDebug(lcSecEngine) << "Looking up bookmark for" << filePath << "based on incoming fileName" << fileName;

    // Check if we have a persisted bookmark for this fileName, or
    // any of its containing directories (which will give us access
    // to the file).
    QReadLocker locker(&m_bookmarkLock);
    auto *cacheKey = cacheKeyForPath(filePath);
    NSObject *bookmarkData = nullptr;
    while (cacheKey.length > 1) {
        bookmarkData = m_bookmarks[cacheKey];
        if (bookmarkData)
            break;
        cacheKey = [cacheKey stringByDeletingLastPathComponent];
    }

    // We didn't find a bookmark, so there's no point in trying to manage
    // this file via a SecurityScopedFileEngine.
    if (!bookmarkData) {
        qCDebug(lcSecEngine) << "No bookmark found. Falling back to QFSFileEngine.";
        return nullptr;
    }

    NSURL *securityScopedUrl = nullptr;
    if ([bookmarkData isKindOfClass:NSURL.class]) {
        securityScopedUrl = static_cast<NSURL*>(bookmarkData);
    } else {
        NSError *error = nullptr;
        BOOL bookmarkDataIsStale = NO;
        securityScopedUrl = [NSURL URLByResolvingBookmarkData:static_cast<NSData*>(bookmarkData)
            options:
            #if defined(Q_OS_MACOS)
                NSURLBookmarkResolutionWithSecurityScope
            #else
                // iOS bookmarks are always security scoped, and we
                // don't need or want any of the other options.
                NSURLBookmarkResolutionOptions(0)
            #endif
            relativeToURL:nil /* app-scoped bookmark */
            bookmarkDataIsStale:&bookmarkDataIsStale
            error:&error];

        if (!securityScopedUrl || error) {
            qCWarning(lcSecEngine) << "Failed to resolve bookmark data for"
                                   << fileName << ":" << error;
            return nullptr;
        }

        if (bookmarkDataIsStale) {
            // This occurs when for example the file has been renamed, moved,
            // or deleted. Normally this would be the place to update the
            // bookmark to point to the new location, but Qt clients may not
            // be prepared for QFiles changing their file-names under their
            // feet so we treat it as a missing file.
            qCDebug(lcSecEngine) << "Bookmark for" << cacheKey << "was stale";
            locker.unlock();
            QWriteLocker writeLocker(&m_bookmarkLock);
            [m_bookmarks removeObjectForKey:cacheKey];
            auto *mutableThis = const_cast<SecurityScopedFileEngineHandler*>(this);
            mutableThis->saveBookmarks();
            return nullptr;
        }
    }

    qCInfo(lcSecEngine) << "Resolved security scope" << securityScopedUrl
                        << "for path" << filePath;
    return std::make_unique<SecurityScopedFileEngine>(fileName, securityScopedUrl);
}

/*
    Create an app-scoped bookmark, and store it in our persistent cache.

    We do this so that the user can continue accessing the file even after
    application restarts.

    Storing the bookmarks to disk (inside the sandbox) is safe, as only the
    app that created the app-scoped bookmarks can obtain access to the file
    system resource that the URL points to. Specifically, a bookmark created
    with security scope fails to resolve if the caller does not have the same
    code signing identity as the caller that created the bookmark.
*/
void SecurityScopedFileEngineHandler::saveBookmark(NSURL *url)
{
    NSError *error = nullptr;
    NSData *bookmarkData = [url bookmarkDataWithOptions:
        #if defined(Q_OS_MACOS)
            NSURLBookmarkCreationWithSecurityScope
        #else
            // iOS bookmarks are always security scoped, and we
            // don't need or want any of the other options.
            NSURLBookmarkCreationOptions(0)
        #endif
        includingResourceValuesForKeys:nil
        relativeToURL:nil /* app-scoped bookmark */
        error:&error];

    if (bookmarkData) {
        QWriteLocker locker(&m_bookmarkLock);
        NSString *cacheKey = cacheKeyForUrl(url);
        qCInfo(lcSecEngine)
            << (m_bookmarks[cacheKey] ? "Updating" : "Registering")
            << "bookmark for" << cacheKey;
        m_bookmarks[cacheKey] = bookmarkData;
        saveBookmarks();
    } else {
        qCWarning(lcSecEngine) << "Failed to create bookmark data for" << url << error;
    }
}

/*
    Saves the bookmarks cache to disk.

    We do this preemptively whenever we create a bookmark, to ensure
    the file can be accessed later on even if the app crashes.
*/
void SecurityScopedFileEngineHandler::saveBookmarks()
{
    QMacAutoReleasePool pool;

    NSError *error = nullptr;
    NSURL *bookmarksFilePath = bookmarksFile();
    [NSFileManager.defaultManager
        createDirectoryAtURL:[bookmarksFilePath URLByDeletingLastPathComponent]
                 withIntermediateDirectories:YES attributes:nil error:&error];
    if (error) {
        qCWarning(lcSecEngine) << "Failed to create bookmarks path:" << error;
        return;
    }
    [m_bookmarks writeToURL:bookmarksFile() error:&error];
    if (error) {
        qCWarning(lcSecEngine) << "Failed to save bookmarks to"
            << bookmarksFile() << ":" << error;
    }
}

NSURL *SecurityScopedFileEngineHandler::bookmarksFile() const
{
    NSURL *appSupportDir = [[NSFileManager.defaultManager URLsForDirectory:
        NSApplicationSupportDirectory inDomains:NSUserDomainMask] firstObject];
    return [appSupportDir URLByAppendingPathComponent:@"SecurityScopedBookmarks.plist"];
}

NSString *SecurityScopedFileEngineHandler::cacheKeyForUrl(NSURL *url)
{
    return cacheKeyForPath(QString::fromNSString(url.path));
}

NSString *SecurityScopedFileEngineHandler::cacheKeyForPath(const QString &path)
{
    auto normalized = path.normalized(QString::NormalizationForm_D);
    // We assume the file paths we get via file dialogs and similar
    // are already canonical, but clean it just in case.
    return QDir::cleanPath(normalized).toNSString();
}

QT_END_NAMESPACE
