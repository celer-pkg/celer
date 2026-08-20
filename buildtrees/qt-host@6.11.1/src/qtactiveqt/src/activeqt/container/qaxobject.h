// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QAXOBJECT_H
#define QAXOBJECT_H

#include <QtAxContainer/qaxbase.h>
#include <QtAxContainer/qaxobjectinterface.h>

QT_BEGIN_NAMESPACE

class QAxObjectPrivate;

class QAxBaseObject : public QObject, public QAxObjectInterface
{
    Q_OBJECT
    Q_PROPERTY(ulong classContext READ classContext WRITE setClassContext)
    Q_PROPERTY(QString control READ control WRITE setControl RESET resetControl)

Q_SIGNALS:
    void exception(int code, const QString &source, const QString &desc, const QString &help);
    void propertyChanged(const QString &name);
    void signal(const QString &name, int argc, void *argv);

protected:
    using QObject::QObject;
    QAxBaseObject(QObjectPrivate &d, QObject* parent);
};

class QAxObject : public QAxBaseObject, public QAxBase
{
    friend class QAxEventSink;
    Q_DECLARE_PRIVATE(QAxObject)
public:
    explicit QAxObject(QObject *parent = nullptr);
    explicit QAxObject(const QString &c, QObject *parent = nullptr);
    explicit QAxObject(IUnknown *iface, QObject *parent = nullptr);
    ~QAxObject() override;

    ulong classContext() const override;
    void setClassContext(ulong classContext) override;

    QString control() const override;
    bool setControl(const QString &c) override;
    void resetControl() override;
    void clear();

    bool doVerb(const QString &verb);

    const QMetaObject *metaObject() const override;
    int qt_metacall(QMetaObject::Call call, int id, void **v) override;
    Q_DECL_HIDDEN static void qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a);
    void *qt_metacast(const char *) override;

protected:
    void connectNotify(const QMetaMethod &signal) override;
};

template <> inline QAxObject *qobject_cast<QAxObject*>(const QObject *o)
{
    void *result = o ? const_cast<QObject *>(o)->qt_metacast("QAxObject") : nullptr;
    return reinterpret_cast<QAxObject*>(result);
}

template <> inline QAxObject *qobject_cast<QAxObject*>(QObject *o)
{
    void *result = o ? o->qt_metacast("QAxObject") : nullptr;
    return reinterpret_cast<QAxObject*>(result);
}

QT_END_NAMESPACE
Q_DECLARE_METATYPE(QAxObject*)

#endif // QAXOBJECT_H
