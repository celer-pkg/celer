// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qsvgcssproperties_p.h"
#include <QtSvg/private/qsvgutils_p.h>


QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;
using namespace QSvgCssValues;

namespace {

int parseCssClockValue(QStringView str, bool *ok)
{
    int res = 0;
    int ms = 1000;
    str = str.trimmed();
    if (str.endsWith("ms"_L1)) {
        str.chop(2);
        ms = 1;
    } else if (str.endsWith("s"_L1)) {
        str.chop(1);
    } else {
        if (ok)
            *ok = false;
        return res;
    }
    double val = ms * str.toDouble(ok);
    if (ok) {
        if (val > std::numeric_limits<int>::min() && val < std::numeric_limits<int>::max())
            res = static_cast<int>(val);
        else
            *ok = false;
    }
    return res;
}

bool isTimeValue(QStringView str) {
    if (str.endsWith("ms"_L1))
        str.chop(2);
    else if (str.endsWith("s"_L1))
        str.chop(1);
    else
        return false;

    bool ok;
    Q_UNUSED(str.toDouble(&ok))
    return ok;
}

bool isIteration(QStringView str) {
    if (str == "infinite"_L1)
        return true;
    bool ok;
    Q_UNUSED(str.toDouble(&ok))
    return ok;
}

bool isDirection(QStringView str) {
    return str == "normal"_L1 || str == "reverse"_L1 || str.startsWith("alternate"_L1);
}

bool isTimingFunction(QStringView str) {
    return str.startsWith("linear"_L1) || str.startsWith("ease"_L1)
           || str.startsWith("step"_L1) || str.startsWith("cubic-bezier"_L1);
}

bool isFillMode(QStringView str) {
    return str == "none"_L1 || str == "forwards"_L1 || str == "backwards"_L1 || str == "both"_L1;
}

bool isPlayState(QStringView str) {
    return str == "paused"_L1 || str =="running"_L1;
}

}

QSvgCssProperties::QSvgCssProperties(const QXmlStreamAttributes &attributes)
{
    QRegularExpression re(";| "_L1);
    for (int i = 0; i < attributes.size(); ++i) {
        const QXmlStreamAttribute &attribute = attributes.at(i);
        QStringView name = attribute.qualifiedName();
        if (name.isEmpty())
            continue;
        QStringView value = attribute.value();
        switch (name.at(0).unicode()) {

        case 'a':
            if (name == "animation"_L1)
                shortHandtoLonghandForm(value);
            if (name == "animation-name"_L1)
                m_names = value.split(re, Qt::SkipEmptyParts);
            if (name == "animation-duration"_L1)
                m_durations = value.split(re, Qt::SkipEmptyParts);
            if (name == "animation-delay"_L1)
                m_delays = value.split(re, Qt::SkipEmptyParts);
            if (name == "animation-iteration-count"_L1)
                m_iterationCounts = value.split(re, Qt::SkipEmptyParts);
            if (name == "animation-direction"_L1)
                m_directions = value.split(re, Qt::SkipEmptyParts);
            if (name == "animation-timing-function"_L1)
                m_timingFunctions = value.split(re, Qt::SkipEmptyParts);
            if (name == "animation-fill-mode"_L1)
                m_fillModes = value.split(re, Qt::SkipEmptyParts);
            if (name == "animation-play-state"_L1)
                m_playStates = value.split(re, Qt::SkipEmptyParts);
            break;

        case 'o':
            if (name == "offset-path"_L1)
                m_offsetPath = value;
            if (name == "offset-distance"_L1)
                m_offsetDistance = value;
            if (name == "offset-rotate"_L1)
                m_offsetRotate = value;
            break;

        default:
            break;
        }
    }
}

QList<QSvgAnimationProperty> QSvgCssProperties::animations() const
{
    bool ok;
    QList<QSvgAnimationProperty> parsedProperties;
    for (int i = 0; i < m_names.size(); i++) {
        QSvgAnimationProperty property;
        property.name = m_names.at(i);

        if (!m_durations.isEmpty()) {
            QStringView durationStr = m_durations.at(i % m_durations.size());
            int duration = parseCssClockValue(durationStr, &ok);
            property.duration = ok ? duration : 0;
        }

        if (!m_delays.isEmpty()) {
            QStringView delayStr = m_delays.at(i % m_delays.size());
            int delay = parseCssClockValue(delayStr, &ok);
            property.delay = ok ? delay : 0;
        }

        if (!m_iterationCounts.isEmpty()) {
            QStringView iterationStr = m_iterationCounts.at(i % m_iterationCounts.size());
            int iteration;
            if (iterationStr == "infinite"_L1) {
                iteration = -1;
            } else {
                qreal count = iterationStr.toDouble(&ok);
                iteration = ok ? qMax(1.0, count) : 0;
            }
            property.iteration = iteration;
        }

        if (!m_timingFunctions.isEmpty()) {
            QStringView timingFunctionStr = m_timingFunctions.at(i % m_timingFunctions.size());
            if (timingFunctionStr == "linear"_L1) {
                property.easingFunction = EasingFunction::Linear;
            } else if (timingFunctionStr == "ease-in"_L1) {
                property.easingFunction = EasingFunction::EaseIn;
            } else if (timingFunctionStr == "ease-out"_L1) {
                property.easingFunction = EasingFunction::EaseOut;
            } else if (timingFunctionStr == "ease-in-out"_L1) {
                property.easingFunction = EasingFunction::EaseInOut;
            } else if (timingFunctionStr == "step-end"_L1) {
                property.easingFunction = EasingFunction::Steps;
                property.easingValues = StepValues{quint32(1), QSvgCssValues::StepPosition::End};
            } else if (timingFunctionStr == "step-start"_L1) {
                property.easingFunction = EasingFunction::Steps;
                property.easingValues = StepValues{quint32(1), QSvgCssValues::StepPosition::Start};
            }
        }
        parsedProperties.append(property);
    }

    return parsedProperties;
}

