package org.qtproject.qt.qtjenny_callback

import org.qtproject.qt.qtjenny.NativeProxy
import org.qtproject.qt.qtjenny.NativeClass
import org.qtproject.qt.qtjenny.NativeProxyForClasses

import android.os.Handler
import android.database.ContentObserver

@NativeClass
@NativeProxy(allMethods = false, allFields = false)
@NativeProxyForClasses(namespace = "android::os", classes = [Handler.Callback::class])
@NativeProxyForClasses(namespace = "android::database", classes = [ContentObserver::class])
class GenerateCppCode {
}
