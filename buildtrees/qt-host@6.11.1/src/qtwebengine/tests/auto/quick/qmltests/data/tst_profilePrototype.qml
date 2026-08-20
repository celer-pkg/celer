// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtWebEngine
import Qt.labs.platform
import Test.util

Item {
    WebEngineProfilePrototype { id: otrProfile }
    WebEngineProfilePrototype { id: nonOtrProfile; storageName: 'Test' }

    TempDir {
        id: tempDir
    }

    function getPath(path, offset = 1) { return path.substr(path.indexOf(':') + offset, path.length) }
    property string appDataLocation: getPath(getPath(StandardPaths.writableLocation(StandardPaths.AppDataLocation).toString(), 3))
    property string cacheLocation: getPath(getPath(StandardPaths.writableLocation(StandardPaths.CacheLocation).toString(), 3))
    property string downloadLocation: getPath(getPath(StandardPaths.writableLocation(StandardPaths.DownloadLocation).toString(), 3))

    TestCase {
        id: profileProtoTypeTest
        name: "ProfilePrototype"

        function test_otrProfile() {
            let p = otrProfile.instance();
            verify(p.offTheRecord)

            compare(p.storageName, '')
            compare(p.cachePath, '')
            compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/OffTheRecord')
            compare(p.httpCacheType, WebEngineProfile.MemoryHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, WebEngineProfile.NoPersistentCookies)
            compare(p.persistentPermissionsPolicy, WebEngineProfile.StoreInMemory)

            compare(getPath(p.downloadPath), downloadLocation)
            compare(p.httpAcceptLanguage, '')
            verify(p.httpUserAgent !== '')
            compare(p.spellCheckEnabled, false)
            compare(p.spellCheckLanguages, [])

            compare(p.userScripts.collection, [])
        }

        function test_nonOtrProfile() {
            let p = nonOtrProfile.instance()
            compare(p.storageName, 'Test')
            verify(!p.offTheRecord)

            compare(getPath(p.downloadPath), downloadLocation)
            compare(p.httpAcceptLanguage, '')
            verify(p.httpUserAgent !== '')
            compare(p.spellCheckEnabled, false)
            compare(p.spellCheckLanguages, [])

            compare(p.userScripts.collection, [])

            compare(p.storageName, 'Test')
            compare(getPath(p.cachePath), cacheLocation + '/QtWebEngine/' + p.storageName)
            compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/' + p.storageName)

            compare(p.httpCacheType, WebEngineProfile.DiskHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, WebEngineProfile.AllowPersistentCookies)
            compare(p.persistentPermissionsPolicy, WebEngineProfile.StoreOnDisk)
        }

        function test_persistentStoragePath() {
            var persistentStoragePath = tempDir.path()
            var profileProtoType = Qt.createQmlObject("
                    import QtWebEngine\n
                    WebEngineProfilePrototype {\n
                            storageName: 'PersistentStoragePathTest'\n
                            persistentStoragePath: '" + persistentStoragePath + "'\n
                        }", profileProtoTypeTest);

            let p = profileProtoType.instance()
            compare(p.storageName, 'PersistentStoragePathTest')
            verify(!p.offTheRecord)
            compare(getPath(p.cachePath), cacheLocation + '/QtWebEngine/' + p.storageName)
            compare(p.persistentStoragePath, persistentStoragePath)
            compare(p.httpCacheType, WebEngineProfile.DiskHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, WebEngineProfile.AllowPersistentCookies)
            compare(p.persistentPermissionsPolicy, WebEngineProfile.StoreOnDisk)
        }

        function test_cachePath() {
            var cachePath = tempDir.path()
            var profileProtoType = Qt.createQmlObject("
                    import QtWebEngine\n
                    WebEngineProfilePrototype {\n
                            storageName: 'CacheTest'\n
                            cachePath: '" + cachePath + "'\n
                        }", profileProtoTypeTest);

            let p = profileProtoType.instance()
            compare(p.storageName, 'CacheTest')
            verify(!p.offTheRecord)
            compare(p.cachePath, cachePath)
            compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/' + p.storageName)
            compare(p.httpCacheType, WebEngineProfile.DiskHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, WebEngineProfile.AllowPersistentCookies)
            compare(p.persistentPermissionsPolicy, WebEngineProfile.StoreOnDisk)
        }

        function test_httpCacheType_data() {
            return [
                   { tag: "NoCache", cacheType: WebEngineProfile.NoCache, isOffTheRecord: true },
                   { tag: "MemoryHttpCache", cacheType: WebEngineProfile.MemoryHttpCache, isOffTheRecord: true },
                   { tag: "NoCacheDiskBased", cacheType: WebEngineProfile.NoCache, isOffTheRecord: false },
                   { tag: "DiskHttpCache", cacheType: WebEngineProfile.DiskHttpCache, isOffTheRecord: false },
            ];
        }

        function test_httpCacheType(row) {
            var cacheType = row.cacheType

            var isOffTheRecord = row.isOffTheRecord;

            var storageName = ""
            if (!isOffTheRecord)
                storageName = row.tag;

            var httpCacheType;
            if (cacheType === WebEngineProfile.NoCache)
                httpCacheType = "WebEngineProfile.NoCache"
            else if (cacheType === WebEngineProfile.MemoryHttpCache)
                httpCacheType = "WebEngineProfile.MemoryHttpCache"
            else if (cacheType === WebEngineProfile.DiskHttpCache)
                 httpCacheType = "WebEngineProfile.DiskHttpCache"

            var profileProtoType = Qt.createQmlObject("
                    import QtWebEngine\n
                    WebEngineProfilePrototype {\n
                            storageName: '" + storageName + "'\n
                            httpCacheType: " + httpCacheType + "\n
                        }", profileProtoTypeTest);

            let p = profileProtoType.instance()
            compare(p.storageName, storageName)
            verify(p.offTheRecord === isOffTheRecord)

            if (isOffTheRecord) {
                compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/OffTheRecord')
                compare(p.cachePath, '')
                compare(p.persistentCookiesPolicy, WebEngineProfile.NoPersistentCookies)
                compare(p.persistentPermissionsPolicy, WebEngineProfile.StoreInMemory)
            } else {
                compare(getPath(p.cachePath), cacheLocation + '/QtWebEngine/' + p.storageName)
                compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/' + p.storageName)
                compare(p.persistentCookiesPolicy, WebEngineProfile.AllowPersistentCookies)
                compare(p.persistentPermissionsPolicy, WebEngineProfile.StoreOnDisk)
            }
            compare(p.httpCacheType,cacheType)
            compare(p.httpCacheMaximumSize, 0)
        }

        function test_persistentCookiesPolicy_data() {
            return [
                   { tag: "noPersistentCookies", persistentCookiesPolicy: WebEngineProfile.NoPersistentCookies, isOffTheRecord: true },
                   { tag: "allowPersistentCookies", persistentCookiesPolicy: WebEngineProfile.AllowPersistentCookies, isOffTheRecord: true },
                   { tag: "allowPersistentCookiesDiskBased", persistentCookiesPolicy: WebEngineProfile.AllowPersistentCookies, isOffTheRecord: false },
                   { tag: "forcePersistentCookiesDiskBased", persistentCookiesPolicy: WebEngineProfile.ForcePersistentCookies, isOffTheRecord: false },
                   { tag: "noPersistentCookiesDiskBased", persistentCookiesPolicy: WebEngineProfile.NoPersistentCookies, isOffTheRecord: false },
            ];
        }

        function test_persistentCookiesPolicy(row) {
            var persistentCookiesPolicy = row.persistentCookiesPolicy
            var isOffTheRecord = row.isOffTheRecord

            var policy;
            if (persistentCookiesPolicy === WebEngineProfile.NoPersistentCookies)
                policy = "WebEngineProfile.NoPersistentCookies"
            else if (persistentCookiesPolicy === WebEngineProfile.AllowPersistentCookies)
                policy = "WebEngineProfile.AllowPersistentCookies"
            else if (persistentCookiesPolicy === WebEngineProfile.ForcePersistentCookies)
                 policy = "WebEngineProfile.ForcePersistentCookies"

            var storageName = '';
            if (!isOffTheRecord)
                storageName = row.tag
            var profileProtoType = Qt.createQmlObject("
                    import QtWebEngine\n
                    WebEngineProfilePrototype {\n
                            storageName: '" + storageName +"'\n
                            persistentCookiesPolicy: " + policy + "\n
                        }", profileProtoTypeTest);

            let p = profileProtoType.instance()
            compare(p.storageName, storageName)
            verify(p.offTheRecord === isOffTheRecord)
            if (isOffTheRecord) {
                compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/OffTheRecord')
                compare(p.cachePath, '')
            } else {
                compare(getPath(p.cachePath), cacheLocation + '/QtWebEngine/' + p.storageName)
                compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/' + p.storageName)
            }
            compare(p.httpCacheType, isOffTheRecord ? WebEngineProfile.MemoryHttpCache : WebEngineProfile.DiskHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, isOffTheRecord ? WebEngineProfile.NoPersistentCookies : persistentCookiesPolicy)
            compare(p.persistentPermissionsPolicy, isOffTheRecord ? WebEngineProfile.StoreInMemory :WebEngineProfile.StoreOnDisk)
        }

        function test_httpCacheMaximumSize() {
            var httpCacheMaximumSize = 100
            var profileProtoType = Qt.createQmlObject("
                    import QtWebEngine\n
                    WebEngineProfilePrototype {\n
                            storageName: 'HttpCacheSizeTest'\n
                            httpCacheMaximumSize: " + httpCacheMaximumSize + "\n
                        }", profileProtoTypeTest);

            let p = profileProtoType.instance()
            compare(p.storageName, 'HttpCacheSizeTest')
            verify(!p.offTheRecord)
            compare(getPath(p.cachePath), cacheLocation + '/QtWebEngine/' + p.storageName)
            compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/' + p.storageName)
            compare(p.httpCacheType, WebEngineProfile.DiskHttpCache)
            compare(p.httpCacheMaximumSize, httpCacheMaximumSize)
            compare(p.persistentCookiesPolicy, WebEngineProfile.AllowPersistentCookies)
            compare(p.persistentPermissionsPolicy, WebEngineProfile.StoreOnDisk)
        }

        function test_persistentPermissionsPolicy_data() {
            return [
                   { tag: "storeInMemory", persistentPermissionsPolicy: WebEngineProfile.StoreInMemory, isOffTheRecord: true },
                   { tag: "askEveryTime", persistentPermissionsPolicy: WebEngineProfile.AskEveryTime, isOffTheRecord: true },
                   { tag: "storeOnDisk", persistentPermissionsPolicy: WebEngineProfile.StoreOnDisk, isOffTheRecord: true },
                   { tag: "storeInMemoryDiskBased", persistentPermissionsPolicy: WebEngineProfile.StoreInMemory, isOffTheRecord: false },
                   { tag: "askEveryTimeDiskBased", persistentPermissionsPolicy: WebEngineProfile.AskEveryTime, isOffTheRecord: false },
                   { tag: "storeOnDiskDiskBased", persistentPermissionsPolicy: WebEngineProfile.StoreOnDisk, isOffTheRecord: false },
            ];
        }

        function test_persistentPermissionsPolicy(row) {
            var persistentPermissionsPolicy = row.persistentPermissionsPolicy
            var isOffTheRecord = row.isOffTheRecord

            var policy;
            if (persistentPermissionsPolicy === WebEngineProfile.StoreInMemory)
                policy = "WebEngineProfile.StoreInMemory"
            else if (persistentPermissionsPolicy === WebEngineProfile.AskEveryTime)
                policy = "WebEngineProfile.AskEveryTime"
            else if (persistentPermissionsPolicy === WebEngineProfile.StoreOnDisk)
                 policy = "WebEngineProfile.StoreOnDisk"

            var storageName = '';
            if (!isOffTheRecord)
                storageName = row.tag
            var profileProtoType = Qt.createQmlObject("
                    import QtWebEngine\n
                    WebEngineProfilePrototype {\n
                            storageName: '" + storageName +"'\n
                            persistentPermissionsPolicy: " + policy + "\n
                        }", profileProtoTypeTest);

            let p = profileProtoType.instance()
            compare(p.storageName, storageName)
            verify(p.offTheRecord === isOffTheRecord)
            if (isOffTheRecord) {
                compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/OffTheRecord')
                compare(p.cachePath, '')
            } else {
                compare(getPath(p.cachePath), cacheLocation + '/QtWebEngine/' + p.storageName)
                compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/' + p.storageName)
            }
            compare(p.httpCacheType, isOffTheRecord ? WebEngineProfile.MemoryHttpCache : WebEngineProfile.DiskHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentPermissionsPolicy, isOffTheRecord && persistentPermissionsPolicy === WebEngineProfile.StoreOnDisk
                    ? WebEngineProfile.StoreInMemory : persistentPermissionsPolicy)
        }

        function test_useSameDataPathForProfiles() {
            var profileProtoType = Qt.createQmlObject("
                   import QtWebEngine\n
                    WebEngineProfilePrototype {\n
                            storageName: 'SamePathTest'\n
                        }", profileProtoTypeTest);

            let p = profileProtoType.instance()
            compare(p.storageName, 'SamePathTest')
            verify(!p.offTheRecord)
            compare(getPath(p.cachePath), cacheLocation + '/QtWebEngine/' + p.storageName)
            compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/' + p.storageName)
            compare(p.httpCacheType, WebEngineProfile.DiskHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, WebEngineProfile.AllowPersistentCookies)

            var secondProfileProtoType = Qt.createQmlObject("
                   import QtWebEngine\n
                    WebEngineProfilePrototype {\n
                            storageName: 'SamePathTest'\n
                        }", profileProtoTypeTest);

            let secondProfile = secondProfileProtoType.instance()
            verify(!secondProfile)
        }
    }
}
