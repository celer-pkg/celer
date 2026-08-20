package org.qtproject.qt.qtjenny_general

import android.app.Activity
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.ContentResolver
import android.content.Context
import android.content.Intent
import android.media.AudioManager
import android.os.BatteryManager
import android.os.Handler
import android.os.Looper
import android.os.PowerManager
import android.os.VibrationEffect
import android.os.Vibrator
import android.os.VibratorManager
import android.provider.Settings
import android.view.Window
import android.view.WindowManager
import org.qtproject.qt.qtjenny.NativeProxy
import org.qtproject.qt.qtjenny.NativeClass
import org.qtproject.qt.qtjenny.NativeProxyForClasses

//! [1]
@NativeClass
@NativeProxy(allMethods = false, allFields = false)
@NativeProxyForClasses(namespace = "android::os", classes = [BatteryManager::class, VibratorManager::class,
    Vibrator::class, VibrationEffect::class, Context::class, PowerManager::class, PowerManager.WakeLock::class,
    Handler::class, Looper::class])
@NativeProxyForClasses(namespace = "android::view", classes = [Window::class, WindowManager.LayoutParams::class])
@NativeProxyForClasses(namespace = "android::media", classes = [AudioManager::class])
@NativeProxyForClasses(namespace = "android::drawable", classes = [android.R.drawable::class])
@NativeProxyForClasses(namespace = "android::app", classes = [Activity::class, Notification::class,
    Notification.Builder::class, NotificationChannel::class, NotificationManager::class])
@NativeProxyForClasses(namespace = "android::provider", classes = [Settings.Global::class, Settings.System::class,
    Settings::class])
@NativeProxyForClasses(namespace = "android::content", classes = [Intent::class, ContentResolver::class])
//! [1]
class GenerateCppCode {
}
