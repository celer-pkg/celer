// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeosatelliteinfosource_gypsy_p.h"

#ifdef Q_LOCATION_GYPSY_DEBUG
#include <QDebug>
#endif
#include <QFile>
#include <QVariantMap>

QT_BEGIN_NAMESPACE

#define UPDATE_TIMEOUT_COLD_START 120000

static const auto deviceNameParameter = "deviceName";
static const auto gconfKeyParameter = "gconfKey";
static const auto defaultGconfKey = "/apps/geoclue/master/org.freedesktop.Geoclue.GPSDevice";

// Callback function for 'satellites-changed' -signal
static void satellites_changed (GypsySatellite *satellite,
                                GPtrArray *satellites,
                                gpointer userdata)
{
#ifdef Q_LOCATION_GYPSY_DEBUG
    qDebug() << "QGeoSatelliteInfoSourceGypsy Gypsy satellites-changed -signal received.";
#endif
    ((QGeoSatelliteInfoSourceGypsy *)userdata)->satellitesChanged(satellite, satellites);
}

SatelliteGypsyEngine::SatelliteGypsyEngine(QGeoSatelliteInfoSource *parent) :
    m_owner(parent)
{
}
SatelliteGypsyEngine::~SatelliteGypsyEngine()
{
}

// Glib symbols
gulong SatelliteGypsyEngine::eng_g_signal_connect(gpointer instance,
                                                  const gchar *detailed_signal,
                                                  GCallback c_handler,
                                                  gpointer data)
{
    return ::g_signal_connect(instance, detailed_signal, c_handler, data);
}
guint SatelliteGypsyEngine::eng_g_signal_handlers_disconnect_by_func (gpointer instance,
                                                                      gpointer func,
                                                                      gpointer data)
{
    return ::g_signal_handlers_disconnect_by_func(instance, func, data);
}

void SatelliteGypsyEngine::eng_g_free(gpointer mem)
{
    return ::g_free(mem);
}
// Gypsy symbols
GypsyControl *SatelliteGypsyEngine::eng_gypsy_control_get_default (void)
{
    return ::gypsy_control_get_default();
}
char *SatelliteGypsyEngine::eng_gypsy_control_create (GypsyControl *control, const char *device_name, GError **error)
{
    return ::gypsy_control_create(control, device_name, error);
}
GypsyDevice *SatelliteGypsyEngine::eng_gypsy_device_new (const char *object_path)
{
    return ::gypsy_device_new(object_path);
}
GypsySatellite *SatelliteGypsyEngine::eng_gypsy_satellite_new (const char *object_path)
{
    return ::gypsy_satellite_new (object_path);
}
gboolean SatelliteGypsyEngine::eng_gypsy_device_start (GypsyDevice *device, GError **error)
{
    return ::gypsy_device_start(device, error);
}
gboolean SatelliteGypsyEngine::eng_gypsy_device_stop (GypsyDevice *device, GError **error)
{
    // Unfortunately this cannot be done; calling this will stop the GPS device
    // (basically makes gypsy-daemon unusable for anyone), regardless of applications
    // using it (see bug http://bugs.meego.com/show_bug.cgi?id=11707).
    Q_UNUSED(device);
    Q_UNUSED(error);
    return true;
    //return ::gypsy_device_stop (device, error);
}
GypsyDeviceFixStatus SatelliteGypsyEngine::eng_gypsy_device_get_fix_status (GypsyDevice *device, GError **error)
{
    return ::gypsy_device_get_fix_status (device, error);
}
GPtrArray *SatelliteGypsyEngine::eng_gypsy_satellite_get_satellites (GypsySatellite *satellite, GError **error)
{
    return ::gypsy_satellite_get_satellites (satellite, error);
}
void SatelliteGypsyEngine::eng_gypsy_satellite_free_satellite_array (GPtrArray *satellites)
{
    return ::gypsy_satellite_free_satellite_array(satellites);
}
// GConf symbols (mockability due to X11 requirement)
GConfClient *SatelliteGypsyEngine::eng_gconf_client_get_default(void)
{
    return ::gconf_client_get_default();
}
gchar *SatelliteGypsyEngine::eng_gconf_client_get_string(GConfClient *client, const gchar *key, GError** err)
{
    return ::gconf_client_get_string(client, key, err);
}

