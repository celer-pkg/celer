// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QITEMMODELSURFACEDATAPROXY_H
#define QTGRAPHS_QITEMMODELSURFACEDATAPROXY_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstringlist.h>
#include <QtGraphs/qsurfacedataproxy.h>

QT_BEGIN_NAMESPACE

class QItemModelSurfaceDataProxyPrivate;

class Q_GRAPHS_EXPORT QItemModelSurfaceDataProxy : public QSurfaceDataProxy
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QItemModelSurfaceDataProxy)
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(QAbstractItemModel *itemModel READ itemModel WRITE setItemModel NOTIFY
                   itemModelChanged FINAL)
    Q_PROPERTY(QString rowRole READ rowRole WRITE setRowRole NOTIFY rowRoleChanged FINAL)
    Q_PROPERTY(QString columnRole READ columnRole WRITE setColumnRole NOTIFY columnRoleChanged FINAL)
    Q_PROPERTY(QString xPosRole READ xPosRole WRITE setXPosRole NOTIFY xPosRoleChanged FINAL)
    Q_PROPERTY(QString yPosRole READ yPosRole WRITE setYPosRole NOTIFY yPosRoleChanged FINAL)
    Q_PROPERTY(QString zPosRole READ zPosRole WRITE setZPosRole NOTIFY zPosRoleChanged FINAL)
    Q_PROPERTY(QStringList rowCategories READ rowCategories WRITE setRowCategories NOTIFY
                   rowCategoriesChanged FINAL)
    Q_PROPERTY(QStringList columnCategories READ columnCategories WRITE setColumnCategories NOTIFY
                   columnCategoriesChanged FINAL)
    Q_PROPERTY(bool useModelCategories READ useModelCategories WRITE setUseModelCategories NOTIFY
                   useModelCategoriesChanged FINAL)
    Q_PROPERTY(bool autoRowCategories READ autoRowCategories WRITE setAutoRowCategories NOTIFY
                   autoRowCategoriesChanged FINAL)
    Q_PROPERTY(bool autoColumnCategories READ autoColumnCategories WRITE setAutoColumnCategories
                   NOTIFY autoColumnCategoriesChanged FINAL)
    Q_PROPERTY(QRegularExpression rowRolePattern READ rowRolePattern WRITE setRowRolePattern NOTIFY
                   rowRolePatternChanged FINAL)
    Q_PROPERTY(QRegularExpression columnRolePattern READ columnRolePattern WRITE
                   setColumnRolePattern NOTIFY columnRolePatternChanged FINAL)
    Q_PROPERTY(QRegularExpression xPosRolePattern READ xPosRolePattern WRITE setXPosRolePattern
                   NOTIFY xPosRolePatternChanged FINAL)
    Q_PROPERTY(QRegularExpression yPosRolePattern READ yPosRolePattern WRITE setYPosRolePattern
                   NOTIFY yPosRolePatternChanged FINAL)
    Q_PROPERTY(QRegularExpression zPosRolePattern READ zPosRolePattern WRITE setZPosRolePattern
                   NOTIFY zPosRolePatternChanged FINAL)
    Q_PROPERTY(QString rowRoleReplace READ rowRoleReplace WRITE setRowRoleReplace NOTIFY
                   rowRoleReplaceChanged FINAL)
    Q_PROPERTY(QString columnRoleReplace READ columnRoleReplace WRITE setColumnRoleReplace NOTIFY
                   columnRoleReplaceChanged FINAL)
    Q_PROPERTY(QString xPosRoleReplace READ xPosRoleReplace WRITE setXPosRoleReplace NOTIFY
                   xPosRoleReplaceChanged FINAL)
    Q_PROPERTY(QString yPosRoleReplace READ yPosRoleReplace WRITE setYPosRoleReplace NOTIFY
                   yPosRoleReplaceChanged FINAL)
    Q_PROPERTY(QString zPosRoleReplace READ zPosRoleReplace WRITE setZPosRoleReplace NOTIFY
                   zPosRoleReplaceChanged FINAL)
    Q_PROPERTY(
        QItemModelSurfaceDataProxy::MultiMatchBehavior multiMatchBehavior READ multiMatchBehavior
            WRITE setMultiMatchBehavior NOTIFY multiMatchBehaviorChanged FINAL)
    QML_NAMED_ELEMENT(ItemModelSurfaceDataProxy)

public:
    enum class MultiMatchBehavior {
        First,
        Last,
        Average,
        CumulativeY,
    };
    Q_ENUM(MultiMatchBehavior)

    explicit QItemModelSurfaceDataProxy(QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel, QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel,
                                        const QString &yPosRole,
                                        QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel,
                                        const QString &rowRole,
                                        const QString &columnRole,
                                        const QString &yPosRole,
                                        QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel,
                                        const QString &rowRole,
                                        const QString &columnRole,
                                        const QString &xPosRole,
                                        const QString &yPosRole,
                                        const QString &zPosRole,
                                        QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel,
                                        const QString &rowRole,
                                        const QString &columnRole,
                                        const QString &yPosRole,
                                        const QStringList &rowCategories,
                                        const QStringList &columnCategories,
                                        QObject *parent = nullptr);
    explicit QItemModelSurfaceDataProxy(QAbstractItemModel *itemModel,
                                        const QString &rowRole,
                                        const QString &columnRole,
                                        const QString &xPosRole,
                                        const QString &yPosRole,
                                        const QString &zPosRole,
                                        const QStringList &rowCategories,
                                        const QStringList &columnCategories,
                                        QObject *parent = nullptr);
    ~QItemModelSurfaceDataProxy() override;

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

    void remap(const QString &rowRole,
               const QString &columnRole,
               const QString &xPosRole,
               const QString &yPosRole,
               const QString &zPosRole,
               const QStringList &rowCategories,
               const QStringList &columnCategories);

    Q_INVOKABLE qsizetype rowCategoryIndex(const QString &category);
    Q_INVOKABLE qsizetype columnCategoryIndex(const QString &category);

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

    void setMultiMatchBehavior(QItemModelSurfaceDataProxy::MultiMatchBehavior behavior);
    QItemModelSurfaceDataProxy::MultiMatchBehavior multiMatchBehavior() const;

Q_SIGNALS:
    void itemModelChanged(const QAbstractItemModel *itemModel);
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
    void rowRolePatternChanged(const QRegularExpression &pattern);
    void columnRolePatternChanged(const QRegularExpression &pattern);
    void xPosRolePatternChanged(const QRegularExpression &pattern);
    void yPosRolePatternChanged(const QRegularExpression &pattern);
    void zPosRolePatternChanged(const QRegularExpression &pattern);
    void rowRoleReplaceChanged(const QString &replace);
    void columnRoleReplaceChanged(const QString &replace);
    void xPosRoleReplaceChanged(const QString &replace);
    void yPosRoleReplaceChanged(const QString &replace);
    void zPosRoleReplaceChanged(const QString &replace);
    void multiMatchBehaviorChanged(QItemModelSurfaceDataProxy::MultiMatchBehavior behavior);

private:
    Q_DISABLE_COPY(QItemModelSurfaceDataProxy)

    friend class SurfaceItemModelHandler;
};

QT_END_NAMESPACE

#endif
