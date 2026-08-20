// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QABSTRACTDATAPROXY_H
#define QTGRAPHS_QABSTRACTDATAPROXY_H

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

class QAbstractDataProxyPrivate;

class Q_GRAPHS_EXPORT QAbstractDataProxy : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractDataProxy)
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(QAbstractDataProxy::DataType type READ type CONSTANT)
    QML_NAMED_ELEMENT(AbstractDataProxy)
    QML_UNCREATABLE("Uncreatable base type")

public:
    enum class DataType {
        None,
        Bar,
        Scatter,
        Surface,
    };
    Q_ENUM(DataType)

protected:
    explicit QAbstractDataProxy(QAbstractDataProxyPrivate &d, QObject *parent = nullptr);

public:
    ~QAbstractDataProxy() override;

    QAbstractDataProxy::DataType type() const;

private:
    Q_DISABLE_COPY(QAbstractDataProxy)

    friend class QAbstract3DSeriesPrivate;
};

QT_END_NAMESPACE

#endif
