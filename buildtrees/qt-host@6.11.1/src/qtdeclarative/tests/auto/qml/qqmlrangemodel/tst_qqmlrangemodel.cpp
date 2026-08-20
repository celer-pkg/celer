// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>
#include <QtCore/qhash.h>
#include <QtCore/qitemselectionmodel.h>
#include <QtCore/qrangemodel.h>
#include <QtQmlModels/private/qqmldelegatemodel_p.h>
#include <QtQuick/qquickview.h>
#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtQuickTestUtils/private/viewtestutils_p.h>

using namespace Qt::StringLiterals;

class tst_QQmlRangeModel : public QQmlDataTest
{
    Q_OBJECT

public:
    tst_QQmlRangeModel()
        : QQmlDataTest(QT_QMLTEST_DATADIR)
    {}

private:
    using RoleNames = QHash<int, QByteArray>;

    std::unique_ptr<QQuickView> makeView(const QVariantMap &properties) const;

    void listTest_data();
    void rangeModelTest_data();

    // subclass of QRangeModel allowing us to monitor API traffic
    struct RangeModel : QRangeModel
    {
        template <typename Data>
        RangeModel(Data &&data)
            : QRangeModel(std::forward<Data>(data))
        {}

        mutable QList<int> dataCalls;
        QList<int> setDataCalls;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
        {
            dataCalls << role;
            return QRangeModel::data(index, role);
        }

        bool setData(const QModelIndex &index, const QVariant &data, int role = Qt::EditRole) override
        {
            setDataCalls << role;
            return QRangeModel::setData(index, data, role);
        }
    };

private slots:
    // reference cases using QList<...> as models
    void variantList_data() { listTest_data(); }
    void variantList();
    void objectList_data() { listTest_data(); }
    void objectList();
    void gadgetList_data() { listTest_data(); }
    void gadgetList();

    // QRangeModel tests
    void intRange_data() { rangeModelTest_data(); }
    void intRange();
    void objectRange_data() { rangeModelTest_data(); }
    void objectRange();
    void gadgetRange_data() { rangeModelTest_data(); }
    void gadgetRange();

    void gadgetTable_data() { rangeModelTest_data(); }
    void gadgetTable();
};

class Entry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int number READ number WRITE setNumber NOTIFY numberChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    enum EntryRoles {
        NumberRole = Qt::UserRole,
        TextRole,
    };
    Entry(int number, const QString &text)
        : m_number(number), m_text(text)
    {}

    int number() const { return m_number; }
    void setNumber(int number)
    {
        if (m_number == number)
            return;
        m_number = number;
        emit numberChanged();
    }

    QString text() const { return m_text; }
    void setText(const QString &text)
    {
        if (m_text == text)
            return;
        m_text = text;
        emit textChanged();
    }

    QString toString() const
    {
        return u"%1: %2"_s.arg(m_number).arg(m_text);
    }

signals:
    void numberChanged();
    void textChanged();

private:
    int m_number;
    QString m_text;
};

template <>
struct QRangeModel::RowOptions<Entry>
{
    static constexpr auto rowCategory = RowCategory::MultiRoleItem;
};

class Gadget
{
    Q_GADGET
    Q_PROPERTY(int number READ number WRITE setNumber)
    Q_PROPERTY(QString text READ text WRITE setText)
    QML_VALUE_TYPE(gadget)

public:
    enum GadgetRoles {
        NumberRole = Qt::UserRole,
        TextRole,
    };

    Gadget() : m_number(-1) {}

    Gadget(int number, const QString &text)
        : m_number(number), m_text(text)
    {}

    int number() const { return m_number; }
    void setNumber(int number) { m_number = number; }

    QString text() const { return m_text; }
    void setText(const QString &text) { m_text = text; }

private:
    friend bool operator==(const Gadget &lhs, const Gadget &rhs)
    {
        return lhs.m_number == rhs.m_number
            && lhs.m_text == rhs.m_text;
    }
    int m_number;
    QString m_text;
};

template <>
struct QRangeModel::RowOptions<Gadget>
{
    static constexpr auto rowCategory = RowCategory::MultiRoleItem;
};


