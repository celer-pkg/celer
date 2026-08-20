// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEPROPERTY_P_H
#define QLOTTIEPROPERTY_P_H

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

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QPointF>
#include <QLoggingCategory>
#include <QtMath>
#include <QString>

#include <QtLottie/private/qlottieconstants_p.h>
#include <QtLottie/private/qlottielayer_p.h>
#include <QtLottie/private/qbeziereasing_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

template<typename T>
struct EasingSegment {
    bool complete = false;
    double startFrame = 0;
    double endFrame = 0;
    T startValue;
    T endValue;
    QBezierEasing easing;
    qreal valueForProgress(qreal progress) const {
        return complete ? easing.valueForProgress(progress) : 1;
    }
};

template<typename T>
class QLottieProperty
{
public:
    virtual ~QLottieProperty() = default;

    virtual void construct(const QJsonObject &definition)
    {
        if (definition.value("s"_L1).toVariant().toInt())
            qCInfo(lcLottieQtLottieParser)
                    << "Property is split into separate x and y but it is not supported";

        bool fromExpression = definition.value("fromExpression"_L1).toBool();

        const QJsonValue value = definition.value("k"_L1);
        const QJsonArray valueArray = value.toArray();
        if (valueArray.size() > 0) {
            QJsonObject firstObject = valueArray.at(0).toObject();
            m_animated = firstObject.contains("t"_L1) && firstObject.contains("s"_L1);
        }

        if (m_animated) {
            const QJsonArray &keyframes = valueArray;
            QJsonArray::const_iterator it = keyframes.constBegin();

            const bool schemaChanged = keyframes.last().toObject().contains("s"_L1);

            if (!schemaChanged) {
                while (it != keyframes.constEnd()) {
                    EasingSegment<T> easing = parseKeyframe((*it).toObject(), fromExpression);
                    addEasing(easing);
                    ++it;
                }
            } else {
                while (it != (keyframes.constEnd())) {
                    EasingSegment<T> easing =
                            parseKeyframe((*it).toObject(),
                                          (*(qMin(it + 1, keyframes.constEnd() - 1))).toObject(),
                                          fromExpression);
                    addEasing(easing);
                    ++it;
                }
                int lastFrame = m_easingCurves.last().startFrame;
                m_easingCurves.last().endFrame = lastFrame;
                m_easingCurves.last().complete = false;
                this->m_endFrame = lastFrame;
            }
            m_value = T();
        } else if (value.isArray()) {
            m_value = getValue(valueArray);
        } else {
            m_value = getValue(value);
        }
    }

    void setValue(const T& value)
    {
        m_value = value;
    }

    const T& value() const
    {
        return m_value;
    }

    inline int startFrame() const
    {
        return m_startFrame;
    }

    inline int endFrame() const
    {
        return m_endFrame;
    }

    bool isAnimated() const
    {
        return m_animated;
    }

    QList<EasingSegment<T> > easingCurves() const
    {
        return m_easingCurves;
    }

    virtual bool update(int frame)
    {
        if (!m_animated)
            return false;

        int adjustedFrame = qBound(m_startFrame, frame, m_endFrame);
        if (const EasingSegment<T> *easing = getEasingSegment(adjustedFrame)) {
            qreal progress;
            if (easing->endFrame == easing->startFrame)
                progress = 1;
            else
                progress = ((adjustedFrame - easing->startFrame) * 1.0) /
                        (easing->endFrame - easing->startFrame);
            qreal easedValue = easing->valueForProgress(progress);
            m_value = easing->startValue + easedValue *
                    ((easing->endValue - easing->startValue));
            return true;
        }
        return false;
    }

protected:
    void addEasing(EasingSegment<T>& easing)
    {
        if (m_easingCurves.size()) {
            EasingSegment<T> prevEase = m_easingCurves.last();
            // The end value has to be hand picked to the
            // previous easing segment, as the json data does
            // not contain end values for segments
            prevEase.endFrame = easing.startFrame - 1;
            m_easingCurves.replace(m_easingCurves.size() - 1, prevEase);
        }
        m_easingCurves.push_back(easing);
    }

