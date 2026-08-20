// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopositioninfosource_cl_p.h"
#include <QtCore/QTimerEvent>
#include <QtCore/QDebug>
#include <QtCore/qdatetime.h>
#include <QtCore/qglobal.h>
#include <QtCore/private/qglobal_p.h>
#include <QtCore/qtimezone.h>
#include <QtCore/QPermission>
#include <QtCore/QCoreApplication>


#define MINIMUM_UPDATE_INTERVAL 1000

@interface PositionLocationDelegate : NSObject <CLLocationManagerDelegate>
@end

@implementation PositionLocationDelegate
{
    QGeoPositionInfoSourceCL *m_positionInfoSource;
}

- (instancetype)initWithInfoSource:(QGeoPositionInfoSourceCL*) positionInfoSource
{
    if ((self = [self init])) {
        m_positionInfoSource = positionInfoSource;
    }
    return self;
}

- (void)locationManagerDidChangeAuthorization: (CLLocationManager *)manager
{
    // Since Qt 6.6 the application requests the desired permissions.
    // This delegate method is invoked always upon CLLocationManager
    // instantiation, and later if authorization status changes.
    m_positionInfoSource->changeAuthorizationStatus([manager authorizationStatus]);
}

- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation
{
    Q_UNUSED(manager);
    Q_UNUSED(oldLocation);

    // Convert location timestamp to QDateTime
    NSTimeInterval locationTimeStamp = [newLocation.timestamp timeIntervalSince1970];
    const QDateTime timeStamp = QDateTime::fromMSecsSinceEpoch(qRound64(locationTimeStamp * 1000),
                                                               QTimeZone::UTC);

    // Construct position info from location data
    QGeoPositionInfo location(QGeoCoordinate(newLocation.coordinate.latitude,
                                             newLocation.coordinate.longitude,
                                             newLocation.altitude),
                                             timeStamp);
    if (newLocation.horizontalAccuracy >= 0)
        location.setAttribute(QGeoPositionInfo::HorizontalAccuracy, newLocation.horizontalAccuracy);
    if (newLocation.verticalAccuracy >= 0)
        location.setAttribute(QGeoPositionInfo::VerticalAccuracy, newLocation.verticalAccuracy);
    if (newLocation.course >= 0) {
        location.setAttribute(QGeoPositionInfo::Direction, newLocation.course);
        if (__builtin_available(iOS 13.4, macOS 10.15.4, *)) {
            if (newLocation.courseAccuracy >= 0) {
                location.setAttribute(QGeoPositionInfo::DirectionAccuracy,
                                      newLocation.courseAccuracy);
            }
        }
    }
    if (newLocation.speed >= 0)
        location.setAttribute(QGeoPositionInfo::GroundSpeed, newLocation.speed);

    m_positionInfoSource->locationDataAvailable(location);
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
    Q_UNUSED(manager);
    m_positionInfoSource->setError(QGeoPositionInfoSource::AccessError);

    qWarning() << QString::fromNSString([error localizedDescription]);

    if ([error code] == 0
        && QString::fromNSString([error domain]) == QStringLiteral("kCLErrorDomain"))
      qWarning() << "(is Wi-Fi turned on?)";
}
@end

QT_BEGIN_NAMESPACE

QGeoPositionInfoSourceCL::QGeoPositionInfoSourceCL(QObject *parent)
    : QGeoPositionInfoSource(parent),
      m_locationManager(0),
      m_updatesWanted(false),
      m_updateTimer(0),
      m_updateTimeout(0),
      m_positionError(QGeoPositionInfoSource::NoError)
{
}

QGeoPositionInfoSourceCL::~QGeoPositionInfoSourceCL()
{
    stopUpdates();
    [m_locationManager release];
}

void QGeoPositionInfoSourceCL::setUpdateInterval(int msec)
{
    // If msec is 0 we send updates as data becomes available, otherwise we force msec to be equal
    // to or larger than the minimum update interval.
    if (msec != 0 && msec < minimumUpdateInterval())
        msec = minimumUpdateInterval();

    QGeoPositionInfoSource::setUpdateInterval(msec);

    // Must timeout if update takes longer than specified interval
    m_updateTimeout = msec;
    if (m_updatesWanted)
        setTimeoutInterval(m_updateTimeout);
}

