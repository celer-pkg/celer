// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "jnipositioning.h"
#include "qgeopositioninfosource_android_p.h"
#include "qgeosatelliteinfosource_android_p.h"
#include <QtPositioning/QGeoPositionInfo>
#include <QtCore/QDateTime>
#include <QtCore/QMap>
#include <QtCore/QRandomGenerator>
#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#include <QtCore/QLoggingCategory>
#include <QtCore/QPermission>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimeZone>
#include <QtCore/QSet>
#include <android/log.h>

Q_DECLARE_JNI_CLASS(QtPositioning, "org/qtproject/qt/android/positioning/QtPositioning")
Q_DECLARE_JNI_CLASS(GnssStatus, "android/location/GnssStatus")
Q_DECLARE_JNI_CLASS(Location, "android/location/Location")

using namespace Qt::StringLiterals;

template<typename T>
class GlobalClassRefWrapper
{
public:
    GlobalClassRefWrapper() = default;
    ~GlobalClassRefWrapper()
    {
        if (m_classRef) {
            QJniEnvironment env;
            if (env.jniEnv())
                env->DeleteGlobalRef(m_classRef);
        }
    }

    bool init()
    {
        QJniEnvironment env;
        if (env.jniEnv()) {
            if (m_classRef) {
                env->DeleteGlobalRef(m_classRef);
                m_classRef = nullptr;
            }

            m_classRef = env.findClass<T>(); // it returns global ref!
        }
        return m_classRef != nullptr;
    }

    jclass operator()() { return m_classRef; }

private:
    jclass m_classRef = nullptr;
};

static GlobalClassRefWrapper<QtJniTypes::QtPositioning> positioningClass;

static jmethodID providerListMethodId;
static jmethodID lastKnownPositionMethodId;
static jmethodID startUpdatesMethodId;
static jmethodID stopUpdatesMethodId;
static jmethodID requestUpdateMethodId;
static jmethodID startSatelliteUpdatesMethodId;

static const char logTag[] = "qt.positioning.android";
static const char methodErrorMsg[] = "Can't find method \"%s%s\"";

Q_LOGGING_CATEGORY(lcPositioning, logTag)

namespace {

/*!
    \internal
    This class encapsulates satellite system types, as defined by Android
    GnssStatus API. Initialize during JNI_OnLoad() by the init() method, from
    the Java side, rather than hard-coding.
*/
class ConstellationMapper
{
public:
    static bool init()
    {
        m_gnssStatusObject = nullptr;
        if (QNativeInterface::QAndroidApplication::sdkVersion() > 23) {
            m_gnssStatusObject = QJniEnvironment().findClass<QtJniTypes::GnssStatus>();
            if (!m_gnssStatusObject)
                return false;
        }
        // no need to query it for API level <= 23
        return true;
    }

    static QGeoSatelliteInfo::SatelliteSystem toSatelliteSystem(int constellationType)
    {
        if (!m_gnssStatusObject)
            return QGeoSatelliteInfo::Undefined;

        static const int gps =
                QJniObject::getStaticField<jint>(m_gnssStatusObject, "CONSTELLATION_GPS");
        static const int glonass =
                QJniObject::getStaticField<jint>(m_gnssStatusObject, "CONSTELLATION_GLONASS");
        static const int galileo =
                QJniObject::getStaticField<jint>(m_gnssStatusObject, "CONSTELLATION_GALILEO");
        static const int beidou =
                QJniObject::getStaticField<jint>(m_gnssStatusObject, "CONSTELLATION_BEIDOU");
        static const int qzss =
                QJniObject::getStaticField<jint>(m_gnssStatusObject, "CONSTELLATION_QZSS");

        if (constellationType == gps) {
            return QGeoSatelliteInfo::GPS;
        } else if (constellationType == glonass) {
            return QGeoSatelliteInfo::GLONASS;
        } else if (constellationType == galileo) {
            return QGeoSatelliteInfo::GALILEO;
        } else if (constellationType == beidou) {
            return QGeoSatelliteInfo::BEIDOU;
        } else if (constellationType == qzss){
            return QGeoSatelliteInfo::QZSS;
        } else {
            qCWarning(lcPositioning) << "Unknown satellite system" << constellationType;
            return QGeoSatelliteInfo::Undefined;
        }
    }

private:
    static jclass m_gnssStatusObject;
};

jclass ConstellationMapper::m_gnssStatusObject = nullptr;

} // anonymous namespace

