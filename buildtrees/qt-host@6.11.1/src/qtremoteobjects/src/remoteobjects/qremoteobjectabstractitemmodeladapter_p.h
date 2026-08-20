// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QREMOTEOBJECTS_ABSTRACT_ITEM_ADAPTER_P_H
#define QREMOTEOBJECTS_ABSTRACT_ITEM_ADAPTER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qremoteobjectabstractitemmodeltypes_p.h"
#include "qremoteobjectsource.h"

#include <QtCore/qsize.h>

QT_BEGIN_NAMESPACE

class QAbstractItemModel;
class QItemSelectionModel;

class QAbstractItemModelSourceAdapter : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QAbstractItemModelSourceAdapter(QAbstractItemModel *object, QItemSelectionModel *sel, const QList<int> &roles = QList<int>());
    Q_PROPERTY(QList<int> availableRoles READ availableRoles WRITE setAvailableRoles NOTIFY availableRolesChanged)
    Q_PROPERTY(QIntHash roleNames READ roleNames)
    static void registerTypes();
    QItemSelectionModel* selectionModel() const;

public Q_SLOTS:
    QList<int> availableRoles() const { return m_availableRoles; }
    void setAvailableRoles(QList<int> availableRoles)
    {
        if (availableRoles != m_availableRoles)
        {
            m_availableRoles = availableRoles;
            Q_EMIT availableRolesChanged();
        }
    }

    QIntHash roleNames() const {return m_model->roleNames();}

    QSize replicaSizeRequest(QtPrivate::IndexList parentList);
    QtPrivate::DataEntries replicaRowRequest(QtPrivate::IndexList start, QtPrivate::IndexList end, QList<int> roles);
    QVariantList replicaHeaderRequest(QList<Qt::Orientation> orientations, QList<int> sections, QList<int> roles);
    void replicaSetCurrentIndex(QtPrivate::IndexList index, QItemSelectionModel::SelectionFlags command);
    void replicaSetData(const QtPrivate::IndexList &index, const QVariant &value, int role);
    QtPrivate::MetaAndDataEntries replicaCacheRequest(size_t size, const QList<int> &roles);

    void sourceDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QList<int> & roles = QList<int> ()) const;
    void sourceRowsInserted(const QModelIndex & parent, int start, int end);
    void sourceColumnsInserted(const QModelIndex & parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex & parent, int start, int end);
    void sourceRowsMoved(const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationChild) const;
    void sourceCurrentChanged(const QModelIndex & current, const QModelIndex & previous);
    void sourceLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);
Q_SIGNALS:
    void availableRolesChanged();
    void dataChanged(QtPrivate::IndexList topLeft, QtPrivate::IndexList bottomRight, QList<int> roles) const;
    void rowsInserted(QtPrivate::IndexList parent, int start, int end) const;
    void rowsRemoved(QtPrivate::IndexList parent, int start, int end) const;
    void rowsMoved(QtPrivate::IndexList sourceParent, int sourceRow, int count, QtPrivate::IndexList destinationParent, int destinationChild) const;
    void currentChanged(QtPrivate::IndexList current, QtPrivate::IndexList previous);
    void columnsInserted(QtPrivate::IndexList parent, int start, int end) const;
    void layoutChanged(QtPrivate::IndexList parents, QAbstractItemModel::LayoutChangeHint hint);

private:
    QAbstractItemModelSourceAdapter();
    QList<QtPrivate::IndexValuePair> fetchTree(const QModelIndex &parent, size_t &size, const QList<int> &roles);

    QAbstractItemModel *m_model;
    QItemSelectionModel *m_selectionModel;
    QList<int> m_availableRoles;
};

