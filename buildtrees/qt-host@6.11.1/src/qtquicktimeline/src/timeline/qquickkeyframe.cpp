// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qquickkeyframe_p.h"

#include "qquicktimeline_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/QVariantAnimation>
#include <QtCore/qmath.h>
#include <QtGui/qpainter.h>
#include <QtQml/QQmlProperty>
#include <QtQml/QQmlFile>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QtCore/QFile>
#include <QtCore/QCborStreamReader>
#include <QCborArray>
#include <QRect>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QQuaternion>
#include <QColor>

#include <private/qvariantanimation_p.h>
#include <private/qqmlproperty_p.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

class QQuickKeyframeGroupPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QQuickKeyframeGroup)
public:
    QQuickKeyframeGroupPrivate() = default;

    QObject *target = nullptr;
    QString propertyName;
    QUrl keyframeSource;
    QByteArray keyframeData;
    bool componentComplete = false;
    int userType = -1;

protected:
    void setupKeyframes();
    bool loadKeyframes(bool fromBinary = false);
    void resetKeyframes();

    static void append_keyframe(QQmlListProperty<QQuickKeyframe> *list, QQuickKeyframe *a);
    static qsizetype keyframe_count(QQmlListProperty<QQuickKeyframe> *list);
    static QQuickKeyframe* keyframe_at(QQmlListProperty<QQuickKeyframe> *list, qsizetype pos);
    static void clear_keyframes(QQmlListProperty<QQuickKeyframe> *list);

    QList<QQuickKeyframe *> keyframes;
    QList<QQuickKeyframe *> sortedKeyframes;

    QVariant originalValue;
    QVariant lastValue;
    QQmlAnyBinding originalBinding;
};

void QQuickKeyframeGroupPrivate::setupKeyframes()
{
    sortedKeyframes = keyframes;
    std::sort(sortedKeyframes.begin(), sortedKeyframes.end(), [](const QQuickKeyframe *first, const QQuickKeyframe *second) {
        return first->frame() < second->frame();
    });
}

// a valid frame : time, easingType, data
// returns total size(data + 2) if valid, -1 otherwise
static int validFrameSize(QMetaType::Type type)
{
    switch (type) {
    case QMetaType::Bool:
    case QMetaType::Int:
    case QMetaType::Float:
    case QMetaType::Double:
        return 3;
    case QMetaType::QVector2D:
    case QMetaType::QPoint:
    case QMetaType::QPointF:
    case QMetaType::QSize:
    case QMetaType::QSizeF:
        return 4;
    case QMetaType::QVector3D:
        return 5;
    case QMetaType::QVector4D:
    case QMetaType::QQuaternion:
    case QMetaType::QColor:
    case QMetaType::QRect:
    case QMetaType::QRectF:
        return 6;
    default:
        qWarning() << "Keyframe property type not handled:" << type;
        return -1;
    }
}