namespace AndroidPositioning {
    typedef QMap<int, QGeoPositionInfoSourceAndroid * > PositionSourceMap;
    typedef QMap<int, QGeoSatelliteInfoSourceAndroid * > SatelliteSourceMap;

    Q_GLOBAL_STATIC(PositionSourceMap, idToPosSource)

    Q_GLOBAL_STATIC(SatelliteSourceMap, idToSatSource)

    int registerPositionInfoSource(QObject *obj)
    {
        int key = -1;
        if (obj->inherits("QGeoPositionInfoSource")) {
            QGeoPositionInfoSourceAndroid *src = qobject_cast<QGeoPositionInfoSourceAndroid *>(obj);
            Q_ASSERT(src);
            do {
                key = qAbs(int(QRandomGenerator::global()->generate()));
            } while (idToPosSource()->contains(key));

            idToPosSource()->insert(key, src);
        } else if (obj->inherits("QGeoSatelliteInfoSource")) {
            QGeoSatelliteInfoSourceAndroid *src = qobject_cast<QGeoSatelliteInfoSourceAndroid *>(obj);
            Q_ASSERT(src);
            do {
                key = qAbs(int(QRandomGenerator::global()->generate()));
            } while (idToSatSource()->contains(key));

            idToSatSource()->insert(key, src);
        }

        return key;
    }

    void unregisterPositionInfoSource(int key)
    {
        if (idToPosSource.exists())
            idToPosSource->remove(key);

        if (idToSatSource.exists())
            idToSatSource->remove(key);
    }

    enum PositionProvider
    {
        PROVIDER_GPS = 0,
        PROVIDER_NETWORK = 1,
        PROVIDER_PASSIVE = 2
    };


    QGeoPositionInfoSource::PositioningMethods availableProviders()
    {
        QGeoPositionInfoSource::PositioningMethods ret = QGeoPositionInfoSource::NoPositioningMethods;
        QJniEnvironment env;
        if (!env.jniEnv())
            return ret;
        QJniObject jniProvidersObj =
                QJniObject::callStaticMethod<jobject>(positioningClass(), providerListMethodId);
        jintArray jProviders = jniProvidersObj.object<jintArray>();
        if (!jProviders) {
            // Work-around for QTBUG-116645
            __android_log_print(ANDROID_LOG_INFO, logTag, "Got null providers array!");
            return ret;
        }
        jint *providers = env->GetIntArrayElements(jProviders, nullptr);
        const int size = env->GetArrayLength(jProviders);
        for (int i = 0; i < size; i++) {
            switch (providers[i]) {
            case PROVIDER_GPS:
                ret |= QGeoPositionInfoSource::SatellitePositioningMethods;
                break;
            case PROVIDER_NETWORK:
                ret |= QGeoPositionInfoSource::NonSatellitePositioningMethods;
                break;
            case PROVIDER_PASSIVE:
                //we ignore as Qt doesn't have interface for it right now
                break;
            default:
                __android_log_print(ANDROID_LOG_INFO, logTag, "Unknown positioningMethod");
            }
        }

        env->ReleaseIntArrayElements(jProviders, providers, 0);

        return ret;
    }