QSvgOffsetProperty QSvgCssProperties::offset() const
{
    QSvgOffsetProperty offset;

    offset.path = QSvgUtils::parsePathDataFast(m_offsetPath);

    qsizetype index;
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    if ((index = m_offsetRotate.indexOf("auto"_L1, 0, cs)) >= 0) {
        std::optional<qreal> angle = QSvgUtils::parseAngle(m_offsetRotate.sliced(index + 4));
        offset.rotateType = angle ? QtSvg::OffsetRotateType::AutoAngle :
                                QtSvg::OffsetRotateType::Auto;
        offset.angle = angle.value_or(0);
    } else if ((index = m_offsetRotate.indexOf("reverse"_L1, 0, cs)) >= 0) {
        std::optional<qreal> angle = QSvgUtils::parseAngle(m_offsetRotate.sliced(index + 7));
        offset.rotateType = angle ? QtSvg::OffsetRotateType::ReverseAngle :
                                QtSvg::OffsetRotateType::Reverse;
        offset.angle = angle.value_or(0);
    } else {
        std::optional<qreal> angle = QSvgUtils::parseAngle(m_offsetRotate);
        offset.rotateType = angle ? QtSvg::OffsetRotateType::Angle :
                                QtSvg::OffsetRotateType::Auto;
        offset.angle = angle.value_or(0);
    }

    QSvgUtils::LengthType type;
    bool *ok = nullptr;
    offset.distance= QSvgUtils::parseLength(m_offsetDistance, &type, ok);
    if (type == QSvgUtils::LengthType::LT_PERCENT) {
        offset.distance = offset.distance / 100.0;
    }

    return offset;
}

void QSvgCssProperties::shortHandtoLonghandForm(QStringView value)
{
    m_names.clear();
    m_durations.clear();
    m_delays.clear();
    m_iterationCounts.clear();
    m_directions.clear();
    m_timingFunctions.clear();
    m_fillModes.clear();
    m_playStates.clear();

    enum Property : uchar {
        Duration = 1,
        Delay = 1 << 1,
        IterationCount = 1 << 2,
        Direction = 1 << 3,
        TimingFunction = 1 << 4,
        FillMode = 1 << 5,
        PlayState = 1 << 6,
        Name = 1 << 7
    };

    const QList<QStringView> animations = value.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for (const QStringView &animation : animations) {
        uchar propertyFlag = 0;
        const QList<QStringView> animationProperties = animation.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        for (const QStringView &property : animationProperties) {
            if (!(propertyFlag & Property::Duration) && isTimeValue(property)) {
                m_durations.append(property);
                propertyFlag |= Property::Duration;
            } else if (!(propertyFlag & Property::Delay) && isTimeValue(property)) {
                m_delays.append(property);
                propertyFlag |= Property::Delay;
            } else if (!(propertyFlag & Property::IterationCount) && isIteration(property)) {
                m_iterationCounts.append(property);
                propertyFlag |= Property::IterationCount;
            } else if (!(propertyFlag & Property::Direction) && isDirection(property)) {
                m_directions.append(property);
                propertyFlag |= Property::Direction;
            } else if (!(propertyFlag & Property::TimingFunction) && isTimingFunction(property)) {
                m_timingFunctions.append(property);
                propertyFlag |= Property::TimingFunction;
            } else if (!(propertyFlag & Property::FillMode) && isFillMode(property)) {
                m_fillModes.append(property);
                propertyFlag |= Property::FillMode;
            }  else if (!(propertyFlag & Property::PlayState)&& isPlayState(property)) {
                m_playStates.append(property);
                propertyFlag |= Property::PlayState;
            } else {
                m_names.append(property);
                propertyFlag |= Property::Name;
            }
        }
    }
}

QT_END_NAMESPACE