std::unique_ptr<QQuickView> tst_QQmlRangeModel::makeView(const QVariantMap &properties) const
{
    auto view = std::make_unique<QQuickView>();
    view->setInitialProperties(properties);

    const QString testFunction = QString::fromUtf8(QTest::currentTestFunction());
    if (!QQuickTest::showView(*view, testFileUrl(testFunction + ".qml")))
        return {};
    return view;
}

// The first two tests are for reference, documenting how modelData works with
// lists as models.
void tst_QQmlRangeModel::listTest_data()
{
    QTest::addColumn<QQmlDelegateModel::DelegateModelAccess>("delegateModelAccess");
    QTest::addColumn<bool>("writeBack");

    QTest::addRow("ReadOnly") << QQmlDelegateModel::ReadOnly << false;
    QTest::addRow("ReadWrite") << QQmlDelegateModel::ReadWrite << true;
}

void tst_QQmlRangeModel::variantList()
{
    QFETCH(const QQmlDelegateModel::DelegateModelAccess, delegateModelAccess);

    QVariantList numbers = {1};
    auto view = makeView({
        {"delegateModelAccess", delegateModelAccess},
        {"model", QVariant::fromValue(numbers)}
    });
    QVERIFY(view);

    QObject *currentItem = nullptr;
    QTRY_VERIFY(currentItem = view->rootObject()->property("currentItem").value<QObject *>());
    QSignalSpy currentValueSpy(currentItem, SIGNAL(currentValueChanged()));
    auto currentValue = currentItem->property("currentValue");
    QCOMPARE(currentValue, numbers.at(0));

    QMetaObject::invokeMethod(currentItem, "setValue", 42);
    QCOMPARE(currentValueSpy.count(), 1);
    QCOMPARE(currentItem->property("currentValue"), 42);

    // the view changing the model cannot modify the C++ data, not even
    // with ReadWrite model access, as we pass a copy of QVariantList.
    QCOMPARE(numbers, QVariantList{1});
}

void tst_QQmlRangeModel::objectList()
{
    QFETCH(const QQmlDelegateModel::DelegateModelAccess, delegateModelAccess);
    QFETCH(const bool, writeBack);

    QPointer<Entry> entry = new Entry(1, "one");
    QList<Entry *> objects = {
        entry.data(),
    };
    auto cleanup = qScopeGuard([&objects]{ qDeleteAll(objects); });

    auto view = makeView({
        {"delegateModelAccess", delegateModelAccess},
        {"model", QVariant::fromValue(objects)}
    });
    QVERIFY(view);

    QObject *currentItem = nullptr;
    QTRY_VERIFY(currentItem = view->rootObject()->property("currentItem").value<QObject *>());
    QSignalSpy currentValueSpy(currentItem, SIGNAL(currentValueChanged()));
    QSignalSpy currentDataSpy(currentItem, SIGNAL(currentDataChanged()));
    auto currentValue = currentItem->property("currentValue");

    QCOMPARE(currentValue, objects.at(0)->toString());

    // changing the required properties from QML...
    QMetaObject::invokeMethod(currentItem, "setValue", 42);
    QCOMPARE(currentValueSpy.count(), 1);

    // ... changes modelData and C++ side only in ReadWrite access mode
    QCOMPARE(currentDataSpy.count(), writeBack ? 1 : 0);
    QCOMPARE(currentItem->property("currentValue"), "42: one");
    QCOMPARE(currentItem->property("currentData"), writeBack ? "42: one" : "1: one");
    QCOMPARE(entry->number(), writeBack ? 42 : 1);

    // changing C++ does update required property values in either case
    entry->setText("fortytwo");
    QCOMPARE(currentValueSpy.count(), 2);
    QCOMPARE(currentItem->property("currentValue"), "42: fortytwo");

    // and updates modelData
    QCOMPARE(currentDataSpy.count(), writeBack ? 2 : 1);
    QCOMPARE(currentItem->property("currentData"), entry->toString());

    // replacing modelData triggers refresh of required properties, but also
    // messes things up a bit
    auto newEntry = std::make_unique<Entry>(2, "two");
#ifndef QT_NO_DEBUG
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("TypeError: Cannot read property '.*' of null"));
#endif
    QMetaObject::invokeMethod(currentItem, "setModelData", QVariant::fromValue(newEntry.get()));
    QVERIFY(entry); // old object still alive
    QCOMPARE(currentItem->property("currentValue"), "42: fortytwo");
    QCOMPARE(entry->toString(), writeBack ? "42: fortytwo" : "1: fortytwo");
    QCOMPARE(currentItem->property("currentData"), "2: two");
    QCOMPARE(newEntry->toString(), "2: two");
}