// Read property 'type' value from CborArray from index 'id' and return it as QVariant.
static QVariant qQuickKeyframeReadProperty(const QCborArray &array, qsizetype id, QMetaType::Type type)
{
    switch (type) {
    case QMetaType::QVector2D:
        {
            QVector2D v;
            v.setX(array.at(id).toDouble());
            v.setY(array.at(id + 1).toDouble());
            return QVariant(v);
        }
        break;
    case QMetaType::QVector3D:
        {
            QVector3D v;
            v.setX(array.at(id).toDouble());
            v.setY(array.at(id + 1).toDouble());
            v.setZ(array.at(id + 2).toDouble());
            return QVariant(v);
        }
        break;
    case QMetaType::QVector4D:
        {
            QVector4D v;
            v.setX(array.at(id).toDouble());
            v.setY(array.at(id + 1).toDouble());
            v.setZ(array.at(id + 2).toDouble());
            v.setW(array.at(id + 3).toDouble());
            return QVariant(v);
        }
        break;
    case QMetaType::QQuaternion:
        {
            QQuaternion q;
            q.setScalar(array.at(id).toDouble());
            q.setX(array.at(id + 1).toDouble());
            q.setY(array.at(id + 2).toDouble());
            q.setZ(array.at(id + 3).toDouble());
            return QVariant(q);
        }
        break;
    case QMetaType::QColor:
        {
            QColor c;
            c.setRed(array.at(id).toInteger());
            c.setGreen(array.at(id + 1).toInteger());
            c.setBlue(array.at(id + 2).toInteger());
            c.setAlpha(array.at(id + 3).toInteger());
            return QVariant(c);
        }
        break;
    case QMetaType::QRectF:
        {
            QRectF r;
            r.setX(array.at(id).toDouble());
            r.setY(array.at(id + 1).toDouble());
            r.setWidth(array.at(id + 2).toDouble());
            r.setHeight(array.at(id + 3).toDouble());
            return QVariant(r);
        }
        break;
    case QMetaType::QRect:
        {
            QRect r;
            r.setX(array.at(id).toInteger());
            r.setY(array.at(id + 1).toInteger());
            r.setWidth(array.at(id + 2).toInteger());
            r.setHeight(array.at(id + 3).toInteger());
            return QVariant(r);
        }
        break;
    case QMetaType::QPointF:
        {
            QPointF p;
            p.setX(array.at(id).toDouble());
            p.setY(array.at(id + 1).toDouble());
            return QVariant(p);
        }
        break;
    case QMetaType::QPoint:
        {
            QPoint p;
            p.setX(array.at(id).toInteger());
            p.setY(array.at(id + 1).toInteger());
            return QVariant(p);
        }
        break;
    case QMetaType::QSizeF:
        {
            QSizeF s;
            s.setWidth(array.at(id).toDouble());
            s.setHeight(array.at(id + 1).toDouble());
            return QVariant(s);
        }
        break;
    case QMetaType::QSize:
        {
            QSize s;
            s.setWidth(array.at(id).toInteger());
            s.setHeight(array.at(id + 1).toInteger());
            return QVariant(s);
        }
        break;
    case QMetaType::Bool:
    case QMetaType::Int:
    case QMetaType::Float:
    case QMetaType::Double:
        {
            return array.at(id).toVariant();
        }

    default:
        qWarning() << "Keyframe property type not handled:" << type;
    }

    return QVariant();
}

bool QQuickKeyframeGroupPrivate::loadKeyframes(bool fromBinary)
{
    Q_Q(QQuickKeyframeGroup);

    QCborStreamReader reader;
    QFile dataFile;
    if (!fromBinary) {
        // Resolve URL similar to QQuickImage source
        QUrl loadUrl = keyframeSource;
        QQmlContext *context = qmlContext(q);
        if (context)
            loadUrl = context->resolvedUrl(keyframeSource);
        QString dataFilePath = QQmlFile::urlToLocalFileOrQrc(loadUrl);

        dataFile.setFileName(dataFilePath);
        if (!dataFile.open(QIODevice::ReadOnly)) {
            // Invalid file
            qWarning() << "Unable to open keyframeSource:" << dataFilePath;
            return false;
        }
        reader.setDevice(&dataFile);
    } else {
        reader.addData(keyframeData);
    }

    auto error = [&reader](const QString &msg = QString()) {
        if (!msg.isEmpty())
            qWarning() << "Corrupt keyframeSource" << msg;
        else
            qWarning() << "Corrupt keyframeSource" << reader.lastError().toString();
        return false;
    };

    QCborValue kfSrcCborValue = QCborValue::fromCbor(reader);
    if (reader.lastError() != QCborError::NoError || !kfSrcCborValue.isArray())
        return error(QStringLiteral("invalid format.(array expected)"));

    QCborArray kfSrcCborArray = kfSrcCborValue.toArray();
    // [ "QTimelineKeyframes", version(int), property type(int), [...]]
    if (kfSrcCborArray.size() != 4)
        return error(QStringLiteral("invalid data size"));

    if (kfSrcCborArray.at(0).toString() != QStringLiteral("QTimelineKeyframes"))
        return error(QStringLiteral("invalid keyframeSource header string"));

    if (kfSrcCborArray.at(1).toInteger() != 1)
        return error(QStringLiteral("invalid keyframeSource version %1").arg(kfSrcCborArray.at(1).toInteger()));

    // QMetaType::UnknownType = 0;
    QMetaType::Type propertyType = static_cast<QMetaType::Type>(kfSrcCborArray.at(2).toInteger(0));
    const int frameSize = validFrameSize(propertyType);
    if (frameSize < 0)
        return error(QStringLiteral("unsupported property type"));

    // Start keyframes array
    QCborArray kfArray = kfSrcCborArray.at(3).toArray();
    const auto arraySize = kfArray.size();
    if (arraySize % frameSize != 0)
        return error(QStringLiteral("invalid keyframe array"));

    bool validKeyframeData = true;
    for (qsizetype i = 0; i < arraySize; i += frameSize) {
        // An array element should be ( time, easingType, keyframeData )
        auto keyframe = std::make_unique<QQuickKeyframe>(q);

        keyframe->setFrame(kfArray.at(i).toDouble());

        QCborValue easingTypeValue = kfArray.at(i + 1);
        if (!easingTypeValue.isInteger()) {
            validKeyframeData = false;
            break;
        }
        keyframe->setEasing(static_cast<QEasingCurve::Type>(easingTypeValue.toInteger()));

        QVariant value = qQuickKeyframeReadProperty(kfArray, i + 2, propertyType);
        if (value.isValid()) {
            keyframe->setValue(value);
            keyframes.append(keyframe.release());
        } else {
            validKeyframeData = false;
            break;
        }
    }

    if (!validKeyframeData) {
        qWarning() << "Invalid keyframe data";
        resetKeyframes();
        return false;
    }

    return true;
}

