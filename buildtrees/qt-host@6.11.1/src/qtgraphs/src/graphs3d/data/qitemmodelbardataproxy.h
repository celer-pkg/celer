// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QITEMMODELBARDATAPROXY_H
#define QTGRAPHS_QITEMMODELBARDATAPROXY_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qregularexpression.h>
#include <QtGraphs/qbardataproxy.h>

QT_BEGIN_NAMESPACE

class QItemModelBarDataProxyPrivate;

class Q_GRAPHS_EXPORT QItemModelBarDataProxy : public QBarDataProxy
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QItemModelBarDataProxy)
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(QAbstractItemModel *itemModel READ itemModel WRITE setItemModel NOTIFY
                   itemModelChanged FINAL)
    Q_PROPERTY(QString rowRole READ rowRole WRITE setRowRole NOTIFY rowRoleChanged FINAL)
    Q_PROPERTY(QString columnRole READ columnRole WRITE setColumnRole NOTIFY columnRoleChanged FINAL)
    Q_PROPERTY(QString valueRole READ valueRole WRITE setValueRole NOTIFY valueRoleChanged FINAL)
    Q_PROPERTY(QString rotationRole READ rotationRole WRITE setRotationRole NOTIFY
                   rotationRoleChanged FINAL)
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
    Q_PROPERTY(QRegularExpression valueRolePattern READ valueRolePattern WRITE setValueRolePattern
                   NOTIFY valueRolePatternChanged FINAL)
    Q_PROPERTY(QRegularExpression rotationRolePattern READ rotationRolePattern WRITE
                   setRotationRolePattern NOTIFY rotationRolePatternChanged FINAL)
    Q_PROPERTY(QString rowRoleReplace READ rowRoleReplace WRITE setRowRoleReplace NOTIFY
                   rowRoleReplaceChanged FINAL)
    Q_PROPERTY(QString columnRoleReplace READ columnRoleReplace WRITE setColumnRoleReplace NOTIFY
                   columnRoleReplaceChanged FINAL)
    Q_PROPERTY(QString valueRoleReplace READ valueRoleReplace WRITE setValueRoleReplace NOTIFY
                   valueRoleReplaceChanged FINAL)
    Q_PROPERTY(QString rotationRoleReplace READ rotationRoleReplace WRITE setRotationRoleReplace
                   NOTIFY rotationRoleReplaceChanged FINAL)
    Q_PROPERTY(QItemModelBarDataProxy::MultiMatchBehavior multiMatchBehavior READ multiMatchBehavior
                   WRITE setMultiMatchBehavior NOTIFY multiMatchBehaviorChanged FINAL)
    QML_NAMED_ELEMENT(ItemModelBarDataProxy)

public:
    enum class MultiMatchBehavior {
        First,
        Last,
        Average,
        Cumulative,
    };
    Q_ENUM(MultiMatchBehavior)

    explicit QItemModelBarDataProxy(QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel, QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                    const QString &valueRole,
                                    QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                    const QString &rowRole,
                                    const QString &columnRole,
                                    const QString &valueRole,
                                    QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                    const QString &rowRole,
                                    const QString &columnRole,
                                    const QString &valueRole,
                                    const QString &rotationRole,
                                    QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                    const QString &rowRole,
                                    const QString &columnRole,
                                    const QString &valueRole,
                                    const QStringList &rowCategories,
                                    const QStringList &columnCategories,
                                    QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                    const QString &rowRole,
                                    const QString &columnRole,
                                    const QString &valueRole,
                                    const QString &rotationRole,
                                    const QStringList &rowCategories,
                                    const QStringList &columnCategories,
                                    QObject *parent = nullptr);
    ~QItemModelBarDataProxy() override;

    void setItemModel(QAbstractItemModel *itemModel);
    QAbstractItemModel *itemModel() const;

    void setRowRole(const QString &role);
    QString rowRole() const;
    void setColumnRole(const QString &role);
    QString columnRole() const;
    void setValueRole(const QString &role);
    QString valueRole() const;
    void setRotationRole(const QString &role);
    QString rotationRole() const;

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
               const QString &valueRole,
               const QString &rotationRole,
               const QStringList &rowCategories,
               const QStringList &columnCategories);

    Q_INVOKABLE qsizetype rowCategoryIndex(const QString &category);
    Q_INVOKABLE qsizetype columnCategoryIndex(const QString &category);

    void setRowRolePattern(const QRegularExpression &pattern);
    QRegularExpression rowRolePattern() const;
    void setColumnRolePattern(const QRegularExpression &pattern);
    QRegularExpression columnRolePattern() const;
    void setValueRolePattern(const QRegularExpression &pattern);
    QRegularExpression valueRolePattern() const;
    void setRotationRolePattern(const QRegularExpression &pattern);
    QRegularExpression rotationRolePattern() const;

    void setRowRoleReplace(const QString &replace);
    QString rowRoleReplace() const;
    void setColumnRoleReplace(const QString &replace);
    QString columnRoleReplace() const;
    void setValueRoleReplace(const QString &replace);
    QString valueRoleReplace() const;
    void setRotationRoleReplace(const QString &replace);
    QString rotationRoleReplace() const;

    void setMultiMatchBehavior(QItemModelBarDataProxy::MultiMatchBehavior behavior);
    QItemModelBarDataProxy::MultiMatchBehavior multiMatchBehavior() const;

Q_SIGNALS:
    void itemModelChanged(const QAbstractItemModel *itemModel);
    void rowRoleChanged(const QString &role);
    void columnRoleChanged(const QString &role);
    void valueRoleChanged(const QString &role);
    void rotationRoleChanged(const QString &role);
    void rowCategoriesChanged();
    void columnCategoriesChanged();
    void useModelCategoriesChanged(bool enable);
    void autoRowCategoriesChanged(bool enable);
    void autoColumnCategoriesChanged(bool enable);
    void rowRolePatternChanged(const QRegularExpression &pattern);
    void columnRolePatternChanged(const QRegularExpression &pattern);
    void valueRolePatternChanged(const QRegularExpression &pattern);
    void rotationRolePatternChanged(const QRegularExpression &pattern);
    void rowRoleReplaceChanged(const QString &replace);
    void columnRoleReplaceChanged(const QString &replace);
    void valueRoleReplaceChanged(const QString &replace);
    void rotationRoleReplaceChanged(const QString &replace);
    void multiMatchBehaviorChanged(QItemModelBarDataProxy::MultiMatchBehavior behavior);

private:
    Q_DISABLE_COPY(QItemModelBarDataProxy)

    friend class BarItemModelHandler;
};

QT_END_NAMESPACE

#endif