    QGeoPositionInfo positionInfoFromJavaLocation(const jobject &location, bool useAltConverter)
    {
        QGeoPositionInfo info;

        QJniObject jniObject(location);
        if (!jniObject.isValid())
            return QGeoPositionInfo();

        const jdouble latitude = jniObject.callMethod<jdouble>("getLatitude");
        const jdouble longitude = jniObject.callMethod<jdouble>("getLongitude");

        QGeoCoordinate coordinate(latitude, longitude);

        // altitude
        jboolean attributeExists = jniObject.callMethod<jboolean>("hasAltitude");
        if (attributeExists) {
            const jdouble value = jniObject.callMethod<jdouble>("getAltitude");
            if (!qFuzzyIsNull(value))
                coordinate.setAltitude(value);
        }
        // MSL altitude, available in API Level 34+.
        // In API Level 34 it was available only if we manually added it.
        // In API Level 35 (and potentially later), it's automatically added
        // to the location object, so we need to use it *only* when the user
        // set the relevant plugin parameter.
        if (useAltConverter && QNativeInterface::QAndroidApplication::sdkVersion() >= 34) {
            attributeExists = jniObject.callMethod<jboolean>("hasMslAltitude");
            if (attributeExists) {
                const jdouble value = jniObject.callMethod<jdouble>("getMslAltitudeMeters");
                if (!qFuzzyIsNull(value))
                    coordinate.setAltitude(value);
            }
        }

        info.setCoordinate(coordinate);

        // time stamp
        const jlong timestamp = jniObject.callMethod<jlong>("getTime");
        info.setTimestamp(QDateTime::fromMSecsSinceEpoch(timestamp, QTimeZone::UTC));

        // horizontal accuracy
        attributeExists = jniObject.callMethod<jboolean>("hasAccuracy");
        if (attributeExists) {
            const jfloat accuracy = jniObject.callMethod<jfloat>("getAccuracy");
            if (!qFuzzyIsNull(accuracy))
                info.setAttribute(QGeoPositionInfo::HorizontalAccuracy, qreal(accuracy));
        }

        // vertical accuracy (available in API Level 26+)
        if (QNativeInterface::QAndroidApplication::sdkVersion() > 25) {
            attributeExists = jniObject.callMethod<jboolean>("hasVerticalAccuracy");
            if (attributeExists) {
                const jfloat accuracy = jniObject.callMethod<jfloat>("getVerticalAccuracyMeters");
                if (!qFuzzyIsNull(accuracy))
                    info.setAttribute(QGeoPositionInfo::VerticalAccuracy, qreal(accuracy));
            }
        }

        // ground speed
        attributeExists = jniObject.callMethod<jboolean>("hasSpeed");
        if (attributeExists) {
            const jfloat speed = jniObject.callMethod<jfloat>("getSpeed");
            if (!qFuzzyIsNull(speed))
                info.setAttribute(QGeoPositionInfo::GroundSpeed, qreal(speed));
        }

        // bearing
        attributeExists = jniObject.callMethod<jboolean>("hasBearing");
        if (attributeExists) {
            const jfloat bearing = jniObject.callMethod<jfloat>("getBearing");
            if (!qFuzzyIsNull(bearing))
                info.setAttribute(QGeoPositionInfo::Direction, qreal(bearing));

            // bearingAccuracy is available in API Level 26+
            if (QNativeInterface::QAndroidApplication::sdkVersion() > 25) {
                const jfloat bearingAccuracy =
                        jniObject.callMethod<jfloat>("getBearingAccuracyDegrees");
                if (!qFuzzyIsNull(bearingAccuracy))
                    info.setAttribute(QGeoPositionInfo::DirectionAccuracy, qreal(bearingAccuracy));
            }
        }

        return info;
    }

    using UniqueId = std::pair<int, int>;
    static UniqueId getUid(const QGeoSatelliteInfo &info)
    {
        return std::make_pair(static_cast<int>(info.satelliteSystem()),
                              info.satelliteIdentifier());
    }