bool QGeoPositionInfoSourceCL::enableLocationManager()
{
    if (!m_locationManager) {
        m_locationManager = [[CLLocationManager alloc] init];

#if defined(Q_OS_IOS)
        NSDictionary<NSString *, id> *infoDict = [[NSBundle mainBundle] infoDictionary];
        if (id value = [infoDict objectForKey:@"UIBackgroundModes"]) {
            if ([value isKindOfClass:[NSArray class]]) {
                NSArray *modes = static_cast<NSArray *>(value);
                for (id mode in modes) {
                    if ([@"location" isEqualToString:mode]) {
                        m_locationManager.allowsBackgroundLocationUpdates = YES;
                        break;
                    }
                }
            }
        }
#endif

        m_locationManager.desiredAccuracy = kCLLocationAccuracyBest;
        m_locationManager.delegate = [[PositionLocationDelegate alloc] initWithInfoSource:this];
    }

    return qApp->checkPermission(QLocationPermission{}) == Qt::PermissionStatus::Granted;
}

void QGeoPositionInfoSourceCL::setTimeoutInterval(int msec)
{
    // Start timeout timer
    if (m_updateTimer) killTimer(m_updateTimer);
    if (msec > 0) m_updateTimer = startTimer(msec);
    else m_updateTimer = 0;
}

void QGeoPositionInfoSourceCL::startUpdates()
{
    m_positionError = QGeoPositionInfoSource::NoError;
    m_updatesWanted = true;
    if (enableLocationManager()) {
        [m_locationManager startUpdatingLocation];
        setTimeoutInterval(m_updateTimeout);
    } else {
        setError(QGeoPositionInfoSource::AccessError);
    }
}

void QGeoPositionInfoSourceCL::stopUpdates()
{
    if (m_locationManager) {
        [m_locationManager stopUpdatingLocation];
        m_updatesWanted = false;

        // Stop timeout timer
        setTimeoutInterval(0);
    } else {
        setError(QGeoPositionInfoSource::AccessError);
    }
}

void QGeoPositionInfoSourceCL::requestUpdate(int timeout)
{
    // Get a single update within timeframe
    m_positionError = QGeoPositionInfoSource::NoError;
    if (timeout < minimumUpdateInterval() && timeout != 0)
        setError(QGeoPositionInfoSource::UpdateTimeoutError);
    else if (enableLocationManager()) {
        // This will force LM to generate a new update
        [m_locationManager stopUpdatingLocation];
        [m_locationManager startUpdatingLocation];
        setTimeoutInterval(timeout);
    } else {
        setError(QGeoPositionInfoSource::AccessError);
    }
}

void QGeoPositionInfoSourceCL::changeAuthorizationStatus(CLAuthorizationStatus status)
{
#ifndef Q_OS_MACOS
    if (status == kCLAuthorizationStatusAuthorizedAlways
        || status == kCLAuthorizationStatusAuthorizedWhenInUse
#else
    if (status == kCLAuthorizationStatusAuthorized
#endif
    ) {
        if (m_updatesWanted)
            startUpdates();
    }
}

void QGeoPositionInfoSourceCL::timerEvent( QTimerEvent * event )
{
    // Update timed out?
    if (event->timerId() == m_updateTimer) {
        setError(QGeoPositionInfoSource::UpdateTimeoutError);

        // Only timeout once since last data
        setTimeoutInterval(0);

        // Started for single update?
        if (!m_updatesWanted)
            stopUpdates();
    }
}

QGeoPositionInfoSource::PositioningMethods QGeoPositionInfoSourceCL::supportedPositioningMethods() const
{
    // CoreLocation doesn't say which positioning method(s) it used
    return QGeoPositionInfoSource::AllPositioningMethods;
}

int QGeoPositionInfoSourceCL::minimumUpdateInterval() const
{
    return MINIMUM_UPDATE_INTERVAL;
}

void QGeoPositionInfoSourceCL::locationDataAvailable(QGeoPositionInfo location)
{
    // Signal position data available
    m_lastUpdate = location;
    emit positionUpdated(location);

    // Started for single update?
    if (!m_updatesWanted)
        stopUpdates();
    // ...otherwise restart timeout timer
    else setTimeoutInterval(m_updateTimeout);
}

QGeoPositionInfo QGeoPositionInfoSourceCL::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    Q_UNUSED(fromSatellitePositioningMethodsOnly);

    return m_lastUpdate;
}

QGeoPositionInfoSource::Error QGeoPositionInfoSourceCL::error() const
{
    return m_positionError;
}

void QGeoPositionInfoSourceCL::setError(QGeoPositionInfoSource::Error positionError)
{
    m_positionError = positionError;
    if (m_positionError != QGeoPositionInfoSource::NoError)
        emit QGeoPositionInfoSource::errorOccurred(positionError);
}

QT_END_NAMESPACE

#include "moc_qgeopositioninfosource_cl_p.cpp"
