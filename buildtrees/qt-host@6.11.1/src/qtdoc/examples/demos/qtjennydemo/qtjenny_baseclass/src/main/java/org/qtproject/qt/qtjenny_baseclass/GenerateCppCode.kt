package org.qtproject.qt.qtjenny_baseclass

import org.qtproject.qt.qtjenny.NativeProxy
import org.qtproject.qt.qtjenny.NativeClass
import org.qtproject.qt.qtjenny.NativeProxyForClasses

import android.database.ContentObserver

@NativeClass
@NativeProxy(allMethods = false, allFields = false)
@NativeProxyForClasses(namespace = "android::database", classes = [ContentObserver::class])
class GenerateCppCode {
}
