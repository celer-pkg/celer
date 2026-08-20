// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#ifndef QREMOTEOBJECTS_ABSTRACT_ITEM_MODEL_TYPES_P_H
#define QREMOTEOBJECTS_ABSTRACT_ITEM_MODEL_TYPES_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qitemselectionmodel.h>
#include <QtCore/qlist.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qpair.h>
#include <QtCore/qsize.h>
#include <QtCore/qvariant.h>
#include <QtRemoteObjects/qtremoteobjectglobal.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

namespace QtPrivate {

struct ModelIndex
{
    ModelIndex() : row(-1), column(-1) {}
    ModelIndex(int row_, int column_)
        : row(row_)
        , column(column_)
    {}

    inline bool operator==(const ModelIndex &other) const { return row == other.row && column == other.column; }
    inline bool operator!=(const ModelIndex &other) const { return !(*this == other); }
    int row;
    int column;
};

typedef QList<ModelIndex> IndexList;

struct IndexValuePair
{
    explicit IndexValuePair(const IndexList index_ = IndexList(), const QVariantList &data_ = QVariantList(),
                            bool hasChildren_ = false, const Qt::ItemFlags &flags_ = Qt::ItemFlags(), const QSize &size_ = {})
        : index(index_)
        , data(data_)
        , flags(flags_)
        , hasChildren(hasChildren_)
        , size(size_)
    {}

    inline bool operator==(const IndexValuePair &other) const { return index == other.index && data == other.data && hasChildren == other.hasChildren && flags == other.flags; }
    inline bool operator!=(const IndexValuePair &other) const { return !(*this == other); }

    IndexList index;
    QVariantList data;
    Qt::ItemFlags flags;
    bool hasChildren;
    QList<IndexValuePair> children;
    QSize size;
};

struct DataEntries
{
    inline bool operator==(const DataEntries &other) const { return data == other.data; }
    inline bool operator!=(const DataEntries &other) const { return !(*this == other); }

    QList<IndexValuePair> data;
};

struct MetaAndDataEntries : DataEntries
{
    QList<int> roles;
    QSize size;
};

inline QDebug operator<<(QDebug stream, const ModelIndex &index)
{
    return stream.nospace() << "ModelIndex[row=" << index.row << ", column=" << index.column << "]";
}

inline QDataStream& operator<<(QDataStream &stream, const ModelIndex &index)
{
    return stream << index.row << index.column;
}

inline QDataStream& operator>>(QDataStream &stream, ModelIndex &index)
{
    return stream >> index.row >> index.column;
}

inline QDataStream& operator<<(QDataStream &stream, Qt::Orientation orient)
{
    return stream << static_cast<int>(orient);
}

inline QDataStream& operator>>(QDataStream &stream, Qt::Orientation &orient)
{
    int val;
    QDataStream &ret = stream >> val;
    orient = static_cast<Qt::Orientation>(val);
    return ret;
}

inline QDataStream& operator<<(QDataStream &stream, QItemSelectionModel::SelectionFlags command)
{
    return stream << static_cast<int>(command);
}

inline QDataStream& operator>>(QDataStream &stream, QItemSelectionModel::SelectionFlags &command)
{
    int val;
    QDataStream &ret = stream >> val;
    command = static_cast<QItemSelectionModel::SelectionFlags>(val);
    return ret;
}

inline QDebug operator<<(QDebug stream, const IndexValuePair &pair)
{
    return stream.nospace() << "IndexValuePair[index=" << pair.index << ", data=" << pair.data << ", hasChildren=" << pair.hasChildren << ", flags=" << pair.flags << "]";
}

inline QDataStream& operator<<(QDataStream &stream, const IndexValuePair &pair)
{
    return stream << pair.index << pair.data << pair.hasChildren << static_cast<int>(pair.flags) << pair.children << pair.size;
}

inline QDataStream& operator>>(QDataStream &stream, IndexValuePair &pair)
{
    int flags;
    QDataStream &ret = stream >> pair.index >> pair.data >> pair.hasChildren >> flags >> pair.children >> pair.size;
    pair.flags = static_cast<Qt::ItemFlags>(flags);
    return ret;
}

inline QDebug operator<<(QDebug stream, const DataEntries &entries)
{
    return stream.nospace() << "DataEntries[" << entries.data << "]";
}

inline QDataStream& operator<<(QDataStream &stream, const DataEntries &entries)
{
    return stream << entries.data;
}

inline QDataStream& operator>>(QDataStream &stream, DataEntries &entries)
{
    return stream >> entries.data;
}

inline QDataStream& operator<<(QDataStream &stream, const MetaAndDataEntries &entries)
{
    return stream << entries.data << entries.roles << entries.size;
}

inline QDataStream& operator>>(QDataStream &stream, MetaAndDataEntries &entries)
{
    return stream >> entries.data >> entries.roles >> entries.size;
}

inline QString modelIndexToString(const IndexList &list)
{
    QString s;
    QDebug(&s) << list;
    return s;
}

inline QString modelIndexToString(const ModelIndex &index)
{
    QString s;
    QDebug(&s) << index;
    return s;
}

inline QModelIndex toQModelIndex(const IndexList &list, const QAbstractItemModel *model, bool *ok = nullptr, bool ensureItem = false)
{
    if (ok)
        *ok = true;
    QModelIndex result;
    for (int i = 0; i < list.size(); ++i) {
        const ModelIndex &index = list[i];
        if (ensureItem)
            const_cast<QAbstractItemModel *>(model)->setData(result, index.row, Qt::UserRole - 1);

        result = model->index(index.row, index.column, result);
        if (!result.isValid()) {
            if (ok) {
                *ok = false;
            } else {
                qFatal("Internal error: invalid index=%s in indexList=%s",
                       qPrintable(modelIndexToString(list[i])), qPrintable(modelIndexToString(list)));
            }
            return QModelIndex();
        }
    }
    return result;
}

inline IndexList toModelIndexList(const QModelIndex &index, const QAbstractItemModel *model)
{
    IndexList list;
    if (index.isValid()) {
        list << ModelIndex(index.row(), index.column());
        for (QModelIndex curIndex = model->parent(index); curIndex.isValid(); curIndex = model->parent(curIndex))
            list.prepend(ModelIndex(curIndex.row(), curIndex.column()));
    }
    return list;
}

} // namespace QtPrivate

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN_TAGGED(QtPrivate::ModelIndex, QtPrivate__ModelIndex, /* not exported */)
QT_DECL_METATYPE_EXTERN_TAGGED(QtPrivate::IndexList, QtPrivate__IndexList, /* not exported */)
QT_DECL_METATYPE_EXTERN_TAGGED(QtPrivate::DataEntries, QtPrivate__DataEntries, /* not exported */)
QT_DECL_METATYPE_EXTERN_TAGGED(QtPrivate::MetaAndDataEntries, QtPrivate__MetaAndDataEntries,
                               /* not exported */)
QT_DECL_METATYPE_EXTERN_TAGGED(QtPrivate::IndexValuePair, QtPrivate__IndexValuePair,
                               /* not exported */)
QT_DECL_METATYPE_EXTERN_TAGGED(Qt::Orientation, Qt__Orientation, /* not exported */)
QT_DECL_METATYPE_EXTERN_TAGGED(QItemSelectionModel::SelectionFlags,
                               QItemSelectionModel__SelectionFlags,
                               /* not exported */)

#endif // QREMOTEOBJECTS_ABSTRACT_ITEM_MODEL_TYPES_P_H
