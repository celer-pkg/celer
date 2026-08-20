// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef JSONVIEWER_H
#define JSONVIEWER_H

#include "viewerinterfaces.h"

#include <QJsonValue>
#include <QJsonDocument>
#include <QAbstractItemModel>

QT_BEGIN_NAMESPACE
class QTreeView;
class QListWidget;
class QListWidgetItem;
class QLineEdit;
QT_END_NAMESPACE
//! [pluginHeader]
class JsonViewer : public ViewerInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.DocumentViewer.ViewerInterface/1.0" FILE "jsonviewer.json")
    Q_INTERFACES(ViewerInterface)
public:
    JsonViewer();
    ~JsonViewer() override;
//! [pluginHeader]

    void init(QFile *file, QWidget *parent, QMainWindow *mainWindow) override;
//! [pluginReimp]
    QString viewerName() const override { return QLatin1StringView(staticMetaObject.className()); };
    QStringList supportedMimeTypes() const override;
    bool hasContent() const override;
    bool supportsOverview() const override { return true; }
//! [pluginReimp]
    QByteArray saveState() const override;
    bool restoreState(QByteArray &) override;

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
protected:
    void printDocument(QPrinter *printer) const override;
#endif // QT_ABSTRACTVIEWER_PRINTSUPPORT

private slots:
    void setupJsonUi();
    void onTopLevelItemClicked(QListWidgetItem *item);
    void onTopLevelItemDoubleClicked(QListWidgetItem *item);
    void onJsonMenuRequested(const QPoint &pos);
    void onBookmarkMenuRequested(const QPoint &pos);
    void onBookmarkAdded();
    void onBookmarkDeleted();

//! [pluginPrivateMembers]
private:
    void retranslate() override;
    bool openJsonFile();

    QTreeView *m_tree = nullptr;
    QListWidget *m_toplevel = nullptr;
    QJsonDocument m_root;
    QAction *m_expandAllAction = nullptr;
    QAction *m_collapseAllAction = nullptr;
};
//! [pluginPrivateMembers]

class JsonTreeItem
{
public:
    JsonTreeItem(JsonTreeItem *parent = nullptr);
    ~JsonTreeItem();
    void appendChild(JsonTreeItem *item);
    JsonTreeItem *child(int row);
    JsonTreeItem *parent();
    int childCount() const;
    int row() const;
    void setKey(const QString& key);
    void setValue(const QVariant& value);
    void setType(const QJsonValue::Type& type);
    QString key() const { return m_key; };
    QVariant value() const { return m_value; };
    QJsonValue::Type type() const { return m_type; };

    static JsonTreeItem* load(const QJsonValue& value, JsonTreeItem *parent = nullptr);

private:
    QString m_key;
    QVariant m_value;
    QJsonValue::Type m_type;
    QList<JsonTreeItem*> m_children;
    JsonTreeItem *m_parent = nullptr;
};

class JsonItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit JsonItemModel(QObject *parent = nullptr);
    JsonItemModel(const QJsonDocument& doc, QObject *parent = nullptr);
    ~JsonItemModel();
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override { return 2; };

private:
    JsonTreeItem *m_rootItem = nullptr;
    QStringList m_headers;
    static JsonTreeItem *itemFromIndex(const QModelIndex &index)
    {return static_cast<JsonTreeItem*>(index.internalPointer()); }
};

#endif //JSONVIEWER_H