    QList<QGeoSatelliteInfo> satelliteInfoFromJavaLocation(JNIEnv *jniEnv,
                                                           jobjectArray satellites,
                                                           QList<QGeoSatelliteInfo>* usedInFix)
    {
        QSet<UniqueId> uids;
        QList<QGeoSatelliteInfo> sats;
        jsize length = jniEnv->GetArrayLength(satellites);
        for (int i = 0; i<length; i++) {
            jobject element = jniEnv->GetObjectArrayElement(satellites, i);
            if (QJniEnvironment::checkAndClearExceptions(jniEnv)) {
                qCWarning(lcPositioning) << "Cannot process all satellite data due to exception.";
                break;
            }

            QJniObject jniObj = QJniObject::fromLocalRef(element);
            if (!jniObj.isValid())
                continue;

            QGeoSatelliteInfo info;

            // signal strength
            const jfloat snr = jniObj.callMethod<jfloat>("getSnr");
            info.setSignalStrength(int(snr));

            // ignore any satellite with no signal whatsoever
            if (qFuzzyIsNull(snr))
                continue;

            // prn
            const jint prn = jniObj.callMethod<jint>("getPrn");
            info.setSatelliteIdentifier(prn);

            if (prn >= 1 && prn <= 32)
                info.setSatelliteSystem(QGeoSatelliteInfo::GPS);
            else if (prn >= 65 && prn <= 96)
                info.setSatelliteSystem(QGeoSatelliteInfo::GLONASS);
            else if (prn >= 193 && prn <= 200)
                info.setSatelliteSystem(QGeoSatelliteInfo::QZSS);
            else if ((prn >= 201 && prn <= 235) || (prn >= 401 && prn <= 437))
                info.setSatelliteSystem(QGeoSatelliteInfo::BEIDOU);
            else if (prn >= 301 && prn <= 336)
                info.setSatelliteSystem(QGeoSatelliteInfo::GALILEO);

            // azimuth
            const jfloat azimuth = jniObj.callMethod<jfloat>("getAzimuth");
            info.setAttribute(QGeoSatelliteInfo::Azimuth, qreal(azimuth));

            // elevation
            const jfloat elevation = jniObj.callMethod<jfloat>("getElevation");
            info.setAttribute(QGeoSatelliteInfo::Elevation, qreal(elevation));

            // Used in fix - true if this satellite is actually used in
            // determining the position.
            const jboolean inFix = jniObj.callMethod<jboolean>("usedInFix");

            const UniqueId id = getUid(info);
            if (uids.contains(id))
                continue;

            sats.append(info);
            uids.insert(id);

            if (inFix)
                usedInFix->append(info);
        }

        return sats;
    }

