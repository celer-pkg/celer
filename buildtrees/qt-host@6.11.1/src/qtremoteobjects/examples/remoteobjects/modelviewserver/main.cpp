// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QTreeView>
#include <QApplication>
#include <QRemoteObjectNode>
#include <QTimer>
#include <QStandardItemModel>
#include <QStandardItem>

#include <memory>

struct TimerHandler : public QObject
{
    Q_OBJECT
public:
    QStandardItemModel *model;
public Q_SLOTS:
    void changeData() {
        Q_ASSERT(model);
        Q_ASSERT(model->rowCount() > 50);
        Q_ASSERT(model->columnCount() > 1);
        for (int i = 10; i < 50; ++i)
            model->setData(model->index(i, 1), QColor(Qt::blue), Qt::BackgroundRole);
    }
    void insertData() {
        Q_ASSERT(model);
        Q_ASSERT(model->rowCount() > 50);
        Q_ASSERT(model->columnCount() > 1);
        model->insertRows(2, 9);
        for (int i = 2; i < 11; ++i) {
            model->setData(model->index(i, 1), QColor(Qt::green), Qt::BackgroundRole);
            model->setData(model->index(i, 1), QLatin1String("InsertedRow"), Qt::DisplayRole);
        }
    }
    void removeData() {
        model->removeRows(2, 4);
    }

    void changeFlags() {
        QStandardItem *item = model->item(0, 0);
        item->setEnabled(false);
        item = item->child(0, 0);
        item->setFlags(item->flags() & Qt::ItemIsSelectable);
    }

    void moveData() {
        model->moveRows(QModelIndex(), 2, 4, QModelIndex(), 10);
    }
};

QList<QStandardItem*> addChild(int numChildren, int nestingLevel)
{
    QList<QStandardItem*> result;
    if (nestingLevel == 0)
        return result;
    for (int i = 0; i < numChildren; ++i) {
        QStandardItem *child = new QStandardItem(QStringLiteral("Child num %1, nesting Level %2").arg(i+1).arg(nestingLevel));
        if (i == 0)
            child->appendRow(addChild(numChildren, nestingLevel -1));
        result.push_back(child);
    }
    return result;
}

std::unique_ptr<QStandardItemModel> createModel()
{
    std::unique_ptr<QStandardItemModel> sourceModel = std::make_unique<QStandardItemModel>();
    const int modelSize = 100000;
    QStringList list;
    QStringList hHeaderList;
    hHeaderList << QStringLiteral("First Column with spacing") << QStringLiteral("Second Column with spacing");
    sourceModel->setHorizontalHeaderLabels(hHeaderList);
    list.reserve(modelSize);
    for (int i = 0; i < modelSize; ++i) {
        QStandardItem *firstItem = new QStandardItem(QStringLiteral("FancyTextNumber %1").arg(i));
        if (i == 0)
            firstItem->appendRow(addChild(2, 2));
        QStandardItem *secondItem = new QStandardItem(QStringLiteral("FancyRow2TextNumber %1").arg(i));
        if (i % 2 == 0)
            firstItem->setBackground(Qt::red);
        QList<QStandardItem*> row;
        row << firstItem << secondItem;
        sourceModel->invisibleRootItem()->appendRow(row);
        list << QStringLiteral("FancyTextNumber %1").arg(i);
    }

    // Needed by QMLModelViewClient
    QHash<int,QByteArray> roleNames = {
        {Qt::DisplayRole, "_text"},
        {Qt::BackgroundRole, "_color"}
    };
    sourceModel->setItemRoleNames(roleNames);
    return sourceModel;
}

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules("qt.remoteobjects.debug=false\n"
                                     "qt.remoteobjects.warning=false");
    QApplication app(argc, argv);

    qDebug() << "Creating registry host";
//! [RegistryHost setup]
    QRemoteObjectRegistryHost node(QUrl(QStringLiteral("local:registry")));
//! [RegistryHost setup]

//! [Model-creation and role-selection]
    std::unique_ptr<QStandardItemModel> sourceModel = createModel();

    QList<int> roles;
    roles << Qt::DisplayRole << Qt::BackgroundRole;
//! [Model-creation and role-selection]

//! [Model-remoting]
    QRemoteObjectHost node2(QUrl(QStringLiteral("local:replica")), QUrl(QStringLiteral("local:registry")));
    node2.enableRemoting(sourceModel.get(), QStringLiteral("RemoteModel"), roles);
//! [Model-remoting]

//! [TreeView-creation]
    QTreeView view;
    view.setWindowTitle(QStringLiteral("SourceView"));
    view.setModel(sourceModel.get());
    view.show();
//! [TreeView-creation]

//! [Automated actions]
    TimerHandler handler;
    handler.model = sourceModel.get();
    QTimer::singleShot(5000, &handler, &TimerHandler::changeData);
    QTimer::singleShot(10000, &handler, &TimerHandler::insertData);
    QTimer::singleShot(11000, &handler, &TimerHandler::changeFlags);
    QTimer::singleShot(12000, &handler, &TimerHandler::removeData);
    QTimer::singleShot(13000, &handler, &TimerHandler::moveData);
//! [Automated actions]


    return app.exec();
}

#include "main.moc"
