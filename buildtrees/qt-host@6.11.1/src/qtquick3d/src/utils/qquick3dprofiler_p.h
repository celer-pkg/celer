// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPROFILER_P_H
#define QQUICK3DPROFILER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/private/qabstractanimation_p.h>
#include <QtQuick/private/qtquickglobal_p.h>
#include <QtQuick3DUtils/private/qtquick3dutilsglobal_p.h>


#if QT_CONFIG(qml_debug)
#include <QtQml/private/qqmlprofilerdefinitions_p.h>
#endif

#include <QtCore/qurl.h>
#include <QtCore/qsize.h>
#include <QtCore/qmutex.h>
#include <QtCore/qthreadstorage.h>

QT_BEGIN_NAMESPACE

#if !QT_CONFIG(qml_debug)

#define Q_QUICK3D_PROFILE_IF_ENABLED(feature, Code)

struct QQuick3DProfiler {

};

#define Q_QUICK3D_PROFILING_ENABLED false
#define Q_QUICK3D_PROFILE_REGISTER_D(obj)
#define Q_QUICK3D_PROFILE_REGISTER(obj)
#define Q_QUICK3D_PROFILE_ID
#define Q_QUICK3D_PROFILE_GET_ID
#define Q_QUICK3D_PROFILE_ASSIGN_ID_SG(obj, bgnode)
#define Q_QUICK3D_PROFILE_ASSIGN_ID(bgnode, obj)

#else

#define Q_QUICK3D_PROFILE_IF_ENABLED(feature, Code)\
    if (QQuick3DProfiler::featuresEnabled & (1 << feature)) {\
        Code;\
    } else\
        (void)0

#define Q_QUICK3D_PROFILING_ENABLED (QQuick3DProfiler::featuresEnabled > 0)
#define Q_QUICK3D_PROFILE_REGISTER_D(obj) d->profilingId = QQuick3DProfiler::registerObject(obj)
#define Q_QUICK3D_PROFILE_REGISTER(obj) profilingId = QQuick3DProfiler::registerObject(obj)
#define Q_QUICK3D_PROFILE_ID int profilingId = -1;
#define Q_QUICK3D_PROFILE_GET_ID(Object) \
    QQuick3DObjectPrivate::get(Object)->profilingId
#define Q_QUICK3D_PROFILE_ASSIGN_ID_SG(obj, bgnode) \
    if (bgnode) \
        (bgnode)->profilingId = Q_QUICK3D_PROFILE_GET_ID(obj);
#define Q_QUICK3D_PROFILE_ASSIGN_ID(bgnode, obj) \
    (obj)->profilingId = (bgnode)->profilingId;


struct Q_QUICK3DUTILS_EXPORT QQuick3DProfilerData
{
    static constexpr int s_numSupportedIds = 2;
    QQuick3DProfilerData() {}

    QQuick3DProfilerData(qint64 time, int messageType, int detailType, qint64 d1, qint64 d2)
        : time(time), messageType(messageType), detailType(detailType), subdata1(d1), subdata2(d2) {}
    QQuick3DProfilerData(qint64 time, int messageType, int detailType, qint64 d1, qint64 d2, const QList<int> &ids);

    qint64 time = 0;
    qint32 messageType = 0;
    qint32 detailType = 0;
    qint64 subdata1 = 0;
    qint64 subdata2 = 0;
    qint32 ids[s_numSupportedIds] = {0}; // keep this even sized
};

Q_DECLARE_TYPEINFO(QQuick3DProfilerData, Q_RELOCATABLE_TYPE);