void QQuickKeyframeGroupPrivate::resetKeyframes()
{
    qDeleteAll(keyframes);
    keyframes.clear();
}

void QQuickKeyframeGroupPrivate::append_keyframe(QQmlListProperty<QQuickKeyframe> *list, QQuickKeyframe *a)
{
    auto q = static_cast<QQuickKeyframeGroup *>(list->object);
    q->d_func()->keyframes.append(a);
    q->d_func()->setupKeyframes();
    q->reset();
}

qsizetype QQuickKeyframeGroupPrivate::keyframe_count(QQmlListProperty<QQuickKeyframe> *list)
{
    auto q = static_cast<QQuickKeyframeGroup *>(list->object);
    return q->d_func()->keyframes.size();
}

QQuickKeyframe* QQuickKeyframeGroupPrivate::keyframe_at(QQmlListProperty<QQuickKeyframe> *list, qsizetype pos)
{
    auto q = static_cast<QQuickKeyframeGroup *>(list->object);
    return q->d_func()->keyframes.at(pos);
}

void QQuickKeyframeGroupPrivate::clear_keyframes(QQmlListProperty<QQuickKeyframe> *list)
{
    auto q = static_cast<QQuickKeyframeGroup *>(list->object);
    q->d_func()->keyframes.clear();
    q->d_func()->setupKeyframes();
}

class QQuickKeyframePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QQuickKeyframe)
public:
    QQuickKeyframePrivate() = default;

    qreal frame = 0;
    QEasingCurve easingCurve;
    QVariant value;
};

/*!
    \qmltype Keyframe
    \inherits QtObject
    \nativetype QQuickKeyframe
    \inqmlmodule QtQuick.Timeline
    \ingroup qtqmltypes

    \brief A keyframe.

    The value of a keyframe on a timeline.

    An easing curve can be attached to the keyframe.
*/

/*!
    \qmlproperty double Keyframe::frame

    The position of the keyframe on the timeline.
*/

/*!
    \qmlproperty var Keyframe::easing

    The easing curve attached to the keyframe.
*/

/*!
    \qmlproperty var Keyframe::value

    The value of the keyframe.
*/

/*!
    \qmlsignal Keyframe::easingCurveChanged

    This signal is emitted when the easing curve attached to the keyframe
    changes.
*/

QQuickKeyframe::QQuickKeyframe(QObject *parent)
    : QObject(*(new QQuickKeyframePrivate), parent)
{
}

qreal QQuickKeyframe::frame() const
{
    Q_D(const QQuickKeyframe);
    return d->frame;
}

void QQuickKeyframe::setFrame(qreal f)
{
    Q_D(QQuickKeyframe);
    if (d->frame == f)
        return;
    d->frame = f;

    reset();

    emit frameChanged();
}

void QQuickKeyframe::reset()
{
    auto keyframes = qobject_cast<QQuickKeyframeGroup*>(parent());
    if (keyframes)
        keyframes->reset();
}

QQuickKeyframe::QQuickKeyframe(QQuickKeyframePrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
}