    const EasingSegment<T>* getEasingSegment(int frame)
    {
        // TODO: Improve with a faster search algorithm
        const EasingSegment<T> *easing = m_currentEasing;
        if (!easing || easing->startFrame < frame ||
                easing->endFrame > frame) {
            for (int i=0; i < m_easingCurves.size(); i++) {
                if (m_easingCurves.at(i).startFrame <= frame &&
                        m_easingCurves.at(i).endFrame >= frame) {
                    m_currentEasing = &m_easingCurves.at(i);
                    break;
                }
            }
        }

        if (!m_currentEasing) {
            qCWarning(lcLottieQtLottieParser)
                    << "Property is animated but easing cannot be found";
        }
        return m_currentEasing;
    }

    virtual EasingSegment<T> parseKeyframe(const QJsonObject keyframe, bool fromExpression)
    {
        Q_UNUSED(fromExpression);

        EasingSegment<T> easing;

        int startTime = keyframe.value("t"_L1).toVariant().toInt();

        // AE exported Lottie file includes the last
        // key frame but no other properties.
        // No need to process in that case
        if (!keyframe.contains(u"s") && !keyframe.contains("e"_L1)) {
            // In this case start time is the last frame for the property
            this->m_endFrame = startTime;
            easing.startFrame = startTime;
            easing.endFrame = startTime;
            if (m_easingCurves.length()) {
                easing.startValue = m_easingCurves.last().endValue;
                easing.endValue = m_easingCurves.last().endValue;
            }
            return easing;
        }

        if (m_startFrame > startTime)
            m_startFrame = startTime;

        easing.startValue = getValue(keyframe.value("s"_L1).toArray());
        easing.endValue = getValue(keyframe.value("e"_L1).toArray());
        easing.startFrame = startTime;

        QJsonObject easingIn = keyframe.value("i"_L1).toObject();
        QJsonObject easingOut = keyframe.value("o"_L1).toObject();

        qreal eix = easingIn.value("x"_L1).toArray().at(0).toDouble();
        qreal eiy = easingIn.value("y"_L1).toArray().at(0).toDouble();

        qreal eox = easingOut.value("x"_L1).toArray().at(0).toDouble();
        qreal eoy = easingOut.value("y"_L1).toArray().at(0).toDouble();

        QPointF c1 = QPointF(eox, eoy);
        QPointF c2 = QPointF(eix, eiy);

        easing.easing.addCubicBezierSegment(c1, c2, QPointF(1.0, 1.0));

        easing.complete = true;

        return easing;
    }

    virtual EasingSegment<T> parseKeyframe(const QJsonObject keyframe,
                                           const QJsonObject nextKeyframe, bool fromExpression)
    {
        Q_UNUSED(fromExpression);

        EasingSegment<T> easing;

        int startTime = keyframe.value("t"_L1).toVariant().toInt();

        if (m_startFrame > startTime)
            m_startFrame = startTime;

        easing.startValue = getValue(keyframe.value("s"_L1).toArray());
        easing.endValue = getValue(nextKeyframe.value("s"_L1).toArray());
        easing.startFrame = startTime;

        QJsonObject easingIn = keyframe.value("i"_L1).toObject();
        QJsonObject easingOut = keyframe.value("o"_L1).toObject();

        qreal eix = easingIn.value("x"_L1).toArray().at(0).toDouble();
        qreal eiy = easingIn.value("y"_L1).toArray().at(0).toDouble();

        qreal eox = easingOut.value("x"_L1).toArray().at(0).toDouble();
        qreal eoy = easingOut.value("y"_L1).toArray().at(0).toDouble();

        QPointF c1 = QPointF(eox, eoy);
        QPointF c2 = QPointF(eix, eiy);

        easing.easing.addCubicBezierSegment(c1, c2, QPointF(1.0, 1.0));

        easing.complete = true;

        return easing;
    }

    virtual T getValue(const QJsonValue &value)
    {
        if (value.isArray())
            return getValue(value.toArray());
        else {
            QVariant val = value.toVariant();
            if (val.canConvert<T>()) {
                T t = val.value<T>();
                return t;
            }
            else
                return T();
        }
    }

    virtual T getValue(const QJsonArray &value)
    {
        QVariant val = value.at(0).toVariant();
        if (val.canConvert<T>()) {
            T t = val.value<T>();
            return t;
        }
        else
            return T();
    }

protected:
    bool m_animated = false;
    QList<EasingSegment<T>> m_easingCurves;
    const EasingSegment<T> *m_currentEasing = nullptr;
    int m_startFrame = INT_MAX;
    int m_endFrame = 0;
    T m_value = {};
};


