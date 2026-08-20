// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QAXWIDGET_H
#define QAXWIDGET_H

#include <QtAxContainer/qaxbase.h>
#include <QtAxContainer/qaxobjectinterface.h>
#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QAxAggregated;

class QAxClientSite;
class QAxWidgetPrivate;

class QAxBaseWidget : public QWidget, public QAxObjectInterface
{
    Q_OBJECT
    Q_PROPERTY(ulong classContext READ classContext WRITE setClassContext)
    Q_PROPERTY(QString control READ control WRITE setControl RESET resetControl)
public:

Q_SIGNALS:
    void exception(int code, const QString &source, const QString &desc, const QString &help);
    void propertyChanged(const QString &name);
    void signal(const QString &name, int argc, void *argv);

protected:
    using QWidget::QWidget;
    QAxBaseWidget(QWidgetPrivate &d, QWidget *parent, Qt::WindowFlags f);
};

class QAxWidget : public QAxBaseWidget, public QAxBase
{
public:
    explicit QAxWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit QAxWidget(const QString &c, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit QAxWidget(IUnknown *iface, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~QAxWidget() override;

    ulong classContext() const override;
    void setClassContext(ulong classContext) override;

    QString control() const override;
    bool setControl(const QString &) override;
    void resetControl() override;
    void clear();
    bool doVerb(const QString &verb);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    virtual QAxAggregated *createAggregate();

    const QMetaObject *metaObject() const override;
    int qt_metacall(QMetaObject::Call call, int id, void **v) override;
    Q_DECL_HIDDEN static void qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a);
    void *qt_metacast(const char *) override;

protected:
    bool initialize(IUnknown **) override;
    virtual bool createHostWindow(bool);
    bool createHostWindow(bool, const QByteArray&);

    void changeEvent(QEvent *e) override;
    void resizeEvent(QResizeEvent *) override;

    virtual bool translateKeyEvent(int message, int keycode) const;

    void connectNotify(const QMetaMethod &signal) override;
private:
    Q_DECLARE_PRIVATE(QAxWidget)

    friend class QAxClientSite;

#ifndef QT_NO_DATASTREAM
    friend QDataStream &operator>>(QDataStream &s, QAxWidget &w);
    friend QDataStream &operator<<(QDataStream &s, const QAxWidget &w);
#endif
};

template <> inline QAxWidget *qobject_cast<QAxWidget*>(const QObject *o)
{
    void *result = o ? const_cast<QObject *>(o)->qt_metacast("QAxWidget") : nullptr;
    return static_cast<QAxWidget *>(result);
}

template <> inline QAxWidget *qobject_cast<QAxWidget*>(QObject *o)
{
    void *result = o ? o->qt_metacast("QAxWidget") : nullptr;
    return static_cast<QAxWidget *>(result);
}

QT_END_NAMESPACE

#endif // QAXWIDGET_H
