// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "jsonviewer.h"

#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QToolBar>
#include <QTreeView>

#include <QDrag>
#include <QEvent>
#include <QMouseEvent>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMimeData>

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
#include <QPrinter>
#include <QPainter>
#endif

using namespace Qt::StringLiterals;

//! [pluginCpp]
JsonViewer::JsonViewer()
    : m_expandAllAction(new QAction(this)),
      m_collapseAllAction(new QAction(this))

{
    connect(this, &AbstractViewer::uiInitialized, this, &JsonViewer::setupJsonUi);

    m_expandAllAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomIn));
    m_collapseAllAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomOut));
}

void JsonViewer::init(QFile *file, QWidget *parent, QMainWindow *mainWindow)
{
    AbstractViewer::init(file, new QTreeView(parent), mainWindow);
    setTranslationBaseName("jsonviewer"_L1);
    m_tree = qobject_cast<QTreeView *>(widget());
}
//! [pluginCpp]

JsonViewer::~JsonViewer()
{
    delete m_toplevel;
}

QStringList JsonViewer::supportedMimeTypes() const
{
    return {"application/json"_L1};
}

void JsonViewer::setupJsonUi()
{
    // Build Menus and toolbars
    QMenu *jsonMenu = addMenu();
    QToolBar *jsonToolBar = addToolBar();

    connect(m_expandAllAction, &QAction::triggered, m_tree, &QTreeView::expandAll);
    jsonMenu->addAction(m_expandAllAction);
    jsonToolBar->addAction(m_expandAllAction);

    connect(m_collapseAllAction, &QAction::triggered, m_tree, &QTreeView::collapseAll);
    jsonMenu->addAction(m_collapseAllAction);
    jsonToolBar->addAction(m_collapseAllAction);

    openJsonFile();

    if (m_root.isEmpty())
        return;

    // Populate bookmarks with toplevel
    m_uiAssets.tabs->clear();
    m_toplevel = new QListWidget(m_uiAssets.tabs);
    m_uiAssets.tabs->addTab(m_toplevel, {});
    qRegisterMetaType<QModelIndex>();
    for (int i = 0; i < m_tree->model()->rowCount(); ++i) {
        const auto &index = m_tree->model()->index(i, 0);
        m_toplevel->addItem(index.data().toString());
        auto *item = m_toplevel->item(i);
        item->setData(Qt::UserRole, index);
    }
    m_toplevel->setAcceptDrops(true);
    m_tree->setDragEnabled(true);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_toplevel->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_toplevel, &QListWidget::itemClicked, this, &JsonViewer::onTopLevelItemClicked);
    connect(m_toplevel, &QListWidget::itemDoubleClicked, this, &JsonViewer::onTopLevelItemDoubleClicked);
    connect(m_toplevel, &QListWidget::customContextMenuRequested, this, &JsonViewer::onBookmarkMenuRequested);
    connect(m_tree, &QTreeView::customContextMenuRequested, this, &JsonViewer::onJsonMenuRequested);

    // Connect back and forward
    connect(m_uiAssets.back, &QAction::triggered, m_tree, [&](){
        const QModelIndex &index = m_tree->indexAbove(m_tree->currentIndex());
        if (index.isValid())
            m_tree->setCurrentIndex(index);
    });
    connect(m_uiAssets.forward, &QAction::triggered, m_tree, [&](){
        QModelIndex current = m_tree->currentIndex();
        QModelIndex next = m_tree->indexBelow(current);
        if (next.isValid()) {
            m_tree->setCurrentIndex(next);
            return;
        }

        // Expand last item to go beyond
        if (!m_tree->isExpanded(current)) {
            m_tree->expand(current);
            QModelIndex next = m_tree->indexBelow(current);
            if (next.isValid()) {
                m_tree->setCurrentIndex(next);
            }
        }
    });

    retranslate();
}

void resizeToContents(QTreeView *tree)
{
    for (int i = 0; i < tree->header()->count(); ++i)
        tree->resizeColumnToContents(i);
}

bool JsonViewer::openJsonFile()
{
    disablePrinting();

    QJsonParseError err;

    const QString type = tr("open");
    if (!m_file->open(QIODevice::ReadOnly)) {
        statusMessage(tr("Unable to open Json document from %1. %2")
                      .arg(QDir::toNativeSeparators(m_file->fileName()),
                           err.errorString()),type);
        return false;
    }
    m_root = QJsonDocument::fromJson(m_file->readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        statusMessage(tr("Unable to parse Json document from %1. %2")
                      .arg(QDir::toNativeSeparators(m_file->fileName()),
                           err.errorString()), type);
        return false;
    }

    statusMessage(tr("Json document %1 opened")
                  .arg(QDir::toNativeSeparators(m_file->fileName())), type);
    m_file->close();

    maybeEnablePrinting();

    JsonItemModel *model = new JsonItemModel(m_root, this);
    m_tree->setModel(model);

    return true;
}