class Q_QUICK3DUTILS_EXPORT QQuick3DProfilerSceneGraphData : public QQmlProfilerDefinitions {
private:
    static const uint s_numSceneGraphTimings = 2;
    static const uint s_numNestedTimings = 5;
    struct Timings {
        uint nesting = 0;
        qint64 values[s_numNestedTimings][s_numSceneGraphTimings + 1] = {};
    };
    struct TimingData
    {
        Timings timings[MaximumQuick3DFrameType] = {};
    };
    static QThreadStorage<TimingData> eventTimings;
public:
    template<int type, bool inc>
    qint64 *timings()
    {
        Timings *timings;
        qint64 *t;
        timings = &eventTimings.localData().timings[type];
        if (inc) {
            Q_ASSERT(timings->nesting < s_numNestedTimings);
            t = timings->values[timings->nesting];
            timings->nesting++;
        } else {
            timings->nesting--;
            t = timings->values[timings->nesting];
        }
        return t;
    }
};

class Q_QUICK3DUTILS_EXPORT QQuick3DProfiler : public QObject, public QQmlProfilerDefinitions {
    Q_OBJECT
public:

    enum Quick3DStage {
        Quick3DStageBegin,
        Quick3DStageEnd
    };

    template<int FrameType>
    static void recordSceneGraphTimestamp(uint position)
    {
        s_instance->m_sceneGraphData.timings<FrameType, true>()[position] = s_instance->timestamp();
    }
    template<int FrameType>
    static void reportQuick3DFrame(uint position, quint64 payload)
    {
        qint64 *timings = s_instance->m_sceneGraphData.timings<FrameType, false>();
        timings[position] = s_instance->timestamp();
        s_instance->processMessage(QQuick3DProfilerData(
                timings[position], Quick3DFrame, FrameType,
                timings[1] - timings[0],
                payload));
    }
    template<int FrameType>
    static void reportQuick3DFrame(uint position, quint64 payload, const QByteArray str)
    {
        qint64 *timings = s_instance->m_sceneGraphData.timings<FrameType, false>();
        timings[position] = s_instance->timestamp();
        int sid = registerString(str);
        QList<int> poids;
        poids << sid;
        s_instance->processMessage(QQuick3DProfilerData(
                timings[position], Quick3DFrame, FrameType,
                timings[1] - timings[0],
                payload,
                poids));
    }
    template<int FrameType>
    static void reportQuick3DFrame(uint position, quint64 payload, int poid)
    {
        qint64 *timings = s_instance->m_sceneGraphData.timings<FrameType, false>();
        timings[position] = s_instance->timestamp();
        QList<int> poids;
        poids << poid;
        s_instance->processMessage(QQuick3DProfilerData(
                timings[position], Quick3DFrame, FrameType,
                timings[1] - timings[0],
                payload,
                poids));
    }
    template<int FrameType>
    static void reportQuick3DFrame(uint position, quint64 payload, int poid, const QByteArray str)
    {
        qint64 *timings = s_instance->m_sceneGraphData.timings<FrameType, false>();
        timings[position] = s_instance->timestamp();
        int sid = registerString(str);
        QList<int> poids;
        poids << poid << sid;
        s_instance->processMessage(QQuick3DProfilerData(
                timings[position], Quick3DFrame, FrameType,
                timings[1] - timings[0],
                payload,
                poids));
    }
    template<int FrameType>
    static void reportQuick3DFrame(uint position, quint64 payload, const QList<int> &poids)
    {
        qint64 *timings = s_instance->m_sceneGraphData.timings<FrameType, false>();
        timings[position] = s_instance->timestamp();
        s_instance->processMessage(QQuick3DProfilerData(
                timings[position], Quick3DFrame, FrameType,
                timings[1] - timings[0],
                payload,
                poids));
    }


    qint64 timestamp() { return m_timer.nsecsElapsed(); }

    static quint64 featuresEnabled;

    static void initialize(QObject *parent);

    ~QQuick3DProfiler() override;

    static int registerObject(const QObject *object);
    static int registerString(const QByteArray &string);

signals:
    void dataReady(const QVector<QQuick3DProfilerData> &data, const QHash<int, QByteArray> &eventData);

protected:
    friend class QQuick3DProfilerAdapter;
    static QMutex s_eventDataMutex;
    static QHash<QByteArray, int> s_eventData;
    static QHash<int, QByteArray> s_eventDataRev;
    static QQuick3DProfiler *s_instance;
    QMutex m_dataMutex;
    QElapsedTimer m_timer;
    QVector<QQuick3DProfilerData> m_data;
    QQuick3DProfilerSceneGraphData m_sceneGraphData;