    QList<QGeoSatelliteInfo> satelliteInfoFromJavaGnssStatus(jobject gnssStatus,
                                                             QList<QGeoSatelliteInfo>* usedInFix)
    {
        QJniObject jniStatus(gnssStatus);
        QList<QGeoSatelliteInfo> sats;
        QSet<UniqueId> uids;

        const int satellitesCount = jniStatus.callMethod<jint>("getSatelliteCount");
        for (int i = 0; i < satellitesCount; ++i) {
            QGeoSatelliteInfo info;

            // signal strength - this is actually a carrier-to-noise density,
            // but the values are very close to what was previously returned by
            // getSnr() method of the GpsSatellite API.
            const jfloat cn0 = jniStatus.callMethod<jfloat>("getCn0DbHz", i);
            info.setSignalStrength(static_cast<int>(cn0));

            // satellite system
            const jint constellationType =
                    jniStatus.callMethod<jint>("getConstellationType", i);
            info.setSatelliteSystem(ConstellationMapper::toSatelliteSystem(constellationType));

            // satellite identifier
            const jint svId = jniStatus.callMethod<jint>("getSvid", i);
            info.setSatelliteIdentifier(svId);

            // azimuth
            const jfloat azimuth = jniStatus.callMethod<jfloat>("getAzimuthDegrees", i);
            info.setAttribute(QGeoSatelliteInfo::Azimuth, static_cast<qreal>(azimuth));

            // elevation
            const jfloat elevation = jniStatus.callMethod<jfloat>("getElevationDegrees", i);
            info.setAttribute(QGeoSatelliteInfo::Elevation, static_cast<qreal>(elevation));

            // Used in fix - true if this satellite is actually used in
            // determining the position.
            const jboolean inFix = jniStatus.callMethod<jboolean>("usedInFix", i);

            const UniqueId id = getUid(info);
            if (uids.contains(id))
                continue;

            sats.append(info);
            uids.insert(id);

            if (inFix)
                usedInFix->append(info);
        }

        return sats;
    }

    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly,
                                       bool useAltitudeConverter)
    {
        QJniEnvironment env;
        if (!env.jniEnv())
            return QGeoPositionInfo();

        const auto accuracy = fromSatellitePositioningMethodsOnly
                ? AccuracyType::Precise
                : AccuracyType::Any;

        if (!hasPositioningPermissions(accuracy))
            return {};

        QJniObject locationObj = QJniObject::callStaticMethod<jobject>(
                positioningClass(), lastKnownPositionMethodId, fromSatellitePositioningMethodsOnly,
                useAltitudeConverter);
        jobject location = locationObj.object();
        if (location == nullptr)
            return QGeoPositionInfo();

        const QGeoPositionInfo info = positionInfoFromJavaLocation(location, useAltitudeConverter);

        return info;
    }

    inline int positioningMethodToInt(QGeoPositionInfoSource::PositioningMethods m)
    {
        int providerSelection = 0;
        if (m & QGeoPositionInfoSource::SatellitePositioningMethods)
            providerSelection |= 1;
        if (m & QGeoPositionInfoSource::NonSatellitePositioningMethods)
            providerSelection |= 2;

        return providerSelection;
    }

    static AccuracyTypes
    accuracyFromPositioningMethods(QGeoPositionInfoSource::PositioningMethods m)
    {
        AccuracyTypes types = AccuracyType::None;
        if (m & QGeoPositionInfoSource::NonSatellitePositioningMethods)
            types |= AccuracyType::Approximate;
        if (m & QGeoPositionInfoSource::SatellitePositioningMethods)
            types |= AccuracyType::Precise;
        return types;
    }

    QGeoPositionInfoSource::Error startUpdates(int androidClassKey)
    {
        QJniEnvironment env;
        if (!env.jniEnv())
            return QGeoPositionInfoSource::UnknownSourceError;

        QGeoPositionInfoSourceAndroid *source = AndroidPositioning::idToPosSource()->value(androidClassKey);

        if (source) {
            const auto preferredMethods = source->preferredPositioningMethods();
            const auto accuracy = accuracyFromPositioningMethods(preferredMethods);
            if (!hasPositioningPermissions(accuracy))
                return QGeoPositionInfoSource::AccessError;

            int errorCode = QJniObject::callStaticMethod<jint>(
                    positioningClass(), startUpdatesMethodId, androidClassKey,
                    positioningMethodToInt(preferredMethods),
                    source->updateInterval(), source->useAltitudeConverter());
            switch (errorCode) {
            case 0:
            case 1:
            case 2:
            case 3:
                return static_cast<QGeoPositionInfoSource::Error>(errorCode);
            default:
                break;
            }
        }

        return QGeoPositionInfoSource::UnknownSourceError;
    }

    //used for stopping regular and single updates
    void stopUpdates(int androidClassKey)
    {
        QJniObject::callStaticMethod<void>(positioningClass(), stopUpdatesMethodId,
                                           androidClassKey);
    }

    QGeoPositionInfoSource::Error requestUpdate(int androidClassKey, int timeout)
    {
        QJniEnvironment env;
        if (!env.jniEnv())
            return QGeoPositionInfoSource::UnknownSourceError;

        QGeoPositionInfoSourceAndroid *source = AndroidPositioning::idToPosSource()->value(androidClassKey);

        if (source) {
            const auto preferredMethods = source->preferredPositioningMethods();
            const auto accuracy = accuracyFromPositioningMethods(preferredMethods);
            if (!hasPositioningPermissions(accuracy))
                return QGeoPositionInfoSource::AccessError;

            int errorCode = QJniObject::callStaticMethod<jint>(
                    positioningClass(), requestUpdateMethodId, androidClassKey,
                    positioningMethodToInt(preferredMethods),
                    timeout, source->useAltitudeConverter());
            switch (errorCode) {
            case 0:
            case 1:
            case 2:
            case 3:
                return static_cast<QGeoPositionInfoSource::Error>(errorCode);
            default:
                break;
            }
        }
        return QGeoPositionInfoSource::UnknownSourceError;
    }

    QGeoSatelliteInfoSource::Error startSatelliteUpdates(int androidClassKey, bool isSingleRequest, int requestTimeout)
    {
        QJniEnvironment env;
        if (!env.jniEnv())
            return QGeoSatelliteInfoSource::UnknownSourceError;

        QGeoSatelliteInfoSourceAndroid *source = AndroidPositioning::idToSatSource()->value(androidClassKey);

        if (source) {
            // Satellite Info request does not make sense with Approximate
            // location permissions, so always check for Precise
            if (!hasPositioningPermissions(AccuracyType::Precise))
                return QGeoSatelliteInfoSource::AccessError;

            int interval = source->updateInterval();
            if (isSingleRequest)
                interval = requestTimeout;
            int errorCode = QJniObject::callStaticMethod<jint>(positioningClass(),
                                                               startSatelliteUpdatesMethodId,
                                                               androidClassKey, interval,
                                                               isSingleRequest);
            switch (errorCode) {
            case -1:
            case 0:
            case 1:
            case 2:
                return static_cast<QGeoSatelliteInfoSource::Error>(errorCode);
            default:
                qCWarning(lcPositioning)
                        << "startSatelliteUpdates: Unknown error code" << errorCode;
                break;
            }
        }
        return QGeoSatelliteInfoSource::UnknownSourceError;
    }


    bool hasPositioningPermissions(AccuracyTypes accuracy)
    {
        QLocationPermission permission;

        // The needed permission depends on whether we run as a service or as an activity
        if (!QNativeInterface::QAndroidApplication::isActivityContext())
            permission.setAvailability(QLocationPermission::Always); // background location

        bool permitted = false;
        if (accuracy & AccuracyType::Precise) {
            permission.setAccuracy(QLocationPermission::Precise);
            permitted = qApp->checkPermission(permission) == Qt::PermissionStatus::Granted;
        }
        if (accuracy & AccuracyType::Approximate) {
            permission.setAccuracy(QLocationPermission::Approximate);
            permitted |= qApp->checkPermission(permission) == Qt::PermissionStatus::Granted;
        }

        if (!permitted)
            qCWarning(lcPositioning) << "Position data not available due to missing permission";

        return permitted;
    }
}