template <typename T>
class QLottieProperty2D : public QLottieProperty<T>
{
protected:
    T getValue(const QJsonArray &value) override
    {
        if (value.count() > 1)
            return T(value.at(0).toDouble(),
                     value.at(1).toDouble());
        else
            return T();
    }

    EasingSegment<T> parseKeyframe(const QJsonObject keyframe, bool fromExpression) override
    {
        QJsonArray startValues = keyframe.value("s"_L1).toArray();
        QJsonArray endValues = keyframe.value("e"_L1).toArray();
        int startTime = keyframe.value("t"_L1).toVariant().toInt();

        EasingSegment<T> easingCurve;
        easingCurve.startFrame = startTime;

        // AE exported Lottie file includes the last
        // key frame but no other properties.
        // No need to process in that case
        if (startValues.isEmpty() && endValues.isEmpty()) {
            // In this case start time is the last frame for the property
            this->m_endFrame = startTime;
            easingCurve.startFrame = startTime;
            easingCurve.endFrame = startTime;
            if (this->m_easingCurves.length()) {
                easingCurve.startValue = this->m_easingCurves.last().endValue;
                easingCurve.endValue = this->m_easingCurves.last().endValue;
            }
            return easingCurve;
        }

        if (this->m_startFrame > startTime)
            this->m_startFrame = startTime;

        qreal xs, ys, xe, ye;
        // Keyframes originating from an expression use only scalar values.
        // They must be expanded for both x and y coordinates
        if (fromExpression) {
            xs = startValues.at(0).toDouble();
            ys = startValues.at(0).toDouble();
            xe = endValues.at(0).toDouble();
            ye = endValues.at(0).toDouble();
        } else {
            xs = startValues.at(0).toDouble();
            ys = startValues.at(1).toDouble();
            xe = endValues.at(0).toDouble();
            ye = endValues.at(1).toDouble();
        }
        T s(xs, ys);
        T e(xe, ye);

        QJsonObject easingIn = keyframe.value("i"_L1).toObject();
        QJsonObject easingOut = keyframe.value("o"_L1).toObject();

        easingCurve.startFrame = startTime;
        easingCurve.startValue = s;
        easingCurve.endValue = e;

        if (easingIn.value("x"_L1).isArray()) {
            QJsonArray eixArr = easingIn.value("x"_L1).toArray();
            QJsonArray eiyArr = easingIn.value("y"_L1).toArray();

            QJsonArray eoxArr = easingOut.value("x"_L1).toArray();
            QJsonArray eoyArr = easingOut.value("y"_L1).toArray();

            // Doc: "For multi-dimensional animated properties, [x and y] are arrays, with one
            // element per dimension so you can have different easing curves per dimension."
            // Not currently supported; the easing curve for the first vector element is used.
            qreal eix = eixArr.at(0).toDouble();
            qreal eiy = eiyArr.at(0).toDouble();

            qreal eox = eoxArr.at(0).toDouble();
            qreal eoy = eoyArr.at(0).toDouble();

            QPointF c1 = QPointF(eox, eoy);
            QPointF c2 = QPointF(eix, eiy);

            easingCurve.easing.addCubicBezierSegment(c1, c2, QPointF(1.0, 1.0));
        } else {
            qreal eix = easingIn.value("x"_L1).toDouble();
            qreal eiy = easingIn.value("y"_L1).toDouble();

            qreal eox = easingOut.value("x"_L1).toDouble();
            qreal eoy = easingOut.value("y"_L1).toDouble();

            QPointF c1 = QPointF(eox, eoy);
            QPointF c2 = QPointF(eix, eiy);

            easingCurve.easing.addCubicBezierSegment(c1, c2, QPointF(1.0, 1.0));
        }

        easingCurve.complete = true;
        return easingCurve;
    }

