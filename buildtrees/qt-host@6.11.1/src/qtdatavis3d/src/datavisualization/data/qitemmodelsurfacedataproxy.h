// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QITEMMODELSURFACEDATAPROXY_H
#define QTDATAVIS3D_QITEMMODELSURFACEDATAPROXY_H

#include <QtDataVisualization/qsurfacedataproxy.h>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QStringList>
#include <QtCore/QRegularExpression>

QT_BEGIN_NAMESPACE

class QItemModelSurfaceDataProxyPrivate;

class Q_DATAVISUALIZATION_EXPORT QItemModelSurfaceDataProxy : public QSurfaceDataProxy
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* itemModel READ itemModel WRITE setItemModel NOTIFY itemModelChanged)
    Q_PROPERTY(QString rowRole READ rowRole WRITE setRowRole NOTIFY rowRoleChanged)
    Q_PROPERTY(QString columnRole READ columnRole WRITE setColumnRole NOTIFY columnRoleChanged)
    Q_PROPERTY(QString xPosRole READ xPosRole WRITE setXPosRole NOTIFY xPosRoleChanged)
    Q_PROPERTY(QString yPosRole READ yPosRole WRITE setYPosRole NOTIFY yPosRoleChanged)
    Q_PROPERTY(QString zPosRole READ zPosRole WRITE setZPosRole NOTIFY zPosRoleChanged)
    Q_PROPERTY(QStringList rowCategories READ rowCategories WRITE setRowCategories NOTIFY rowCategoriesChanged)
    Q_PROPERTY(QStringList columnCategories READ columnCategories WRITE setColumnCategories NOTIFY columnCategoriesChanged)
    Q_PROPERTY(bool useModelCategories READ useModelCategories WRITE setUseModelCategories NOTIFY useModelCategoriesChanged)
    Q_PROPERTY(bool autoRowCategories READ autoRowCategories WRITE setAutoRowCategories NOTIFY autoRowCategoriesChanged)
    Q_PROPERTY(bool autoColumnCategories READ autoColumnCategories WRITE setAutoColumnCategories NOTIFY autoColumnCategoriesChanged)
    Q_PROPERTY(QRegularExpression rowRolePattern READ rowRolePattern WRITE setRowRolePattern NOTIFY rowRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QRegularExpression columnRolePattern READ columnRolePattern WRITE setColumnRolePattern NOTIFY columnRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QRegularExpression xPosRolePattern READ xPosRolePattern WRITE setXPosRolePattern NOTIFY xPosRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QRegularExpression yPosRolePattern READ yPosRolePattern WRITE setYPosRolePattern NOTIFY yPosRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QRegularExpression zPosRolePattern READ zPosRolePattern WRITE setZPosRolePattern NOTIFY zPosRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QString rowRoleReplace READ rowRoleReplace WRITE setRowRoleReplace NOTIFY rowRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(QString columnRoleReplace READ columnRoleReplace WRITE setColumnRoleReplace NOTIFY columnRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(QString xPosRoleReplace READ xPosRoleReplace WRITE setXPosRoleReplace NOTIFY xPosRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(QString yPosRoleReplace READ yPosRoleReplace WRITE setYPosRoleReplace NOTIFY yPosRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(QString zPosRoleReplace READ zPosRoleReplace WRITE setZPosRoleReplace NOTIFY zPosRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(MultiMatchBehavior multiMatchBehavior READ multiMatchBehavior WRITE setMultiMatchBehavior NOTIFY multiMatchBehaviorChanged REVISION(1, 1))

public:
    enum MultiMatchBehavior {
        MMBFirst = 0,
        MMBLast = 1,
        MMBAverage = 2,
        MMBCumulativeY = 3
    };
    Q_ENUM(MultiMatchBehavior)

    explicit QItemModelSurfaceDataProxy(QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel, QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel, const QString &yPosRole,
                                        QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel, const QString &rowRole,
                                        const QString &columnRole, const QString &yPosRole,
                                        QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel, const QString &rowRole,
                                        const QString &columnRole, const QString &xPosRole,
                                        const QString &yPosRole, const QString &zPosRole,
                                        QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel, const QString &rowRole,
                                        const QString &columnRole, const QString &yPosRole,
                                        const QStringList &rowCategories,
                                        const QStringList &columnCategories,
                                        QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel, const QString &rowRole,
                                        const QString &columnRole, const QString &xPosRole,
                                        const QString &yPosRole, const QString &zPosRole,
                                        const QStringList &rowCategories,
                                        const QStringList &columnCategories,
                                        QObject *parent = nullptr);
    virtual ~QItemModelSurfaceDataProxy();

    void setItemModel(QAbstractItemModel *itemModel);
    QAbstractItemModel *itemModel() const;

    void setRowRole(const QString &role);
    QString rowRole() const;
    void setColumnRole(const QString &role);
    QString columnRole() const;
    void setXPosRole(const QString &role);
    QString xPosRole() const;
    void setYPosRole(const QString &role);
    QString yPosRole() const;
    void setZPosRole(const QString &role);
    QString zPosRole() const;

    void setRowCategories(const QStringList &categories);
    QStringList rowCategories() const;
    void setColumnCategories(const QStringList &categories);
    QStringList columnCategories() const;

    void setUseModelCategories(bool enable);
    bool useModelCategories() const;
    void setAutoRowCategories(bool enable);
    bool autoRowCategories() const;
    void setAutoColumnCategories(bool enable);
    bool autoColumnCategories() const;

    void remap(const QString &rowRole, const QString &columnRole,
               const QString &xPosRole, const QString &yPosRole,
               const QString &zPosRole, const QStringList &rowCategories,
               const QStringList &columnCategories);

    Q_INVOKABLE int rowCategoryIndex(const QString& category);
    Q_INVOKABLE int columnCategoryIndex(const QString& category);

    void setRowRolePattern(const QRegularExpression &pattern);
    QRegularExpression rowRolePattern() const;
    void setColumnRolePattern(const QRegularExpression &pattern);
    QRegularExpression columnRolePattern() const;
    void setXPosRolePattern(const QRegularExpression &pattern);
    QRegularExpression xPosRolePattern() const;
    void setYPosRolePattern(const QRegularExpression &pattern);
    QRegularExpression yPosRolePattern() const;
    void setZPosRolePattern(const QRegularExpression &pattern);
    QRegularExpression zPosRolePattern() const;

    void setRowRoleReplace(const QString &replace);
    QString rowRoleReplace() const;
    void setColumnRoleReplace(const QString &replace);
    QString columnRoleReplace() const;
    void setXPosRoleReplace(const QString &replace);
    QString xPosRoleReplace() const;
    void setYPosRoleReplace(const QString &replace);
    QString yPosRoleReplace() const;
    void setZPosRoleReplace(const QString &replace);
    QString zPosRoleReplace() const;

    void setMultiMatchBehavior(MultiMatchBehavior behavior);
    MultiMatchBehavior multiMatchBehavior() const;

Q_SIGNALS:
    void itemModelChanged(const QAbstractItemModel* itemModel);
    void rowRoleChanged(const QString &role);
    void columnRoleChanged(const QString &role);
    void xPosRoleChanged(const QString &role);
    void yPosRoleChanged(const QString &role);
    void zPosRoleChanged(const QString &role);
    void rowCategoriesChanged();
    void columnCategoriesChanged();
    void useModelCategoriesChanged(bool enable);
    void autoRowCategoriesChanged(bool enable);
    void autoColumnCategoriesChanged(bool enable);
    Q_REVISION(1, 1) void rowRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void columnRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void xPosRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void yPosRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void zPosRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void rowRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void columnRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void xPosRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void yPosRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void zPosRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void multiMatchBehaviorChanged(MultiMatchBehavior behavior);

protected:
    QItemModelSurfaceDataProxyPrivate *dptr();
    const QItemModelSurfaceDataProxyPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QItemModelSurfaceDataProxy)

    friend class SurfaceItemModelHandler;
};

QT_END_NAMESPACE

#endif
