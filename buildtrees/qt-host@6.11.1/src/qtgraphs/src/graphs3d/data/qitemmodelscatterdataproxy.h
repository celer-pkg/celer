// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QITEMMODELSCATTERDATAPROXY_H
#define QTGRAPHS_QITEMMODELSCATTERDATAPROXY_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstring.h>
#include <QtGraphs/qscatterdataproxy.h>

QT_BEGIN_NAMESPACE

class QItemModelScatterDataProxyPrivate;

class Q_GRAPHS_EXPORT QItemModelScatterDataProxy : public QScatterDataProxy
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QItemModelScatterDataProxy)
    Q_PROPERTY(QAbstractItemModel *itemModel READ itemModel WRITE setItemModel NOTIFY
                   itemModelChanged FINAL)
    Q_PROPERTY(QString xPosRole READ xPosRole WRITE setXPosRole NOTIFY xPosRoleChanged FINAL)
    Q_PROPERTY(QString yPosRole READ yPosRole WRITE setYPosRole NOTIFY yPosRoleChanged FINAL)
    Q_PROPERTY(QString zPosRole READ zPosRole WRITE setZPosRole NOTIFY zPosRoleChanged FINAL)
    Q_PROPERTY(QString rotationRole READ rotationRole WRITE setRotationRole NOTIFY
                   rotationRoleChanged FINAL)
    Q_PROPERTY(QString scaleRole READ scaleRole WRITE setScaleRole NOTIFY
                   scaleRoleChanged FINAL REVISION(6, 10))
    Q_PROPERTY(QRegularExpression xPosRolePattern READ xPosRolePattern WRITE setXPosRolePattern
                   NOTIFY xPosRolePatternChanged FINAL)
    Q_PROPERTY(QRegularExpression yPosRolePattern READ yPosRolePattern WRITE setYPosRolePattern
                   NOTIFY yPosRolePatternChanged FINAL)
    Q_PROPERTY(QRegularExpression zPosRolePattern READ zPosRolePattern WRITE setZPosRolePattern
                   NOTIFY zPosRolePatternChanged FINAL)
    Q_PROPERTY(QRegularExpression rotationRolePattern READ rotationRolePattern WRITE
                   setRotationRolePattern NOTIFY rotationRolePatternChanged FINAL)
    Q_PROPERTY(QRegularExpression scaleRolePattern READ scaleRolePattern WRITE
                   setScaleRolePattern NOTIFY scaleRolePatternChanged FINAL REVISION (6, 10))
    Q_PROPERTY(QString xPosRoleReplace READ xPosRoleReplace WRITE setXPosRoleReplace NOTIFY
                   xPosRoleReplaceChanged FINAL)
    Q_PROPERTY(QString yPosRoleReplace READ yPosRoleReplace WRITE setYPosRoleReplace NOTIFY
                   yPosRoleReplaceChanged FINAL)
    Q_PROPERTY(QString zPosRoleReplace READ zPosRoleReplace WRITE setZPosRoleReplace NOTIFY
                   zPosRoleReplaceChanged FINAL)
    Q_PROPERTY(QString rotationRoleReplace READ rotationRoleReplace WRITE setRotationRoleReplace
                   NOTIFY rotationRoleReplaceChanged FINAL)
    Q_PROPERTY(QString scaleRoleReplace READ scaleRoleReplace WRITE setScaleRoleReplace
                   NOTIFY scaleRoleReplaceChanged FINAL REVISION (6, 10))
    QML_NAMED_ELEMENT(ItemModelScatterDataProxy)

public:
    explicit QItemModelScatterDataProxy(QObject *parent = nullptr);
    explicit QItemModelScatterDataProxy(QAbstractItemModel *itemModel, QObject *parent = nullptr);
    explicit QItemModelScatterDataProxy(QAbstractItemModel *itemModel,
                                        const QString &xPosRole,
                                        const QString &yPosRole,
                                        const QString &zPosRole,
                                        QObject *parent = nullptr);
    explicit QItemModelScatterDataProxy(QAbstractItemModel *itemModel,
                                        const QString &xPosRole,
                                        const QString &yPosRole,
                                        const QString &zPosRole,
                                        const QString &rotationRole,
                                        QObject *parent = nullptr);

    ~QItemModelScatterDataProxy() override;

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
    void setScaleRole(const QString &role);
    QString scaleRole() const;

    void remap(const QString &xPosRole,
               const QString &yPosRole,
               const QString &zPosRole,
               const QString &rotationRole);

    void remap(const QString &xPosRole,
               const QString &yPosRole,
               const QString &zPosRole,
               const QString &rotationRole,
               const QString &scaleRole);

    void setXPosRolePattern(const QRegularExpression &pattern);
    QRegularExpression xPosRolePattern() const;
    void setYPosRolePattern(const QRegularExpression &pattern);
    QRegularExpression yPosRolePattern() const;
    void setZPosRolePattern(const QRegularExpression &pattern);
    QRegularExpression zPosRolePattern() const;
    void setRotationRolePattern(const QRegularExpression &pattern);
    QRegularExpression rotationRolePattern() const;
    void setScaleRolePattern(const QRegularExpression &pattern);
    QRegularExpression scaleRolePattern() const;

    void setXPosRoleReplace(const QString &replace);
    QString xPosRoleReplace() const;
    void setYPosRoleReplace(const QString &replace);
    QString yPosRoleReplace() const;
    void setZPosRoleReplace(const QString &replace);
    QString zPosRoleReplace() const;
    void setRotationRoleReplace(const QString &replace);
    QString rotationRoleReplace() const;
    void setScaleRoleReplace(const QString &replace);
    QString scaleRoleReplace() const;

Q_SIGNALS:
    void itemModelChanged(const QAbstractItemModel *itemModel);
    void xPosRoleChanged(const QString &role);
    void yPosRoleChanged(const QString &role);
    void zPosRoleChanged(const QString &role);
    void rotationRoleChanged(const QString &role);
    void scaleRoleChanged(const QString &role);
    void xPosRolePatternChanged(const QRegularExpression &pattern);
    void yPosRolePatternChanged(const QRegularExpression &pattern);
    void zPosRolePatternChanged(const QRegularExpression &pattern);
    void rotationRolePatternChanged(const QRegularExpression &pattern);
    void scaleRolePatternChanged(const QRegularExpression &pattern);
    void rotationRoleReplaceChanged(const QString &replace);
    void xPosRoleReplaceChanged(const QString &replace);
    void yPosRoleReplaceChanged(const QString &replace);
    void zPosRoleReplaceChanged(const QString &replace);
    void scaleRoleReplaceChanged(const QString &replace);

private:
    Q_DISABLE_COPY(QItemModelScatterDataProxy)
};

QT_END_NAMESPACE

#endif
