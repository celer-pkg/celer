// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLEVENT_H
#define QSCXMLEVENT_H

#include <QtScxml/qscxmlglobals.h>

#include <QtCore/qstringlist.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QScxmlEventPrivate;

class Q_SCXML_EXPORT QScxmlEvent
{
    Q_GADGET
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(EventType eventType READ eventType WRITE setEventType)
    Q_PROPERTY(QString scxmlType READ scxmlType)
    Q_PROPERTY(QString sendId READ sendId WRITE setSendId)
    Q_PROPERTY(QString origin READ origin WRITE setOrigin)
    Q_PROPERTY(QString originType READ originType WRITE setOriginType)
    Q_PROPERTY(QString invokeId READ invokeId WRITE setInvokeId)
    Q_PROPERTY(int delay READ delay WRITE setDelay)
    Q_PROPERTY(QVariant data READ data WRITE setData)
    Q_PROPERTY(bool errorEvent READ isErrorEvent)
    Q_PROPERTY(QString errorMessage READ errorMessage WRITE setErrorMessage)

public:
    QScxmlEvent();
    ~QScxmlEvent();

    QScxmlEvent &operator=(const QScxmlEvent &other);
    QScxmlEvent(const QScxmlEvent &other);

    enum EventType {
        PlatformEvent,
        InternalEvent,
        ExternalEvent
    };
    Q_ENUM(EventType)

    QString name() const;
    void setName(const QString &name);

    EventType eventType() const;
    void setEventType(const EventType &type);

    QString scxmlType() const;

    QString sendId() const;
    void setSendId(const QString &sendId);

    QString origin() const;
    void setOrigin(const QString &origin);

    QString originType() const;
    void setOriginType(const QString &originType);

    QString invokeId() const;
    void setInvokeId(const QString &invokeId);

    int delay() const;
    void setDelay(int delayInMiliSecs);

    Q_INVOKABLE void clear();

    QVariant data() const;
    void setData(const QVariant &data);

    bool isErrorEvent() const;
    QString errorMessage() const;
    void setErrorMessage(const QString &message);

private:
    QScxmlEventPrivate *d;

};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QScxmlEvent)

#endif // QSCXMLEVENT_H