    EasingSegment<T> parseKeyframe(const QJsonObject keyframe, const QJsonObject nextKeyframe,
                                   bool fromExpression) override
    {
        QJsonArray startValues = keyframe.value("s"_L1).toArray();
        QJsonArray endValues = nextKeyframe.value("s"_L1).toArray();
        int startTime = keyframe.value("t"_L1).toVariant().toInt();

        EasingSegment<T> easingCurve;
        easingCurve.startFrame = startTime;

        if (this->m_startFrame > startTime)
            this->m_startFrame = startTime;

        qreal xs, ys, xe, ye;
        // Keyframes originating from an expression use only scalar values.
        // They must be expanded for both x and y coordinates
        if (fromExpression) {
            xs = startValues.at(0).toDouble();
            ys = startValues.at(0).toDouble();
            xe = endValues.at(0).toDouble();
            ye = endValues.at(0).toDouble();
        } else {
            xs = startValues.at(0).toDouble();
            ys = startValues.at(1).toDouble();
            xe = endValues.at(0).toDouble();
            ye = endValues.at(1).toDouble();
        }
        T s(xs, ys);
        T e(xe, ye);

        QJsonObject easingIn = keyframe.value("i"_L1).toObject();
        QJsonObject easingOut = keyframe.value("o"_L1).toObject();

        easingCurve.startFrame = startTime;
        easingCurve.startValue = s;
        easingCurve.endValue = e;

        if (easingIn.value("x"_L1).isArray()) {
            QJsonArray eixArr = easingIn.value("x"_L1).toArray();
            QJsonArray eiyArr = easingIn.value("y"_L1).toArray();

            QJsonArray eoxArr = easingOut.value("x"_L1).toArray();
            QJsonArray eoyArr = easingOut.value("y"_L1).toArray();

            // Doc: "For multi-dimensional animated properties, [x and y] are arrays, with one
            // element per dimension so you can have different easing curves per dimension."
            // Not currently supported; the easing curve for the first vector element is used.
            qreal eix = eixArr.takeAt(0).toDouble();
            qreal eiy = eiyArr.takeAt(0).toDouble();

            qreal eox =eoxArr.takeAt(0).toDouble();
            qreal eoy = eoyArr.takeAt(0).toDouble();

            QPointF c1 = QPointF(eox, eoy);
            QPointF c2 = QPointF(eix, eiy);

            easingCurve.easing.addCubicBezierSegment(c1, c2, QPointF(1.0, 1.0));
        } else {
            qreal eix = easingIn.value("x"_L1).toDouble();
            qreal eiy = easingIn.value("y"_L1).toDouble();

            qreal eox = easingOut.value("x"_L1).toDouble();
            qreal eoy = easingOut.value("y"_L1).toDouble();

            QPointF c1 = QPointF(eox, eoy);
            QPointF c2 = QPointF(eix, eiy);

            easingCurve.easing.addCubicBezierSegment(c1, c2, QPointF(1.0, 1.0));
        }

        easingCurve.complete = true;
        return easingCurve;
    }
};

template <typename T>
class QLottieProperty4D : public QLottieProperty<T>
{
public:
    bool update(int frame) override
    {
        if (!this->m_animated)
            return false;

        int adjustedFrame = qBound(this->m_startFrame, frame, this->m_endFrame);
        if (const EasingSegment<T> *easing = QLottieProperty<T>::getEasingSegment(adjustedFrame)) {
            qreal progress = ((adjustedFrame - this->m_startFrame) * 1.0) /
                    (this->m_endFrame - this->m_startFrame);
            qreal easedValue = easing->valueForProgress(progress);
            // For the time being, 4D vectors are used only for colors, and
            // the value must be restricted to between [0, 1]
            easedValue = qBound(qreal(0.0), easedValue, qreal(1.0));
            T sv = easing->startValue;
            T ev = easing->endValue;
            qreal x = sv.x() + easedValue * (ev.x() - sv.x());
            qreal y = sv.y() + easedValue * (ev.y() - sv.y());
            qreal z = sv.z() + easedValue * (ev.z() - sv.z());
            qreal w = sv.w() + easedValue * (ev.w() - sv.w());
            this->m_value = T(x, y, z, w);
        }

        return true;
    }

protected:
    T getValue(const QJsonArray &value) override
    {
        if (value.count() >= 3) {
            // Assuming color value, so limit values to [0, 1] and default alpha to 1.
            float vals[4];
            bool isNormalized = true;
            for (int i = 0; i < 4; i++) {
                vals[i] = float(value.at(i).toDouble());
                if (vals[i] > float(1))
                    isNormalized = false;
            }
            for (int i = 0; i < 4; i++) {
                if (!isNormalized)
                    vals[i] /= float(255);
                vals[i] = qBound(float(0), vals[i], float(1));
            }
            if (value.count() == 3)
                vals[3] = float(1);

            return T(vals[0], vals[1], vals[2], vals[3]);
        } else {
            return T();
        }
    }
};

QT_END_NAMESPACE

#endif // QLOTTIEPROPERTY_P_H
