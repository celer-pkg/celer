// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QITEMMODELSCATTERDATAPROXY_H
#define QTDATAVIS3D_QITEMMODELSCATTERDATAPROXY_H

#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QString>
#include <QtCore/QRegularExpression>

QT_BEGIN_NAMESPACE

class QItemModelScatterDataProxyPrivate;

class Q_DATAVISUALIZATION_EXPORT QItemModelScatterDataProxy : public QScatterDataProxy
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* itemModel READ itemModel WRITE setItemModel NOTIFY itemModelChanged)
    Q_PROPERTY(QString xPosRole READ xPosRole WRITE setXPosRole NOTIFY xPosRoleChanged)
    Q_PROPERTY(QString yPosRole READ yPosRole WRITE setYPosRole NOTIFY yPosRoleChanged)
    Q_PROPERTY(QString zPosRole READ zPosRole WRITE setZPosRole NOTIFY zPosRoleChanged)
    Q_PROPERTY(QString rotationRole READ rotationRole WRITE setRotationRole NOTIFY rotationRoleChanged)
    Q_PROPERTY(QRegularExpression xPosRolePattern READ xPosRolePattern WRITE setXPosRolePattern NOTIFY xPosRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QRegularExpression yPosRolePattern READ yPosRolePattern WRITE setYPosRolePattern NOTIFY yPosRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QRegularExpression zPosRolePattern READ zPosRolePattern WRITE setZPosRolePattern NOTIFY zPosRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QRegularExpression rotationRolePattern READ rotationRolePattern WRITE setRotationRolePattern NOTIFY rotationRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QString xPosRoleReplace READ xPosRoleReplace WRITE setXPosRoleReplace NOTIFY xPosRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(QString yPosRoleReplace READ yPosRoleReplace WRITE setYPosRoleReplace NOTIFY yPosRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(QString zPosRoleReplace READ zPosRoleReplace WRITE setZPosRoleReplace NOTIFY zPosRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(QString rotationRoleReplace READ rotationRoleReplace WRITE setRotationRoleReplace NOTIFY rotationRoleReplaceChanged REVISION(1, 1))

public:
    explicit QItemModelScatterDataProxy(QObject *parent = nullptr);
    explicit QItemModelScatterDataProxy(QAbstractItemModel *itemModel, QObject *parent = nullptr);
    explicit QItemModelScatterDataProxy(QAbstractItemModel *itemModel,
                                        const QString &xPosRole, const QString &yPosRole,
                                        const QString &zPosRole, QObject *parent = nullptr);
    explicit QItemModelScatterDataProxy(QAbstractItemModel *itemModel,
                                        const QString &xPosRole, const QString &yPosRole,
                                        const QString &zPosRole, const QString &rotationRole,
                                        QObject *parent = nullptr);
    virtual ~QItemModelScatterDataProxy();

    void setItemModel(QAbstractItemModel *itemModel);
    QAbstractItemModel *itemModel() const;

    void setXPosRole(const QString &role);
    QString xPosRole() const;
    void setYPosRole(const QString &role);
    QString yPosRole() const;
    void setZPosRole(const QString &role);
    QString zPosRole() const;
    void setRotationRole(const QString &role);
    QString rotationRole() const;

    void remap(const QString &xPosRole, const QString &yPosRole, const QString &zPosRole,
               const QString &rotationRole);

    void setXPosRolePattern(const QRegularExpression &pattern);
    QRegularExpression xPosRolePattern() const;
    void setYPosRolePattern(const QRegularExpression &pattern);
    QRegularExpression yPosRolePattern() const;
    void setZPosRolePattern(const QRegularExpression &pattern);
    QRegularExpression zPosRolePattern() const;
    void setRotationRolePattern(const QRegularExpression &pattern);
    QRegularExpression rotationRolePattern() const;

    void setXPosRoleReplace(const QString &replace);
    QString xPosRoleReplace() const;
    void setYPosRoleReplace(const QString &replace);
    QString yPosRoleReplace() const;
    void setZPosRoleReplace(const QString &replace);
    QString zPosRoleReplace() const;
    void setRotationRoleReplace(const QString &replace);
    QString rotationRoleReplace() const;

Q_SIGNALS:
    void itemModelChanged(const QAbstractItemModel* itemModel);
    void xPosRoleChanged(const QString &role);
    void yPosRoleChanged(const QString &role);
    void zPosRoleChanged(const QString &role);
    void rotationRoleChanged(const QString &role);
    Q_REVISION(1, 1) void xPosRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void yPosRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void zPosRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void rotationRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void rotationRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void xPosRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void yPosRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void zPosRoleReplaceChanged(const QString &replace);

protected:
    QItemModelScatterDataProxyPrivate *dptr();
    const QItemModelScatterDataProxyPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QItemModelScatterDataProxy)

    friend class ScatterItemModelHandler;
};

QT_END_NAMESPACE

#endif