QGeoSatelliteInfoSourceGypsy::QGeoSatelliteInfoSourceGypsy(QObject *parent)
    : QGeoSatelliteInfoSource(parent), m_engine(0), m_satellite(0), m_device(0),
      m_requestTimer(this), m_updatesOngoing(false), m_requestOngoing(false)
{
    m_requestTimer.setSingleShot(true);
    QObject::connect(&m_requestTimer, SIGNAL(timeout()), this, SLOT(requestUpdateTimeout()));
}

void QGeoSatelliteInfoSourceGypsy::createEngine()
{
    delete m_engine;
    m_engine = new SatelliteGypsyEngine(this);
}

QGeoSatelliteInfoSourceGypsy::~QGeoSatelliteInfoSourceGypsy()
{
    GError *error = NULL;
    if (m_device) {
        m_engine->eng_gypsy_device_stop (m_device, &error);
        g_object_unref(m_device);
    }
    if (m_satellite)
        g_object_unref(m_satellite);
    if (m_control)
        g_object_unref(m_control);
    if (error)
        g_error_free(error);
    delete m_engine;
}

static QGeoSatelliteInfo::SatelliteSystem idToSystem(int prn)
{
    if (prn >= 1 && prn <= 32)
        return QGeoSatelliteInfo::GPS;
    else if (prn >= 65 && prn <= 96)
        return QGeoSatelliteInfo::GLONASS;
    else if (prn >= 193 && prn <= 200)
        return QGeoSatelliteInfo::QZSS;
    else if ((prn >= 201 && prn <= 235) || (prn >= 401 && prn <= 437))
        return QGeoSatelliteInfo::BEIDOU;
    else if (prn >= 301 && prn <= 336)
        return QGeoSatelliteInfo::GALILEO;
    return QGeoSatelliteInfo::Undefined;
}

void QGeoSatelliteInfoSourceGypsy::satellitesChanged(GypsySatellite *satellite,
                                                     GPtrArray *satellites)
{
    if (!satellite || !satellites)
        return;
    // We have satellite data and assume it is valid.
    // If a single updateRequest was active, send signals right away.
    // If a periodic timer was running (meaning that the client wishes
    // to have updates at defined intervals), store the data for later sending.
    QList<QGeoSatelliteInfo> lastSatellitesInView;
    QList<QGeoSatelliteInfo> lastSatellitesInUse;

    unsigned int i;
    for (i = 0; i < satellites->len; i++) {
        GypsySatelliteDetails *details = (GypsySatelliteDetails *)satellites->pdata[i];
        QGeoSatelliteInfo info;
        info.setSatelliteIdentifier(details->satellite_id);
        info.setSatelliteSystem(idToSystem(details->satellite_id));
        info.setAttribute(QGeoSatelliteInfo::Elevation, details->elevation);
        info.setAttribute(QGeoSatelliteInfo::Azimuth, details->azimuth);
        info.setSignalStrength(details->snr);
        if (details->in_use)
            lastSatellitesInUse.append(info);
        lastSatellitesInView.append(info);
    }
    bool sendUpdates(false);
    // If a single updateRequest() has been issued:
    if (m_requestOngoing) {
        sendUpdates = true;
        m_requestTimer.stop();
        m_requestOngoing = false;
        // If there is no regular updates ongoing, disconnect now.
        if (!m_updatesOngoing) {
            m_engine->eng_g_signal_handlers_disconnect_by_func(G_OBJECT(m_satellite), (void *)satellites_changed, this);
        }
    }
    // If regular updates are to be delivered as they come:
    if (m_updatesOngoing)
        sendUpdates = true;

    if (sendUpdates) {
        emit satellitesInUseUpdated(lastSatellitesInUse);
        emit satellitesInViewUpdated(lastSatellitesInView);
    }
}

