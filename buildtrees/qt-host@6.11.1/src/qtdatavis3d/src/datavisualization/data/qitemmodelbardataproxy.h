// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QITEMMODELBARDATAPROXY_H
#define QTDATAVIS3D_QITEMMODELBARDATAPROXY_H

#include <QtDataVisualization/qbardataproxy.h>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QRegularExpression>

QT_BEGIN_NAMESPACE

class QItemModelBarDataProxyPrivate;

class Q_DATAVISUALIZATION_EXPORT QItemModelBarDataProxy : public QBarDataProxy
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* itemModel READ itemModel WRITE setItemModel NOTIFY itemModelChanged)
    Q_PROPERTY(QString rowRole READ rowRole WRITE setRowRole NOTIFY rowRoleChanged)
    Q_PROPERTY(QString columnRole READ columnRole WRITE setColumnRole NOTIFY columnRoleChanged)
    Q_PROPERTY(QString valueRole READ valueRole WRITE setValueRole NOTIFY valueRoleChanged)
    Q_PROPERTY(QString rotationRole READ rotationRole WRITE setRotationRole NOTIFY rotationRoleChanged)
    Q_PROPERTY(QStringList rowCategories READ rowCategories WRITE setRowCategories NOTIFY rowCategoriesChanged)
    Q_PROPERTY(QStringList columnCategories READ columnCategories WRITE setColumnCategories NOTIFY columnCategoriesChanged)
    Q_PROPERTY(bool useModelCategories READ useModelCategories WRITE setUseModelCategories NOTIFY useModelCategoriesChanged)
    Q_PROPERTY(bool autoRowCategories READ autoRowCategories WRITE setAutoRowCategories NOTIFY autoRowCategoriesChanged)
    Q_PROPERTY(bool autoColumnCategories READ autoColumnCategories WRITE setAutoColumnCategories NOTIFY autoColumnCategoriesChanged)
    Q_PROPERTY(QRegularExpression rowRolePattern READ rowRolePattern WRITE setRowRolePattern NOTIFY rowRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QRegularExpression columnRolePattern READ columnRolePattern WRITE setColumnRolePattern NOTIFY columnRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QRegularExpression valueRolePattern READ valueRolePattern WRITE setValueRolePattern NOTIFY valueRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QRegularExpression rotationRolePattern READ rotationRolePattern WRITE setRotationRolePattern NOTIFY rotationRolePatternChanged REVISION(1, 1))
    Q_PROPERTY(QString rowRoleReplace READ rowRoleReplace WRITE setRowRoleReplace NOTIFY rowRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(QString columnRoleReplace READ columnRoleReplace WRITE setColumnRoleReplace NOTIFY columnRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(QString valueRoleReplace READ valueRoleReplace WRITE setValueRoleReplace NOTIFY valueRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(QString rotationRoleReplace READ rotationRoleReplace WRITE setRotationRoleReplace NOTIFY rotationRoleReplaceChanged REVISION(1, 1))
    Q_PROPERTY(MultiMatchBehavior multiMatchBehavior READ multiMatchBehavior WRITE setMultiMatchBehavior NOTIFY multiMatchBehaviorChanged REVISION(1, 1))

public:
    enum MultiMatchBehavior {
        MMBFirst = 0,
        MMBLast = 1,
        MMBAverage = 2,
        MMBCumulative = 3
    };
    Q_ENUM(MultiMatchBehavior)

    explicit QItemModelBarDataProxy(QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel, QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel, const QString &valueRole,
                                    QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel, const QString &rowRole,
                                    const QString &columnRole, const QString &valueRole,
                                    QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel, const QString &rowRole,
                                    const QString &columnRole, const QString &valueRole,
                                    const QString &rotationRole, QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel, const QString &rowRole,
                                    const QString &columnRole, const QString &valueRole,
                                    const QStringList &rowCategories, const QStringList &columnCategories,
                                    QObject *parent = nullptr);
    explicit QItemModelBarDataProxy(QAbstractItemModel *itemModel, const QString &rowRole,
                                    const QString &columnRole, const QString &valueRole,
                                    const QString &rotationRole, const QStringList &rowCategories,
                                    const QStringList &columnCategories, QObject *parent = nullptr);
    virtual ~QItemModelBarDataProxy();

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

    void remap(const QString &rowRole, const QString &columnRole,
               const QString &valueRole, const QString &rotationRole,
               const QStringList &rowCategories,
               const QStringList &columnCategories);

    Q_INVOKABLE int rowCategoryIndex(const QString& category);
    Q_INVOKABLE int columnCategoryIndex(const QString& category);

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

    void setMultiMatchBehavior(MultiMatchBehavior behavior);
    MultiMatchBehavior multiMatchBehavior() const;

Q_SIGNALS:
    void itemModelChanged(const QAbstractItemModel* itemModel);
    void rowRoleChanged(const QString &role);
    void columnRoleChanged(const QString &role);
    void valueRoleChanged(const QString &role);
    void rotationRoleChanged(const QString &role);
    void rowCategoriesChanged();
    void columnCategoriesChanged();
    void useModelCategoriesChanged(bool enable);
    void autoRowCategoriesChanged(bool enable);
    void autoColumnCategoriesChanged(bool enable);
    Q_REVISION(1, 1) void rowRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void columnRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void valueRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void rotationRolePatternChanged(const QRegularExpression &pattern);
    Q_REVISION(1, 1) void rowRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void columnRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void valueRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void rotationRoleReplaceChanged(const QString &replace);
    Q_REVISION(1, 1) void multiMatchBehaviorChanged(MultiMatchBehavior behavior);

protected:
    QItemModelBarDataProxyPrivate *dptr();
    const QItemModelBarDataProxyPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QItemModelBarDataProxy)

    friend class BarItemModelHandler;
};

QT_END_NAMESPACE

#endif
