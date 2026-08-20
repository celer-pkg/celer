// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
#include "addressview.h"
#include "msoutl.h"
#include <QtWidgets>

class AddressBookModel : public QAbstractListModel
{
public:
    explicit AddressBookModel(AddressView *parent);
    virtual ~AddressBookModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;

    void changeItem(const QModelIndex &index, const QString &firstName, const QString &lastName, const QString &address, const QString &email);
    void addItem(const QString &firstName, const QString &lastName, const QString &address, const QString &email);
    void update();

private:
    Outlook::Application outlook;
    Outlook::Items *folderItems = nullptr;

    mutable QHash<QModelIndex, QStringList> cache;
};
//! [0] //! [1]

AddressBookModel::AddressBookModel(AddressView *parent)
: QAbstractListModel(parent)
{
    if (!outlook.isNull()) {
        Outlook::NameSpace session(outlook.Session());
        session.Logon();
        Outlook::MAPIFolder *folder = session.GetDefaultFolder(Outlook::olFolderContacts);
        folderItems = new Outlook::Items(folder->Items());
        connect(folderItems, SIGNAL(ItemAdd(IDispatch*)), parent, SLOT(updateOutlook()));
        connect(folderItems, SIGNAL(ItemChange(IDispatch*)), parent, SLOT(updateOutlook()));
        connect(folderItems, SIGNAL(ItemRemove()), parent, SLOT(updateOutlook()));

        delete folder;
    }
}

//! [1] //! [2]
AddressBookModel::~AddressBookModel()
{
    delete folderItems;

    if (!outlook.isNull())
        Outlook::NameSpace(outlook.Session()).Logoff();
}

//! [2] //! [3]
int AddressBookModel::rowCount(const QModelIndex &) const
{
    return folderItems ? folderItems->Count() : 0;
}

int AddressBookModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 4;
}

//! [3] //! [4]
QVariant AddressBookModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case 0:
        return tr("First Name");
    case 1:
        return tr("Last Name");
    case 2:
        return tr("Address");
    case 3:
        return tr("Email");
    default:
        break;
    }

    return QVariant();
}

//! [4] //! [5]
QVariant AddressBookModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    QStringList data;
    if (cache.contains(index)) {
        data = cache.value(index);
    } else {
        Outlook::ContactItem contact(folderItems->Item(index.row() + 1));

        if (contact.Class() == Outlook::OlObjectClass::olContact)
            data << contact.FirstName() << contact.LastName() << contact.HomeAddress() << contact.Email1Address();

        cache.insert(index, data);
    }

    if (index.column() < data.size())
        return data.at(index.column());

    return QVariant();
}

//! [5] //! [6]
void AddressBookModel::changeItem(const QModelIndex &index, const QString &firstName, const QString &lastName, const QString &address, const QString &email)
{
    Outlook::ContactItem item(folderItems->Item(index.row() + 1));

    if (item.Class() != Outlook::OlObjectClass::olContact)
        return; // Not a contact

    item.SetFirstName(firstName);
    item.SetLastName(lastName);
    item.SetHomeAddress(address);
    item.SetEmail1Address(email);

    item.Save();

    cache.take(index);
}

//! [6] //! [7]
void AddressBookModel::addItem(const QString &firstName, const QString &lastName, const QString &address, const QString &email)
{
    Outlook::ContactItem item(outlook.CreateItem(Outlook::olContactItem));
    if (!item.isNull()) {
        item.SetFirstName(firstName);
        item.SetLastName(lastName);
        item.SetHomeAddress(address);
        item.SetEmail1Address(email);

        item.Save();
    }
}

//! [7] //! [8]
void AddressBookModel::update()
{
    beginResetModel();
    cache.clear();
    endResetModel();
}

//! [8] //! [9]
AddressView::AddressView(QWidget *parent)
: QWidget(parent)
{
    QGridLayout *mainGrid = new QGridLayout(this);

    QLabel *firstNameLabel = new QLabel(tr("First &Name"), this);
    firstNameLabel->resize(firstNameLabel->sizeHint());
    mainGrid->addWidget(firstNameLabel, 0, 0);

    QLabel *lastNameLabel = new QLabel(tr("&Last Name"), this);
    lastNameLabel->resize(lastNameLabel->sizeHint());
    mainGrid->addWidget(lastNameLabel, 0, 1);

    QLabel *addressLabel = new QLabel(tr("Add&ress"), this);
    addressLabel->resize(addressLabel->sizeHint());
    mainGrid->addWidget(addressLabel, 0, 2);

    QLabel *emailLabel = new QLabel(tr("&E-Mail"), this);
    emailLabel->resize(emailLabel->sizeHint());
    mainGrid->addWidget(emailLabel, 0, 3);

    m_addButton = new QPushButton(tr("A&dd"), this);
    m_addButton->resize(m_addButton->sizeHint());
    mainGrid->addWidget(m_addButton, 0, 4);
    connect(m_addButton, &QPushButton::clicked, this, &AddressView::addEntry);

    m_firstName = new QLineEdit(this);
    m_firstName->resize(m_firstName->sizeHint());
    mainGrid->addWidget(m_firstName, 1, 0);
    firstNameLabel->setBuddy(m_firstName);

    m_lastName = new QLineEdit(this);
    m_lastName->resize(m_lastName->sizeHint());
    mainGrid->addWidget(m_lastName, 1, 1);
    lastNameLabel->setBuddy(m_lastName);

    m_address = new QLineEdit(this);
    m_address->resize(m_address->sizeHint());
    mainGrid->addWidget(m_address, 1, 2);
    addressLabel->setBuddy(m_address);

    m_email = new QLineEdit(this);
    m_email->resize(m_email->sizeHint());
    mainGrid->addWidget(m_email, 1, 3);
    emailLabel->setBuddy(m_email);

    m_changeButton = new QPushButton(tr("&Change"), this);
    m_changeButton->resize(m_changeButton->sizeHint());
    mainGrid->addWidget(m_changeButton, 1, 4);
    connect(m_changeButton, &QPushButton::clicked, this, &AddressView::changeEntry);

    m_treeView = new QTreeView(this);
    m_treeView->setSelectionMode(QTreeView::SingleSelection);
    m_treeView->setRootIsDecorated(false);

    model = new AddressBookModel(this);
    m_treeView->setModel(model);

    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AddressView::itemSelected);

    mainGrid->addWidget(m_treeView, 2, 0, 1, 5);
}

void AddressView::updateOutlook()
{
    model->update();
}

void AddressView::addEntry()
{
    if (!m_firstName->text().isEmpty() || !m_lastName->text().isEmpty() ||
         !m_address->text().isEmpty() || !m_email->text().isEmpty()) {
        model->addItem(m_firstName->text(), m_lastName->text(), m_address->text(), m_email->text());
    }

    m_firstName->clear();
    m_lastName->clear();
    m_address->clear();
    m_email->clear();
}

void AddressView::changeEntry()
{
    QModelIndex current = m_treeView->currentIndex();

    if (current.isValid())
        model->changeItem(current, m_firstName->text(), m_lastName->text(), m_address->text(), m_email->text());
}

//! [9] //! [10]
void AddressView::itemSelected(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QAbstractItemModel *model = m_treeView->model();
    m_firstName->setText(model->data(model->index(index.row(), 0)).toString());
    m_lastName->setText(model->data(model->index(index.row(), 1)).toString());
    m_address->setText(model->data(model->index(index.row(), 2)).toString());
    m_email->setText(model->data(model->index(index.row(), 3)).toString());
}
//! [10]