QString QGeoSatelliteInfoSourceGypsy::extractDeviceNameFromParameters(const QVariantMap &parameters) const
{
    // The logic is as follows:
    // 1. If the deviceNameParameter is specified, its value is used to get the
    // device name.
    // 2. If the gconfKeyParameter is specified, its value is used as a key to
    // extract the device name from GConf.
    // 3. If nothing is specified, defaultGconfKey is used as a key to extract
    // the device name from GConf.
    if (parameters.contains(deviceNameParameter))
        return parameters.value(deviceNameParameter).toString();

    QString gconfKey = parameters.value(gconfKeyParameter).toString();
    if (gconfKey.isEmpty())
        gconfKey = defaultGconfKey;

    if (!m_engine)
        return QString();

    GConfClient *client = m_engine->eng_gconf_client_get_default();
    if (!client)
        return QString();

    gchar *device_name = m_engine->eng_gconf_client_get_string(client,
                                                               gconfKey.toLatin1().constData(),
                                                               nullptr);
    g_object_unref(client);

    const QString deviceName = QString::fromLatin1(device_name);
    m_engine->eng_g_free(device_name);

    return deviceName;
}

int QGeoSatelliteInfoSourceGypsy::init(const QVariantMap parameters)
{
    GError *error = NULL;
    char *path;

#if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init (); // this function was deprecated in glib 2.36
#endif
    createEngine();

    const QString deviceName = extractDeviceNameFromParameters(parameters);

    if (deviceName.isEmpty() ||
            (deviceName.trimmed().at(0) == '/' && !QFile::exists(deviceName.trimmed()))) {
        qWarning ("QGeoSatelliteInfoSourceGypsy Empty/nonexistent GPS device name detected.");
        qWarning("Use '%s' plugin parameter to specify device name directly", deviceNameParameter);
        qWarning("or use '%s' plugin parameter to specify a GConf key to extract the device name.",
                 gconfKeyParameter);
        qWarning ("If the GConf key is used, the gconftool-2 tool can be used to set device name "
                  "for the selected key, e.g. on terminal:");
        qWarning ("gconftool-2 -t string -s %s /dev/ttyUSB0", gconfKeyParameter);
        return -1;
    }
    m_control = m_engine->eng_gypsy_control_get_default();
    if (!m_control) {
        qWarning("QGeoSatelliteInfoSourceGypsy unable to create Gypsy control.");
        return -1;
    }
    // (path is the DBus path)
    path = m_engine->eng_gypsy_control_create(m_control, deviceName.toLatin1().constData(), &error);
    if (!path) {
        qWarning ("QGeoSatelliteInfoSourceGypsy error creating client.");
        if (error) {
            qWarning ("error message: %s", error->message);
            g_error_free (error);
        }
        return -1;
    }
    m_device = m_engine->eng_gypsy_device_new (path);
    m_satellite = m_engine->eng_gypsy_satellite_new (path);
    m_engine->eng_g_free(path);
    if (!m_device || !m_satellite) {
        qWarning ("QGeoSatelliteInfoSourceGypsy error creating satellite device.");
        qWarning ("Please check that the GPS device is specified correctly.");
        qWarning("Use '%s' plugin parameter to specify device name directly", deviceNameParameter);
        qWarning("or use '%s' plugin parameter to specify a GConf key to extract the device name.",
                 gconfKeyParameter);
        qWarning ("If the GConf key is used, the gconftool-2 tool can be used to set device name "
                  "for the selected key, e.g. on terminal:");
        qWarning ("gconftool-2 -t string -s %s /dev/ttyUSB0", gconfKeyParameter);
        if (m_device)
            g_object_unref(m_device);
        if (m_satellite)
            g_object_unref(m_satellite);
        return -1;
    }
    m_engine->eng_gypsy_device_start (m_device, &error);
    if (error) {
        qWarning ("QGeoSatelliteInfoSourceGypsy error starting device: %s ",
                   error->message);
        g_error_free(error);
        g_object_unref(m_device);
        g_object_unref(m_satellite);
        return -1;
    }
    return 0;
}

int QGeoSatelliteInfoSourceGypsy::minimumUpdateInterval() const
{
    return 1;
}