template <class ObjectType, class AdapterType>
struct QAbstractItemAdapterSourceAPI : public SourceApiMap
{
    QAbstractItemAdapterSourceAPI(const QString &name)
        : SourceApiMap()
        , m_signalArgTypes {}
        , m_methodArgTypes {}
        , m_name(name)
    {
        m_properties[0] = 2;
        m_properties[1] = QtPrivate::qtro_property_index<AdapterType>(&AdapterType::availableRoles, static_cast<QList<int> (QObject::*)()>(nullptr),"availableRoles");
        m_properties[2] = QtPrivate::qtro_property_index<AdapterType>(&AdapterType::roleNames, static_cast<QIntHash (QObject::*)()>(nullptr),"roleNames");
        m_signals[0] = 10;
        m_signals[1] = QtPrivate::qtro_signal_index<AdapterType>(&AdapterType::availableRolesChanged, static_cast<void (QObject::*)()>(nullptr),m_signalArgCount+0,&m_signalArgTypes[0]);
        m_signals[2] = QtPrivate::qtro_signal_index<AdapterType>(&AdapterType::dataChanged, static_cast<void (QObject::*)(QtPrivate::IndexList,QtPrivate::IndexList,QList<int>)>(nullptr),m_signalArgCount+1,&m_signalArgTypes[1]);
        m_signals[3] = QtPrivate::qtro_signal_index<AdapterType>(&AdapterType::rowsInserted, static_cast<void (QObject::*)(QtPrivate::IndexList,int,int)>(nullptr),m_signalArgCount+2,&m_signalArgTypes[2]);
        m_signals[4] = QtPrivate::qtro_signal_index<AdapterType>(&AdapterType::rowsRemoved, static_cast<void (QObject::*)(QtPrivate::IndexList,int,int)>(nullptr),m_signalArgCount+3,&m_signalArgTypes[3]);
        m_signals[5] = QtPrivate::qtro_signal_index<AdapterType>(&AdapterType::rowsMoved, static_cast<void (QObject::*)(QtPrivate::IndexList,int,int,QtPrivate::IndexList,int)>(nullptr),m_signalArgCount+4,&m_signalArgTypes[4]);
        m_signals[6] = QtPrivate::qtro_signal_index<AdapterType>(&AdapterType::currentChanged, static_cast<void (QObject::*)(QtPrivate::IndexList,QtPrivate::IndexList)>(nullptr),m_signalArgCount+5,&m_signalArgTypes[5]);
        m_signals[7] = QtPrivate::qtro_signal_index<ObjectType>(&ObjectType::modelReset, static_cast<void (QObject::*)()>(nullptr),m_signalArgCount+6,&m_signalArgTypes[6]);
        m_signals[8] = QtPrivate::qtro_signal_index<ObjectType>(&ObjectType::headerDataChanged, static_cast<void (QObject::*)(Qt::Orientation,int,int)>(nullptr),m_signalArgCount+7,&m_signalArgTypes[7]);
        m_signals[9] = QtPrivate::qtro_signal_index<AdapterType>(&AdapterType::columnsInserted, static_cast<void (QObject::*)(QtPrivate::IndexList,int,int)>(nullptr),m_signalArgCount+8,&m_signalArgTypes[8]);
        m_signals[10] = QtPrivate::qtro_signal_index<AdapterType>(&AdapterType::layoutChanged, static_cast<void (QObject::*)(QtPrivate::IndexList,QAbstractItemModel::LayoutChangeHint)>(nullptr),m_signalArgCount+9,&m_signalArgTypes[9]);
        m_methods[0] = 6;
        m_methods[1] = QtPrivate::qtro_method_index<AdapterType>(&AdapterType::replicaSizeRequest, static_cast<void (QObject::*)(QtPrivate::IndexList)>(nullptr),"replicaSizeRequest(QtPrivate::IndexList)",m_methodArgCount+0,&m_methodArgTypes[0]);
        m_methods[2] = QtPrivate::qtro_method_index<AdapterType>(&AdapterType::replicaRowRequest, static_cast<void (QObject::*)(QtPrivate::IndexList,QtPrivate::IndexList,QList<int>)>(nullptr),"replicaRowRequest(QtPrivate::IndexList,QtPrivate::IndexList,QList<int>)",m_methodArgCount+1,&m_methodArgTypes[1]);
        m_methods[3] = QtPrivate::qtro_method_index<AdapterType>(&AdapterType::replicaHeaderRequest, static_cast<void (QObject::*)(QList<Qt::Orientation>,QList<int>,QList<int>)>(nullptr),"replicaHeaderRequest(QList<Qt::Orientation>,QList<int>,QList<int>)",m_methodArgCount+2,&m_methodArgTypes[2]);
        m_methods[4] = QtPrivate::qtro_method_index<AdapterType>(&AdapterType::replicaSetCurrentIndex, static_cast<void (QObject::*)(QtPrivate::IndexList,QItemSelectionModel::SelectionFlags)>(nullptr),"replicaSetCurrentIndex(QtPrivate::IndexList,QItemSelectionModel::SelectionFlags)",m_methodArgCount+3,&m_methodArgTypes[3]);
        m_methods[5] = QtPrivate::qtro_method_index<AdapterType>(&AdapterType::replicaSetData, static_cast<void (QObject::*)(QtPrivate::IndexList,QVariant,int)>(nullptr),"replicaSetData(QtPrivate::IndexList,QVariant,int)",m_methodArgCount+4,&m_methodArgTypes[4]);
        m_methods[6] = QtPrivate::qtro_method_index<AdapterType>(&AdapterType::replicaCacheRequest, static_cast<void (QObject::*)(size_t,QList<int>)>(nullptr),"replicaCacheRequest(size_t,QList<int>)",m_methodArgCount+5,&m_methodArgTypes[5]);
    }