    QQuick3DProfiler(QObject *parent);

    void processMessage(const QQuick3DProfilerData &message)
    {
        QMutexLocker lock(&m_dataMutex);
        m_data.append(message);
    }

    void startProfilingImpl(quint64 features);
    void stopProfilingImpl();
    void reportDataImpl();
    void setTimer(const QElapsedTimer &t);
};

#endif // QT_CONFIG(qml_debug)

#define Q_QUICK3D_PROFILE(feature, Method) \
    Q_QUICK3D_PROFILE_IF_ENABLED(feature, QQuick3DProfiler::Method)

#define Q_QUICK3D_PROFILE_START(Type) \
    Q_QUICK3D_PROFILE_IF_ENABLED(QQuick3DProfiler::ProfileQuick3D, \
                               (QQuick3DProfiler::recordSceneGraphTimestamp<Type>(QQuick3DProfiler::Quick3DStageBegin)))

#define Q_QUICK3D_PROFILE_END(Type) \
    Q_QUICK3D_PROFILE_IF_ENABLED(QQuick3DProfiler::ProfileQuick3D, \
                               (QQuick3DProfiler::reportQuick3DFrame<Type>(QQuick3DProfiler::Quick3DStageEnd, 0, 0)))

#define Q_QUICK3D_PROFILE_END_WITH_PAYLOAD(Type, Payload) \
    Q_QUICK3D_PROFILE_IF_ENABLED(QQuick3DProfiler::ProfileQuick3D, \
                               (QQuick3DProfiler::reportQuick3DFrame<Type>(QQuick3DProfiler::Quick3DStageEnd, \
                                                                                  Payload)))

#define Q_QUICK3D_PROFILE_END_WITH_STRING(Type, Payload, Str) \
    Q_QUICK3D_PROFILE_IF_ENABLED(QQuick3DProfiler::ProfileQuick3D, \
                               (QQuick3DProfiler::reportQuick3DFrame<Type>(QQuick3DProfiler::Quick3DStageEnd, \
                                                                                  Payload, Str)))
#define Q_QUICK3D_PROFILE_END_WITH_ID(Type, Payload, POID) \
    Q_QUICK3D_PROFILE_IF_ENABLED(QQuick3DProfiler::ProfileQuick3D, \
                               (QQuick3DProfiler::reportQuick3DFrame<Type>(QQuick3DProfiler::Quick3DStageEnd, \
                                                                                  Payload, POID)))
#define Q_QUICK3D_PROFILE_END_WITH_IDS(Type, Payload, POIDs) \
    Q_QUICK3D_PROFILE_IF_ENABLED(QQuick3DProfiler::ProfileQuick3D, \
                               (QQuick3DProfiler::reportQuick3DFrame<Type>(QQuick3DProfiler::Quick3DStageEnd, \
                                                                                  Payload, POIDs)))
#define Q_QUICK3D_PROFILE_END_WITH_ALL(Type, Payload, POID, Str) \
    Q_QUICK3D_PROFILE_IF_ENABLED(QQuick3DProfiler::ProfileQuick3D, \
                               (QQuick3DProfiler::reportQuick3DFrame<Type>(QQuick3DProfiler::Quick3DStageEnd, \
                                                                                  Payload, POID, Str)))

#define QSSG_RENDERPASS_NAME(passName, level, face) \
    QByteArrayLiteral(passName)+ QByteArrayLiteral("_level_") + QByteArray::number(level) \
    + QByteArrayLiteral("_face_") + QByteArrayView(QSSGBaseTypeHelpers::toString(QSSGRenderTextureCubeFace(face)))

QT_END_NAMESPACE

#endif