static void positionUpdated(JNIEnv *env, jobject thiz, QtJniTypes::Location location,
                            jint androidClassKey, jboolean isSingleUpdate)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QGeoPositionInfoSourceAndroid *source = AndroidPositioning::idToPosSource()->value(androidClassKey);
    if (!source) {
        qCWarning(lcPositioning) << "positionUpdated: source == 0";
        return;
    }

    const bool useAltitudeConverter = source->useAltitudeConverter();
    QGeoPositionInfo info =
            AndroidPositioning::positionInfoFromJavaLocation(location.object(), useAltitudeConverter);

    //we need to invoke indirectly as the Looper thread is likely to be not the same thread
    if (!isSingleUpdate)
        QMetaObject::invokeMethod(source, "processPositionUpdate", Qt::AutoConnection,
                              Q_ARG(QGeoPositionInfo, info));
    else
        QMetaObject::invokeMethod(source, "processSinglePositionUpdate", Qt::AutoConnection,
                              Q_ARG(QGeoPositionInfo, info));
}
Q_DECLARE_JNI_NATIVE_METHOD(positionUpdated)

static void locationProvidersDisabled(JNIEnv *env, jobject thiz, jint androidClassKey)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);
    QObject *source = AndroidPositioning::idToPosSource()->value(androidClassKey);
    if (!source)
        source = AndroidPositioning::idToSatSource()->value(androidClassKey);
    if (!source) {
        qCWarning(lcPositioning) << "locationProvidersDisabled: source == 0";
        return;
    }

    QMetaObject::invokeMethod(source, "locationProviderDisabled", Qt::AutoConnection);
}
Q_DECLARE_JNI_NATIVE_METHOD(locationProvidersDisabled)

static void locationProvidersChanged(JNIEnv *env, jobject thiz, jint androidClassKey)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);
    QObject *source = AndroidPositioning::idToPosSource()->value(androidClassKey);
    if (!source) {
        qCWarning(lcPositioning) << "locationProvidersChanged: source == 0";
        return;
    }

    QMetaObject::invokeMethod(source, "locationProvidersChanged", Qt::AutoConnection);
}
Q_DECLARE_JNI_NATIVE_METHOD(locationProvidersChanged)

static void notifySatelliteInfoUpdated(const QList<QGeoSatelliteInfo> &inView,
                                       const QList<QGeoSatelliteInfo> &inUse,
                                       jint androidClassKey, jboolean isSingleUpdate)
{
    QGeoSatelliteInfoSourceAndroid *source = AndroidPositioning::idToSatSource()->value(androidClassKey);
    if (!source) {
        qCWarning(lcPositioning) << "notifySatelliteInfoUpdated: source == 0";
        return;
    }

    QMetaObject::invokeMethod(source, "processSatelliteUpdate", Qt::AutoConnection,
                              Q_ARG(QList<QGeoSatelliteInfo>, inView),
                              Q_ARG(QList<QGeoSatelliteInfo>, inUse),
                              Q_ARG(bool, isSingleUpdate));
}