    QString name() const override { return m_name; }
    QString typeName() const override { return QStringLiteral("QAbstractItemModelAdapter"); }
    int enumCount() const override { return 0; }
    int propertyCount() const override { return m_properties[0]; }
    int signalCount() const override { return m_signals[0]; }
    int methodCount() const override { return m_methods[0]; }
    int sourceEnumIndex(int /*index*/) const override
    {
        return -1;
    }
    int sourcePropertyIndex(int index) const override
    {
        if (index < 0 || index >= m_properties[0])
            return -1;
        return m_properties[index+1];
    }
    int sourceSignalIndex(int index) const override
    {
        if (index < 0 || index >= m_signals[0])
            return -1;
        return m_signals[index+1];
    }
    int sourceMethodIndex(int index) const override
    {
        if (index < 0 || index >= m_methods[0])
            return -1;
        return m_methods[index+1];
    }
    int signalParameterCount(int index) const override { return m_signalArgCount[index]; }
    int signalParameterType(int sigIndex, int paramIndex) const override { return m_signalArgTypes[sigIndex][paramIndex]; }
    int methodParameterCount(int index) const override { return m_methodArgCount[index]; }
    int methodParameterType(int methodIndex, int paramIndex) const override { return m_methodArgTypes[methodIndex][paramIndex]; }
    QByteArrayList signalParameterNames(int index) const override
    {
        QByteArrayList res;
        int count = signalParameterCount(index);
        while (count--)
            res << QByteArray{};
        return res;
    }
    int propertyIndexFromSignal(int index) const override
    {
        switch (index) {
        case 0: return m_properties[1];
        }
        return -1;
    }
    int propertyRawIndexFromSignal(int index) const override
    {
        switch (index) {
        case 0: return 0;
        }
        return -1;
    }
    const QByteArray signalSignature(int index) const override
    {
        switch (index) {
        case 0: return QByteArrayLiteral("availableRolesChanged()");
        case 1: return QByteArrayLiteral("dataChanged(QtPrivate::IndexList,QtPrivate::IndexList,QList<int>)");
        case 2: return QByteArrayLiteral("rowsInserted(QtPrivate::IndexList,int,int)");
        case 3: return QByteArrayLiteral("rowsRemoved(QtPrivate::IndexList,int,int)");
        case 4: return QByteArrayLiteral("rowsMoved(QtPrivate::IndexList,int,int,QtPrivate::IndexList,int)");
        case 5: return QByteArrayLiteral("currentChanged(QtPrivate::IndexList,QtPrivate::IndexList)");
        case 6: return QByteArrayLiteral("resetModel()");
        case 7: return QByteArrayLiteral("headerDataChanged(Qt::Orientation,int,int)");
        case 8: return QByteArrayLiteral("columnsInserted(QtPrivate::IndexList,int,int)");
        case 9: return QByteArrayLiteral("layoutChanged(QtPrivate::IndexList,QAbstractItemModel::LayoutChangeHint)");
        }
        return QByteArrayLiteral("");
    }
    const QByteArray methodSignature(int index) const override
    {
        switch (index) {
        case 0: return QByteArrayLiteral("replicaSizeRequest(QtPrivate::IndexList)");
        case 1: return QByteArrayLiteral("replicaRowRequest(QtPrivate::IndexList,QtPrivate::IndexList,QList<int>)");
        case 2: return QByteArrayLiteral("replicaHeaderRequest(QList<Qt::Orientation>,QList<int>,QList<int>)");
        case 3: return QByteArrayLiteral("replicaSetCurrentIndex(QtPrivate::IndexList,QItemSelectionModel::SelectionFlags)");
        case 4: return QByteArrayLiteral("replicaSetData(QtPrivate::IndexList,QVariant,int)");
        case 5: return QByteArrayLiteral("replicaCacheRequest(size_t,QList<int>)");
        }
        return QByteArrayLiteral("");
    }
    QMetaMethod::MethodType methodType(int) const override
    {
        return QMetaMethod::Slot;
    }
    const QByteArray typeName(int index) const override
    {
        switch (index) {
        case 0: return QByteArrayLiteral("QSize");
        case 1: return QByteArrayLiteral("QtPrivate::DataEntries");
        case 2: return QByteArrayLiteral("QVariantList");
        case 3: return QByteArrayLiteral("");
        case 5: return QByteArrayLiteral("QtPrivate::MetaAndDataEntries");
        }
        return QByteArrayLiteral("");
    }

    QByteArrayList methodParameterNames(int index) const override
    {
        QByteArrayList res;
        int count = methodParameterCount(index);
        while (count--)
            res << QByteArray{};
        return res;
    }

    QByteArray objectSignature() const override { return QByteArray{}; }
    bool isAdapterSignal(int index) const override
    {
        switch (index) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 8:
        case 9:
            return true;
        }
        return false;
    }
    bool isAdapterMethod(int index) const override
    {
        switch (index) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            return true;
        }
        return false;
    }
    bool isAdapterProperty(int index) const override
    {
        switch (index) {
        case 0:
        case 1:
            return true;
        }
        return false;
    }

    int m_properties[3];
    int m_signals[11];
    int m_methods[7];
    int m_signalArgCount[10];
    const int* m_signalArgTypes[10];
    int m_methodArgCount[6];
    const int* m_methodArgTypes[6];
    QString m_name;
};

QT_END_NAMESPACE

#endif //QREMOTEOBJECTS_ABSTRACT_ITEM_ADAPTER_P_H