/*!
    \qmltype KeyframeGroup
    \inherits QtObject
    \nativetype QQuickKeyframeGroup
    \inqmlmodule QtQuick.Timeline
    \ingroup qtqmltypes

    \brief A keyframe group.

    A keyframe group contains all keyframes for a specific property of an item
    and always belongs to a timeline.
*/

/*!
    \qmlproperty var KeyframeGroup::target

    The item that is targeted by the keyframe group.
*/

/*!
    \qmlproperty string KeyframeGroup::property

    The property that is targeted by the keyframe group.
*/

/*!
    \qmlproperty list KeyframeGroup::keyframes
    \readonly

    A list of keyframes that belong to the keyframe group.
*/

/*!
    \qmlproperty url KeyframeGroup::keyframeSource

    The URL to a file containing binary keyframe data.

    \note KeyframeGroup should either set this property or contain
    Keyframe child elements. Using both can lead to an undefined behavior.

    \since 1.1
*/

QQuickKeyframeGroup::QQuickKeyframeGroup(QObject *parent)
    : QObject(*(new QQuickKeyframeGroupPrivate), parent)
{

}

QQmlListProperty<QQuickKeyframe> QQuickKeyframeGroup::keyframes()
{
    Q_D(QQuickKeyframeGroup);

    return { this, &d->keyframes, QQuickKeyframeGroupPrivate::append_keyframe,
                QQuickKeyframeGroupPrivate::keyframe_count,
                QQuickKeyframeGroupPrivate::keyframe_at,
                QQuickKeyframeGroupPrivate::clear_keyframes };
}

QObject *QQuickKeyframeGroup::target() const
{
    Q_D(const QQuickKeyframeGroup);
    return d->target;
}

void QQuickKeyframeGroup::setTargetObject(QObject *o)
{
    Q_D(QQuickKeyframeGroup);
    if (d->target == o)
        return;
    d->target = o;

    if (!property().isEmpty())
        init();

    emit targetChanged();
}

QString QQuickKeyframeGroup::property() const
{
    Q_D(const QQuickKeyframeGroup);
    return d->propertyName;
}

void QQuickKeyframeGroup::setProperty(const QString &n)
{
    Q_D(QQuickKeyframeGroup);
    if (d->propertyName == n)
        return;
    d->propertyName = n;

    if (target())
        init();

    emit propertyChanged();
}

QUrl QQuickKeyframeGroup::keyframeSource() const
{
    Q_D(const QQuickKeyframeGroup);
    return d->keyframeSource;
}

void QQuickKeyframeGroup::setKeyframeSource(const QUrl &source)
{
    Q_D(QQuickKeyframeGroup);
    if (d->keyframeSource == source)
        return;

    if (d->keyframes.size() > 0) {
        // Remove possible previously loaded keyframes
        d->resetKeyframes();
        d->keyframeData.clear();
    }

    d->keyframeSource = source;
    if (d->loadKeyframes())
        d->setupKeyframes();
    reset();

    emit keyframeSourceChanged();
}

const QByteArray QQuickKeyframeGroup::keyframeData() const
{
    Q_D(const QQuickKeyframeGroup);
    return d->keyframeData;
}

void QQuickKeyframeGroup::setKeyframeData(const QByteArray &data)
{
    Q_D(QQuickKeyframeGroup);
    if (d->keyframeData == data)
        return;

    if (d->keyframes.size() > 0) {
        // Remove possible previously loaded keyframes
        d->resetKeyframes();
        d->keyframeSource.clear();
    }

    d->keyframeData = data;
    if (d->loadKeyframes(true))
        d->setupKeyframes();
    reset();

    emit keyframeSourceChanged();
}

QVariant QQuickKeyframeGroup::evaluate(qreal frame) const
{
    Q_D(const QQuickKeyframeGroup);

    if (d->sortedKeyframes.isEmpty())
        return QVariant();

    static QQuickKeyframe dummy;
    auto timeline = qobject_cast<QQuickTimeline*>(parent());
    if (timeline)
        dummy.setFrame(timeline->startFrame() - 0.0001);
    dummy.setValue(d->originalValue);

     QQuickKeyframe *lastFrame = &dummy;

    for (auto keyFrame :  std::as_const(d->sortedKeyframes)) {
        if (qFuzzyCompare(frame, keyFrame->frame()) || frame < keyFrame->frame())
            return keyFrame->evaluate(lastFrame, frame, d->userType);
        lastFrame = keyFrame;
    }

    return lastFrame->value();
}