void tst_QQmlRangeModel::gadgetList()
{
    QFETCH(const QQmlDelegateModel::DelegateModelAccess, delegateModelAccess);

    // the only way to get a list of gadgets into QML is via a QVariantList
    const Gadget oldValue = Gadget{1, "one"};
    QVariantList gadgets {
        QVariant::fromValue(oldValue),
        QVariant::fromValue(Gadget{2, "two"}),
    };

    auto view = makeView({
        {"delegateModelAccess", delegateModelAccess},
        {"model", QVariant::fromValue(gadgets)}
    });
    QVERIFY(view);

    QObject *currentItem = nullptr;
    QTRY_VERIFY(currentItem = view->rootObject()->property("currentItem").value<QObject *>());
    auto currentData = currentItem->property("modelData");
    QCOMPARE(currentData.value<Gadget>(), oldValue);
    QCOMPARE(currentItem->property("text"), oldValue.text());

    const Gadget newValue = Gadget{42, "fortytwo"};
    QMetaObject::invokeMethod(currentItem, "setModelData", QVariant::fromValue(newValue));
    currentData = currentItem->property("modelData");
    QCOMPARE(currentData.value<Gadget>(), newValue);
    // replacing the gadget on the QML side updates bindings to modelData
    QCOMPARE(currentItem->property("number"), newValue.number());
    // but not required properties
    QCOMPARE(currentItem->property("text"), oldValue.text());

    // but since nothing can be written back, changes will not outlive the delegate
    QCOMPARE(gadgets.at(0).value<Gadget>(), oldValue);
}

// The first two tests are for reference, documenting how modelData works with
// lists as models.
void tst_QQmlRangeModel::rangeModelTest_data()
{
    QTest::addColumn<QQmlDelegateModel::DelegateModelAccess>("delegateModelAccess");
    QTest::addColumn<bool>("writeBack");

    QTest::addRow("ReadOnly")
        << QQmlDelegateModel::ReadOnly << false;
    QTest::addRow("ReadWrite")
        << QQmlDelegateModel::ReadWrite << true;
}

void tst_QQmlRangeModel::intRange()
{
    QFETCH(const QQmlDelegateModel::DelegateModelAccess, delegateModelAccess);
    QFETCH(const bool, writeBack);

    const int oldValue = 42;
    std::vector<int> data{oldValue};
    RangeModel model(&data);

    auto view = makeView({
        {"delegateModelAccess", delegateModelAccess},
        {"model", QVariant::fromValue(&model)}
    });

    QVERIFY(view);
    QObject *currentItem = nullptr;
    QTRY_VERIFY(currentItem = view->rootObject()->property("currentItem").value<QObject *>());
    QCOMPARE(currentItem->property("currentValue"), oldValue);

    // nothing happened so far, so there shouldn't have been any calls to setData
    QCOMPARE(model.setDataCalls, QList<int>{});
    model.setDataCalls.clear();
    model.dataCalls.clear();

    // Changing the data via QAIM api...
    const QModelIndex index = model.index(0, 0);
    const QVariant newValue = 7;
    QVERIFY(model.setData(index, newValue, Qt::RangeModelDataRole)); // default: Qt::EditRole
    // ... should give us one call to setData (our own)
    QCOMPARE(model.setDataCalls, QList<int>{Qt::RangeModelDataRole});
    model.setDataCalls.clear();
    // ... and results in a single call to data() to get the new value
    QCOMPARE(model.dataCalls, QList<int>{Qt::RangeModelDataRole});
    model.dataCalls.clear();
    // ... which updates the QML side
    QCOMPARE(currentItem->property("currentValue"), newValue);

    // The delegate changing the property ...
    QMetaObject::invokeMethod(currentItem, "setValue", oldValue);
    // ... should result in a single call to QRM::data()
    QCOMPARE(model.dataCalls, writeBack ? QList<int>{Qt::RangeModelDataRole} : QList<int>{});
    // ... and one call to setData if access mode is ReadWrite
    QCOMPARE(model.setDataCalls, writeBack ? QList<int>{Qt::RangeModelDataRole} : QList<int>{});
    // ... which writes back to the model and updates our data structure
    QCOMPARE(model.data(index) == oldValue, writeBack);
    QCOMPARE(data.at(0) == oldValue, writeBack);
}

