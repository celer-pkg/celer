// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dprofiler_p.h"

#include <QtQml/qqmlfile.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qthread.h>

QT_BEGIN_NAMESPACE

#if QT_CONFIG(qml_debug)

// Enable to debug profiling without client app.
//#define PROFILE_WITHOUT_CLIENT

// instance will be set, unset in constructor. Allows static methods to be inlined.
QQuick3DProfiler *QQuick3DProfiler::s_instance = nullptr;
#ifdef PROFILE_WITHOUT_CLIENT
quint64 QQuick3DProfiler::featuresEnabled = 0xffffffff;
#else
quint64 QQuick3DProfiler::featuresEnabled = 0;
#endif
QHash<QByteArray, int> QQuick3DProfiler::s_eventData = {};
QHash<int, QByteArray> QQuick3DProfiler::s_eventDataRev = {};
QMutex QQuick3DProfiler::s_eventDataMutex;

QThreadStorage<QQuick3DProfilerSceneGraphData::TimingData> QQuick3DProfilerSceneGraphData::eventTimings = QThreadStorage<QQuick3DProfilerSceneGraphData::TimingData>();

QQuick3DProfilerData::QQuick3DProfilerData(qint64 time, int messageType, int detailType, qint64 d1, qint64 d2, const QList<int> &ids)
    : QQuick3DProfilerData(time, messageType, detailType, d1, d2)
{
    static int ID_MARKER = 0xed000000;
    int size = qMin(ids.size(), s_numSupportedIds);
    for (int i = 0; i < size; i++)
        this->ids[i] = ids[i] | ID_MARKER;
}

int QQuick3DProfiler::registerObject(const QObject *object)
{
    QMutexLocker lock(&s_eventDataMutex);

#ifdef PROFILE_WITHOUT_CLIENT
    if (!s_instance)
        s_instance = new QQuick3DProfiler(nullptr);
#else
    if (!s_instance)
        return 0;
#endif
    QQmlData *qmlData = QQmlData::get(object);
    QByteArray typeAndLocation;
    int id = 0;
    if (qmlData) {
        QQmlType qmlType = QQmlMetaType::qmlType(object->metaObject());
        QString fileName = qmlData->compilationUnit->fileName();
        typeAndLocation = (qmlType.qmlTypeName() + QLatin1Char(' ') + fileName + QLatin1Char(':') + QString::number(qmlData->lineNumber)).toUtf8();
        if (!s_eventData.contains(typeAndLocation)) {
            id = s_eventData.size() + 1;
            s_eventData.insert(typeAndLocation, id);
            s_eventDataRev.insert(id, typeAndLocation);
            s_instance->processMessage(QQuick3DProfilerData(s_instance->timestamp(), Quick3DFrame, Quick3DEventData, id, 0));
        } else {
            id = s_eventData.value(typeAndLocation);
        }
    }
    return id;
}

int QQuick3DProfiler::registerString(const QByteArray &string)
{
    QMutexLocker lock(&s_eventDataMutex);
#ifdef PROFILE_WITHOUT_CLIENT
    if (!s_instance)
        s_instance = new QQuick3DProfiler(nullptr);
#else
    if (!s_instance)
        return 0;
#endif
    int id = 0;
    if (!s_eventData.contains(string)) {
        id = s_eventData.size() + 1;
        s_eventData.insert(string, id);
        s_eventDataRev.insert(id, string);
        s_instance->processMessage(QQuick3DProfilerData(s_instance->timestamp(), Quick3DFrame, Quick3DEventData, id, 0));
    } else {
        id = s_eventData.value(string);
    }
    return id;
}

void QQuick3DProfiler::initialize(QObject *parent)
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = new QQuick3DProfiler(parent);
}

QQuick3DProfiler::QQuick3DProfiler(QObject *parent)
    : QObject(parent)
{
    m_timer.start();
}

QQuick3DProfiler::~QQuick3DProfiler()
{
    QMutexLocker lock(&m_dataMutex);
    featuresEnabled = 0;
    s_instance = nullptr;
}

void QQuick3DProfiler::startProfilingImpl(quint64 features)
{
    QMutexLocker lock(&m_dataMutex);
    featuresEnabled = features;
}

void QQuick3DProfiler::stopProfilingImpl()
{
    QMutexLocker lock(&m_dataMutex);
    featuresEnabled = 0;
    emit dataReady(m_data, s_eventDataRev);
    m_data.clear();
}

void QQuick3DProfiler::reportDataImpl()
{
    QMutexLocker lock(&m_dataMutex);
    emit dataReady(m_data, s_eventDataRev);
    m_data.clear();
}

void QQuick3DProfiler::setTimer(const QElapsedTimer &t)
{
    QMutexLocker lock(&m_dataMutex);
    m_timer = t;
}

#endif
QT_END_NAMESPACE