void QQuickKeyframeGroup::setProperty(qreal frame)
{
    Q_D(QQuickKeyframeGroup);

    if (target()) {
        QQmlProperty qmlProperty(target(), property());

        d->lastValue = evaluate(frame);

        if (!qmlProperty.write(d->lastValue))
            qWarning() << "Cannot set property" << property();
    }
}

void QQuickKeyframeGroup::init()
{
    Q_D(QQuickKeyframeGroup);
    if (target()) {
        QQmlProperty qmlProperty(target(), property());
        d->originalValue = QQmlProperty::read(target(), property());
        d->userType = qmlProperty.property().userType();
        if (d->originalBinding)
            d->originalBinding = nullptr;
        d->originalBinding = QQmlAnyBinding::ofProperty(qmlProperty);
        if (property().contains(QLatin1Char('.'))) {
            if (d->userType == QMetaType::QVector2D
                    || d->userType == QMetaType::QVector3D
                    || d->userType == QMetaType::QVector4D
                    || d->userType == QMetaType::QQuaternion)
                d->userType = QMetaType::Double;
        }
    }
}

void QQuickKeyframeGroup::resetDefaultValue()
{
    Q_D(QQuickKeyframeGroup);

    if (QQmlProperty::read(target(), property()) == d->lastValue) {
        if (d->originalBinding) {
            QQmlProperty qmlProperty(target(), property());
            d->originalBinding.installOn(qmlProperty);
            d->originalBinding = nullptr;
        } else {
            QQmlProperty::write(target(), property(), d->originalValue);
        }
    }
}

void QQuickKeyframeGroup::reset()
{
    Q_D(QQuickKeyframeGroup);
    if (!d->componentComplete)
        return;

    auto *timeline = qobject_cast<QQuickTimeline*>(parent());
    if (timeline)
        timeline->reevaluate();
}

void QQuickKeyframeGroup::setupKeyframes()
{
    Q_D(QQuickKeyframeGroup);

    if (d->componentComplete)
        d->setupKeyframes();
}

void QQuickKeyframeGroup::classBegin()
{
    Q_D(QQuickKeyframeGroup);
    d->componentComplete = false;
}

void QQuickKeyframeGroup::componentComplete()
{
    Q_D(QQuickKeyframeGroup);
    d->componentComplete = true;
    setupKeyframes();
}

QEasingCurve QQuickKeyframe::easing() const
{
    Q_D(const QQuickKeyframe);
    return d->easingCurve;
}

void QQuickKeyframe::setEasing(const QEasingCurve &e)
{
    Q_D(QQuickKeyframe);
    if (d->easingCurve == e)
        return;

    d->easingCurve = e;

    reset();

    emit easingCurveChanged();
}

QVariant QQuickKeyframe::value() const
{
    Q_D(const QQuickKeyframe);
    return d->value;
}

void QQuickKeyframe::setValue(const QVariant &v)
{
    Q_D(QQuickKeyframe);
    if (d->value == v)
        return;
    d->value = v;

    reset();

    emit valueChanged();
}

QVariant QQuickKeyframe::evaluate(QQuickKeyframe *pre, qreal frametime, int userType) const
{
    QVariantAnimation::Interpolator interpolator = QVariantAnimationPrivate::getInterpolator(userType);
    if (!pre)
        return value();

    QVariant preValue = pre->value();
    qreal preFrame = pre->frame();

    qreal duration = frame() - preFrame;
    qreal offset = frametime - preFrame;

    qreal progress = easing().valueForProgress(offset / duration);

    const QMetaType targetType(userType);
    preValue.convert(targetType);
    QVariant convertedValue = value();
    convertedValue.convert(targetType);

    if (!interpolator) {
        if (progress < 1.0)
            return preValue;

        return convertedValue;
    }

    if (preValue.isValid() && convertedValue.isValid())
        return interpolator(preValue.constData(), convertedValue.constData(), progress);

    qWarning() << "invalid keyframe target" << preValue << convertedValue << userType;

    return QVariant();
}

QT_END_NAMESPACE