QModelIndex indexOf(const QListWidgetItem *item)
{
    return qvariant_cast<QModelIndex>(item->data(Qt::UserRole));
}

// Move to the clicked toplevel index
void JsonViewer::onTopLevelItemClicked(QListWidgetItem *item)
{
    // return in the unlikely case that the tree has not been built
    if (Q_UNLIKELY(!m_tree->model()))
        return;

    auto index = indexOf(item);
    if (Q_UNLIKELY(!index.isValid()))
        return;

    m_tree->setCurrentIndex(index);
}

// Toggle double clicked index between collaps/expand
void JsonViewer::onTopLevelItemDoubleClicked(QListWidgetItem *item)
{
    // return in the unlikely case that the tree has not been built
    if (Q_UNLIKELY(!m_tree->model()))
        return;

    auto index = indexOf(item);
    if (Q_UNLIKELY(!index.isValid()))
        return;

    if (m_tree->isExpanded(index)) {
        m_tree->collapse(index);
        return;
    }

    // Make sure the node and all parents are expanded
    while (index.isValid()) {
        m_tree->expand(index);
        index = index.parent();
    }
}

void JsonViewer::onJsonMenuRequested(const QPoint &pos)
{
    const auto &index = m_tree->indexAt(pos);
    if (!index.isValid())
        return;

    // Don't show a context menu, if the index is already a bookmark
    for (int i = 0; i < m_toplevel->count(); ++i) {
        if (indexOf(m_toplevel->item(i)) == index)
            return;
    }

    QMenu menu(m_tree);
    QAction *action = new QAction(tr("Add bookmark"));
    action->setData(index);
    menu.addAction(action);
    connect(action, &QAction::triggered, this, &JsonViewer::onBookmarkAdded);
    menu.exec(m_tree->mapToGlobal(pos));
}

void JsonViewer::onBookmarkMenuRequested(const QPoint &pos)
{
    auto *item = m_toplevel->itemAt(pos);
    if (!item)
        return;

    // Don't delete toplevel items
    const QModelIndex index = indexOf(item);
    if (!index.parent().isValid())
        return;

    QMenu menu;
    QAction *action = new QAction(tr("Delete bookmark"));
    action->setData(m_toplevel->row(item));
    menu.addAction(action);
    connect(action, &QAction::triggered, this, &JsonViewer::onBookmarkDeleted);
    menu.exec(m_toplevel->mapToGlobal(pos));
}

void JsonViewer::onBookmarkAdded()
{
    const QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;

    const QModelIndex index = qvariant_cast<QModelIndex>(action->data());
    if (!index.isValid())
        return;

    auto *item = new QListWidgetItem(index.data(Qt::DisplayRole).toString(), m_toplevel);
    item->setData(Qt::UserRole, index);

    // Set a tooltip that shows where the item is located in the tree
    QModelIndex parent = index.parent();
    QString tooltip = index.data(Qt::DisplayRole).toString();
    while (parent.isValid()) {
        tooltip = parent.data(Qt::DisplayRole).toString() + "->"_L1 + tooltip;
        parent = parent.parent();
    }
    item->setToolTip(tooltip);
}

void JsonViewer::onBookmarkDeleted()
{
    const QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;

    const int row = action->data().toInt();
    if (row < 0 || row >= m_toplevel->count())
        return;

    delete m_toplevel->takeItem(row);
}

bool JsonViewer::hasContent() const
{
    return !m_root.isEmpty();
}

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
void JsonViewer::printDocument(QPrinter *printer) const
{
    if (!hasContent())
        return;

    const QTextDocument doc(QString::fromUtf8(m_root.toJson(QJsonDocument::JsonFormat::Indented)));
    doc.print(printer);
}

#endif // DOCUMENTVIEWER_PRINTSUPPORT

QByteArray JsonViewer::saveState() const
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << QString(viewerName());
    stream << m_tree->header()->saveState();
    return array;
}

bool JsonViewer::restoreState(QByteArray &array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    QString viewer;
    stream >> viewer;
    if (viewer != viewerName())
        return false;
    QByteArray header;
    stream >> header;
    return m_tree->header()->restoreState(header);
}