void tst_QQmlRangeModel::objectRange()
{
    QFETCH(const QQmlDelegateModel::DelegateModelAccess, delegateModelAccess);
    QFETCH(const bool, writeBack);

    QScopedPointer<Entry> entry(new Entry(1, "one"));
    std::vector<Entry *> objects{entry.get()};
    RangeModel model(&objects);

    auto view = makeView({
        {"delegateModelAccess", delegateModelAccess},
        {"model", QVariant::fromValue(&model)}
    });

    QVERIFY(view);
    QObject *currentItem = nullptr;
    QTRY_VERIFY(currentItem = view->rootObject()->property("currentItem").value<QObject *>());

    // loading should call data() for all bound properties
    QVERIFY(model.dataCalls.contains(Entry::NumberRole));
    QVERIFY(model.dataCalls.contains(Qt::RangeModelDataRole));
    model.dataCalls.clear();
    // there shouldn't have been any attempts to write yet
    QCOMPARE(model.setDataCalls, QList<int>{});
    model.setDataCalls.clear();

    const QModelIndex index = model.index(0, 0);
    const QVariant oldNumber = entry->number();
    const QVariant newNumber = 2;
    // Changing bound-to data via QAIM API...
    model.setData(index, newNumber, Entry::NumberRole);
    // .. calls data once, for that role
    QCOMPARE(model.dataCalls, QList<int>{Entry::NumberRole});
    // ... to update the QML properties
    QCOMPARE(currentItem->property("number"), newNumber);
    QCOMPARE(currentItem->property("modelNumber"), newNumber);
    // ... and there should only be our call to setData
    QCOMPARE(model.setDataCalls, QList<int>{Entry::NumberRole});
    model.setDataCalls.clear();
    model.dataCalls.clear();

    // changing a property on the QML side ...
    QMetaObject::invokeMethod(currentItem, "setValue", oldNumber);
    // ... should call QRM::setData for the changed role, if write back is enabled
    QCOMPARE(model.setDataCalls, writeBack ? QList<int>{Entry::NumberRole} : QList<int>{});
    // ... to update our model, and the backing QObject
    QCOMPARE(entry->number(), writeBack ? oldNumber : newNumber);
    QCOMPARE(currentItem->property("number"), oldNumber);
    // ... and call QRM::data, once, to get the new value
    QCOMPARE(model.dataCalls, writeBack ? QList<int>{Entry::NumberRole} : QList<int>{});
    model.dataCalls.clear();
    model.setDataCalls.clear();
}