QGeoSatelliteInfoSource::Error QGeoSatelliteInfoSourceGypsy::error() const
{
    return m_error;
}

void QGeoSatelliteInfoSourceGypsy::startUpdates()
{
    if (m_updatesOngoing)
        return;

    m_error = QGeoSatelliteInfoSource::NoError;

    // If there is a request timer ongoing, we've connected to the signal already
    if (!m_requestTimer.isActive()) {
        m_engine->eng_g_signal_connect (m_satellite, "satellites-changed",
                          G_CALLBACK (satellites_changed), this);
    }
    m_updatesOngoing = true;
}

void QGeoSatelliteInfoSourceGypsy::stopUpdates()
{
    if (!m_updatesOngoing)
        return;
    m_updatesOngoing = false;
    // Disconnect only if there is no single update request ongoing. Once single update request
    // is completed and it notices that there is no active update ongoing, it will disconnect
    // the signal.
    if (!m_requestTimer.isActive())
        m_engine->eng_g_signal_handlers_disconnect_by_func(G_OBJECT(m_satellite), (void *)satellites_changed, this);
}

void QGeoSatelliteInfoSourceGypsy::requestUpdate(int timeout)
{
    if (m_requestOngoing)
        return;

    m_error = QGeoSatelliteInfoSource::NoError;

    if (timeout < 0) {
        setError(QGeoSatelliteInfoSource::UpdateTimeoutError);
        return;
    }
    m_requestOngoing = true;
    GError *error = 0;
    // If GPS has a fix a already, request current data.
    GypsyDeviceFixStatus fixStatus = m_engine->eng_gypsy_device_get_fix_status(m_device, &error);
    if (!error && (fixStatus != GYPSY_DEVICE_FIX_STATUS_INVALID &&
            fixStatus != GYPSY_DEVICE_FIX_STATUS_NONE)) {
#ifdef Q_LOCATION_GYPSY_DEBUG
        qDebug() << "QGeoSatelliteInfoSourceGypsy fix available, requesting current satellite data";
#endif
        GPtrArray *satelliteData = m_engine->eng_gypsy_satellite_get_satellites(m_satellite, &error);
        if (!error) {
            // The fix was available and we have satellite data to deliver right away.
            satellitesChanged(m_satellite, satelliteData);
            m_engine->eng_gypsy_satellite_free_satellite_array(satelliteData);
            return;
        }
    }
    // No fix is available. If updates are not ongoing already, start them.
    m_requestTimer.setInterval(timeout == 0? UPDATE_TIMEOUT_COLD_START: timeout);
    if (!m_updatesOngoing) {
        m_engine->eng_g_signal_connect (m_satellite, "satellites-changed",
                          G_CALLBACK (satellites_changed), this);
    }
    m_requestTimer.start();
    if (error) {
#ifdef Q_LOCATION_GYPSY_DEBUG
        qDebug() << "QGeoSatelliteInfoSourceGypsy error asking fix status or satellite data: " << error->message;
#endif
        g_error_free(error);
    }
}

void QGeoSatelliteInfoSourceGypsy::requestUpdateTimeout()
{
#ifdef Q_LOCATION_GYPSY_DEBUG
    qDebug("QGeoSatelliteInfoSourceGypsy request update timeout occurred.");
#endif
    // If we end up here, there has not been valid satellite update.
    // Emit timeout and disconnect from signal if regular updates are not
    // ongoing (as we were listening just for one single requestUpdate).
    if (!m_updatesOngoing) {
        m_engine->eng_g_signal_handlers_disconnect_by_func(G_OBJECT(m_satellite), (void *)satellites_changed, this);
    }
    m_requestOngoing = false;
    setError(QGeoSatelliteInfoSource::UpdateTimeoutError);
}

void QGeoSatelliteInfoSourceGypsy::setError(QGeoSatelliteInfoSource::Error error)
{
    m_error = error;
    if (m_error != QGeoSatelliteInfoSource::NoError)
        emit QGeoSatelliteInfoSource::errorOccurred(m_error);
}

QT_END_NAMESPACE