void JsonViewer::retranslate()
{
    if (toolBars().isEmpty())
        return;
    menus().at(0)->setTitle(tr("Json"));
    toolBars().at(0)->setWindowTitle(tr("Json Actions"));
    m_expandAllAction->setText(tr("&+Expand all"));
    m_collapseAllAction->setText(tr("&-Collapse all"));

    if (m_toplevel && m_uiAssets.tabs) {
        // Update the tab title
        int tabIndex = m_uiAssets.tabs->indexOf(m_toplevel);
        if (tabIndex >= 0)
            m_uiAssets.tabs->setTabText(tabIndex, tr("Bookmarks"));
        // Update tooltip for all bookmark items
        for (int i = 0; i < m_toplevel->count(); ++i) {
            if (QListWidgetItem *item = m_toplevel->item(i))
                item->setToolTip(tr("Toplevel Item %1").arg(i));
        }
    }
}

JsonTreeItem::JsonTreeItem(JsonTreeItem *parent)
{
    m_parent = parent;
}

JsonTreeItem::~JsonTreeItem()
{
    qDeleteAll(m_children);
}

void JsonTreeItem::appendChild(JsonTreeItem *item)
{
    m_children.append(item);
}

JsonTreeItem *JsonTreeItem::child(int row)
{
    return m_children.value(row);
}

JsonTreeItem *JsonTreeItem::parent()
{
    return m_parent;
}

int JsonTreeItem::childCount() const
{
    return m_children.count();
}

int JsonTreeItem::row() const
{
    if (m_parent)
        return m_parent->m_children.indexOf(const_cast<JsonTreeItem*>(this));

    return 0;
}

void JsonTreeItem::setKey(const QString &key)
{
    m_key = key;
}

void JsonTreeItem::setValue(const QVariant &value)
{
    m_value = value;
}

void JsonTreeItem::setType(const QJsonValue::Type &type)
{
    m_type = type;
}

JsonTreeItem* JsonTreeItem::load(const QJsonValue& value, JsonTreeItem* parent)
{
    JsonTreeItem *rootItem = new JsonTreeItem(parent);
    rootItem->setKey("root"_L1);

    if (value.isObject()) {
        const QStringList &keys = value.toObject().keys();
        for (const QString &key : keys) {
            QJsonValue v = value.toObject().value(key);
            JsonTreeItem *child = load(v, rootItem);
            child->setKey(key);
            child->setType(v.type());
            rootItem->appendChild(child);
        }
    } else if (value.isArray()) {
        int index = 0;
        const QJsonArray &array = value.toArray();
        for (const QJsonValue &val : array) {
            JsonTreeItem *child = load(val, rootItem);
            child->setKey(QString::number(index));
            child->setType(val.type());
            rootItem->appendChild(child);
            ++index;
        }
    } else {
        rootItem->setValue(value.toVariant());
        rootItem->setType(value.type());
    }

    return rootItem;
}

JsonItemModel::JsonItemModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_rootItem{new JsonTreeItem}
{
    m_headers.append("Key"_L1);
    m_headers.append("Value"_L1);
}

JsonItemModel::JsonItemModel(const QJsonDocument &doc, QObject *parent)
    : QAbstractItemModel(parent)
    , m_rootItem{new JsonTreeItem}
{
    // Append header lines and return on empty document
    m_headers.append("Key"_L1);
    m_headers.append("Value"_L1);
    if (doc.isNull())
        return;

    // Reset the model. Root can either be a value or an array.
    beginResetModel();
    delete m_rootItem;
    if (doc.isArray()) {
        m_rootItem = JsonTreeItem::load(QJsonValue(doc.array()));
        m_rootItem->setType(QJsonValue::Array);

    } else {
        m_rootItem = JsonTreeItem::load(QJsonValue(doc.object()));
        m_rootItem->setType(QJsonValue::Object);
    }
    endResetModel();
}

JsonItemModel::~JsonItemModel()
{
    delete m_rootItem;
}

QVariant JsonItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    JsonTreeItem *item = itemFromIndex(index);

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return item->key();
        if (index.column() == 1)
            return item->value();
        break;
    case Qt::EditRole:
        if (index.column() == 1)
            return item->value();
        break;
    default:
        break;
    }
    return {};
}

QVariant JsonItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal)
        return m_headers.value(section);
    else
        return {};
}

QModelIndex JsonItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    JsonTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = itemFromIndex(parent);

    JsonTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return {};
}

QModelIndex JsonItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    JsonTreeItem *childItem = itemFromIndex(index);
    JsonTreeItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int JsonItemModel::rowCount(const QModelIndex &parent) const
{
    JsonTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = itemFromIndex(parent);

    return parentItem->childCount();
}