void tst_QQmlRangeModel::gadgetRange()
{
    QFETCH(const QQmlDelegateModel::DelegateModelAccess, delegateModelAccess);
    QFETCH(const bool, writeBack);

    Gadget oldValue = {1, "one"};
    std::vector<Gadget> gadgets{oldValue};
    RangeModel model(&gadgets);

    auto view = makeView({
        {"delegateModelAccess", delegateModelAccess},
        {"model", QVariant::fromValue(&model)}
    });

    QVERIFY(view);
    QObject *currentItem = nullptr;
    QTRY_VERIFY(currentItem = view->rootObject()->property("currentItem").value<QObject *>());
    auto currentData = currentItem->property("modelData");
    QCOMPARE(currentData.value<Gadget>(), oldValue);
    QCOMPARE(currentItem->property("text"), oldValue.text());

    // setting modelData on the QML side...
    const Gadget newValue = Gadget{42, "fortytwo"};
    QMetaObject::invokeMethod(currentItem, "setModelData", QVariant::fromValue(newValue));
    currentData = currentItem->property("modelData");
    QCOMPARE(currentData.value<Gadget>(), newValue);
    // ... updates bindings to modelData
    QCOMPARE(currentItem->property("number"), newValue.number());
    // ... and, with ReadWrite, required properties
    QCOMPARE(currentItem->property("text"), writeBack ? newValue.text() : oldValue.text());
    // ... as well as the C++ data storage
    QCOMPARE(gadgets.at(0), writeBack ? newValue : oldValue);

    // updating the model using QAIM API updates all QML properties,
    // in all access modes
    const Gadget newestValue = Gadget(2, "two");
    const QModelIndex index = model.index(0, 0);
    QVERIFY(model.setData(index, QVariant::fromValue(newestValue), Qt::RangeModelDataRole));
    QCOMPARE(currentItem->property("text"), newestValue.text());
    QCOMPARE(currentItem->property("number"), newestValue.number());

    // updating a required property on the QML side...
    const QString newText = "three";
    QMetaObject::invokeMethod(currentItem, "setValue", QVariant(newText));
    // ... updates the model for ReadWrite access.
    QCOMPARE(gadgets.at(0).text(), writeBack ? newText : newestValue.text());
}

void tst_QQmlRangeModel::gadgetTable()
{
    QFETCH(const QQmlDelegateModel::DelegateModelAccess, delegateModelAccess);
    QFETCH(const bool, writeBack);

    Gadget oldGadget = {11, "1.a"};
    std::vector<std::pair<Gadget, Gadget>> gadgets{
        {oldGadget, {12, "1.b"}},
        {{21, "2.a"}, {22, "2.b"}},
    };
    RangeModel model(&gadgets);

    auto view = makeView({
        {"delegateModelAccess", delegateModelAccess},
        {"model", QVariant::fromValue(&model)}
    });

    QVERIFY(view);
    QObject *currentItem = nullptr;
    QTRY_VERIFY(currentItem = view->rootObject()->property("currentItem").value<QObject *>());
    auto currentData = currentItem->property("text");

    auto *selectionModel = view->rootObject()->property("selectionModel").value<QItemSelectionModel *>();
    QVERIFY(selectionModel);
    const QModelIndex index = selectionModel->currentIndex();
    QVERIFY(index.isValid());

    const QString oldText = gadgets.at(0).second.text();
    const QString newText = "1.A";

    // updating data via QAIM API
    model.setData(index, newText, Gadget::TextRole);
    // ... updates delegate
    QCOMPARE(currentItem->property("text"), newText);
    // ... and C++ data
    QCOMPARE(gadgets.at(0).first.text(), newText);

    // updating properties in QML
    QMetaObject::invokeMethod(currentItem, "setValue", oldText);
    // ... updates model and C++ in ReadWrite access mode
    QCOMPARE(model.data(index, Gadget::TextRole), writeBack ? oldText : newText);
    QCOMPARE(gadgets.at(0).first.text(), writeBack ? oldText : newText);

    // replaceing the gadget via QAIM API
    Gadget newGadget{33, "3.c"};
    model.setData(index, QVariant::fromValue(newGadget), Qt::RangeModelDataRole);
    // ... updates delegate and C++
    QCOMPARE(currentItem->property("modelData").value<Gadget>(), newGadget);
    QCOMPARE(gadgets.at(0).first, newGadget);

    // updating the gadget in QML
    QMetaObject::invokeMethod(currentItem, "setModelData", QVariant::fromValue(oldGadget));
    // ... updates the model and C++ in ReadWrite access mode
    QCOMPARE(model.data(index, Qt::RangeModelDataRole).value<Gadget>(),
             writeBack ? oldGadget : newGadget);

    // updating a gadget property in QML
    QMetaObject::invokeMethod(currentItem, "setModelDataNumber", 42);
    // ... modifies the local copy and does nothing
    QCOMPARE(model.data(index, Qt::RangeModelDataRole).value<Gadget>(),
             writeBack ? oldGadget : newGadget);
}

QTEST_MAIN(tst_QQmlRangeModel)

#include "tst_qqmlrangemodel.moc"