static void satelliteGpsUpdated(JNIEnv *env, jobject thiz,
                                jobjectArray satellites,
                                jint androidClassKey, jboolean isSingleUpdate)
{
    Q_UNUSED(thiz);
    QList<QGeoSatelliteInfo> inUse;
    QList<QGeoSatelliteInfo> sats =
            AndroidPositioning::satelliteInfoFromJavaLocation(env, satellites, &inUse);

    notifySatelliteInfoUpdated(sats, inUse, androidClassKey, isSingleUpdate);
}
Q_DECLARE_JNI_NATIVE_METHOD(satelliteGpsUpdated)

static void satelliteGnssUpdated(JNIEnv *env, jobject thiz, QtJniTypes::GnssStatus gnssStatus,
                                 jint androidClassKey, jboolean isSingleUpdate)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QList<QGeoSatelliteInfo> inUse;
    QList<QGeoSatelliteInfo> sats =
            AndroidPositioning::satelliteInfoFromJavaGnssStatus(gnssStatus.object(), &inUse);

    notifySatelliteInfoUpdated(sats, inUse, androidClassKey, isSingleUpdate);
}
Q_DECLARE_JNI_NATIVE_METHOD(satelliteGnssUpdated)

#define GET_AND_CHECK_STATIC_METHOD(VAR, METHOD_NAME, ...)                                  \
    VAR = env.findStaticMethod<__VA_ARGS__>(positioningClass(),  METHOD_NAME);              \
    if (!VAR) {                                                                             \
        __android_log_print(ANDROID_LOG_FATAL, logTag, methodErrorMsg, METHOD_NAME,         \
                            QtJniTypes::methodSignature<__VA_ARGS__>().data());             \
        return false;                                                                       \
    }

static bool registerNatives()
{
    QJniEnvironment env;
    if (!env.jniEnv()) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "Failed to create environment");
        return false;
    }

    if (!positioningClass.init()) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "Failed to create global class ref");
        return false;
    }

    if (!env.registerNativeMethods(positioningClass(), {
                                      Q_JNI_NATIVE_METHOD(positionUpdated),
                                      Q_JNI_NATIVE_METHOD(locationProvidersDisabled),
                                      Q_JNI_NATIVE_METHOD(satelliteGpsUpdated),
                                      Q_JNI_NATIVE_METHOD(locationProvidersChanged),
                                      Q_JNI_NATIVE_METHOD(satelliteGnssUpdated)
                                  })) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "Failed to register native methods");
        return false;
    }

    GET_AND_CHECK_STATIC_METHOD(providerListMethodId, "providerList", jintArray);
    GET_AND_CHECK_STATIC_METHOD(lastKnownPositionMethodId, "lastKnownPosition",
                                QtJniTypes::Location, bool, bool);
    GET_AND_CHECK_STATIC_METHOD(startUpdatesMethodId, "startUpdates", jint, jint, jint, jint,
                                bool);
    GET_AND_CHECK_STATIC_METHOD(stopUpdatesMethodId, "stopUpdates", void, jint);
    GET_AND_CHECK_STATIC_METHOD(requestUpdateMethodId, "requestUpdate", jint, jint, jint, jint,
                                bool);
    GET_AND_CHECK_STATIC_METHOD(startSatelliteUpdatesMethodId, "startSatelliteUpdates",
                                jint, jint, jint, bool);

    return true;
}

extern "C" Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM * /*vm*/, void * /*reserved*/)
{
    static bool initialized = false;
    if (initialized)
        return JNI_VERSION_1_6;
    initialized = true;

    __android_log_print(ANDROID_LOG_INFO, logTag, "Positioning start");

    const auto context = QNativeInterface::QAndroidApplication::context();
    QtJniTypes::QtPositioning::callStaticMethod<void>("setContext", context);

    if (!registerNatives()) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "registerNatives() failed");
        return -1;
    }

    if (!ConstellationMapper::init()) {
        __android_log_print(ANDROID_LOG_ERROR, logTag,
                            "Failed to extract constellation type constants. "
                            "Satellite system will be undefined!");
    }

    return JNI_VERSION_1_6;
}
